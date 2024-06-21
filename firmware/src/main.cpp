#include <cstring>
#include <harp_synchronizer.h>
#include <harp_core.h>
#include <core_registers.h>
#include <reg_types.h>
#include <config.h>
#include <hardware/dma.h>
#include <hardware/timer.h>
#include <utility>
//#include <pico/divider.h>

// Alarm number used to trigger dispatch of periodic time msg.
int32_t clkout_alarm_num = -1;
uint32_t clkout_irq_number;

int32_t slow_clkout_alarm_num = -1;
uint32_t slow_clkout_irq_number;

// Double-buffered outgoing msg buffer.
volatile uint8_t harp_time_msg [2][6] = {{0xAA, 0xAF, 0, 0, 0, 0},
                                         {0xAA, 0xAF, 0, 0, 0, 0}};
volatile uint8_t (*dispatch_buffer)[6];
volatile uint8_t (*load_buffer)[6];

/**
 * \brief nonblocking way to dispatch uart characters.
 * \details assumes a global DMA channel has already been assigned.
 * \note this fn returns the dma channel so that we can check later if
 *  the consumed channel is busy with the Pico SDK's:
 *  <code>dma_channel_is_busy(dma_chan)</code>;
 * \return the dma channel used to dispatch the uart stream.
 */
uint dispatch_uart_stream(uart_inst_t* uart, uint8_t* starting_address,
                          size_t word_count)
{
    // DMA channel will write data to the uart, paced by DREQ_TX.
    uint dma_chan = dma_claim_unused_channel(true);
    dma_channel_config conf = dma_channel_get_default_config(dma_chan);

    // Setup Sample Channel.
    channel_config_set_transfer_data_size(&conf, DMA_SIZE_8);
    channel_config_set_read_increment(&conf, true); // read from starting memory address.
    channel_config_set_write_increment(&conf, false); // write to fixed uart memorey address.
    channel_config_set_irq_quiet(&conf, true);
    // Pace data according to pio providing data.
    uint uart_dreq = (uart == uart0)? DREQ_UART0_TX : DREQ_UART1_TX;
    channel_config_set_dreq(&conf, uart_dreq);
    channel_config_set_enable(&conf, true);
    // Apply samp_chan_ configuration.
    dma_channel_configure(
        dma_chan,               // Channel to be configured
        &conf,                  // corresponding DMA config.
        &uart_get_hw(uart)->dr, // write (dst) address.
        starting_address,       // read (source) address.
        word_count,             // Number of word transfers i.e: len(string).
        true                    // Do start immediately.
    );

    return dma_chan;
}

int64_t disable_led(alarm_id_t id, void* user_data)
{
    gpio_put(LED_PIN, 0);
    return 0; // Return 0 to indicate not to reschedule the alarm.
}

/**
 * \brief Dispatch the time to 16x outputs.
 * \warning called inside of interrupt. Do not block.
 */
void __not_in_flash_func(dispatch_harp_clkout)()
{
    // Dispatch the previously-configured time.
    dispatch_uart_stream(HARP_UART, (uint8_t*)&dispatch_buffer, 4);
    // Clear the latched hardware interrupt.
    timer_hw->intr |= (1u << clkout_alarm_num);

    // Calculate next whole harp time second.
    uint32_t curr_harp_seconds = HarpCore::harp_time_s();
    uint32_t next_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 1'000'000UL;
    // Offset such that the start of last byte occurs on the whole second per:
    // https://harp-tech.org/protocol/SynchronizationClock.html#serial-configuration
    // Offset such that the start of last byte occurs on the whole second per:
    // https://harp-tech.org/protocol/SynchronizationClock.html#serial-configuration
    next_msg_harp_time_us_32 += HARP_SYNC_START_OFFSET_US;
    // Update time contents in the next message.
    memcpy((void*)(load_buffer + 2), (void*)(&curr_harp_seconds), 4);
    // Toggle ping-pong buffers.
    std::swap(load_buffer, dispatch_buffer);
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    uint32_t alarm_time_us = HarpCore::harp_to_system_us_32(next_msg_harp_time_us_32);
    timer_hw->inte |= (1u << clkout_alarm_num); // enable alarm to trigger interrupt.
    timer_hw->alarm[clkout_alarm_num] = alarm_time_us; // write start time & arm alarm.
}

void __not_in_flash_func(dispatch_slow_sync_clkout)()
{
    // TODO: implement this.
//    dispatch_uart_stream(SLOW_SYNC_UART, (uint8_t*)&dispatch_buffer, 4);
}

// Core0 main.
int main()
{
// Setup GPIO pin.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
    // Init Synchronizer.
    HarpSynchronizer::init(HARP_UART, HARP_CLKIN_PIN);
    // Setup UART TX for periodic transmission of the time at a low baudrate.
    uart_inst_t* uart_id = HARP_UART;
    uart_init(uart_id, HARP_SYNC_BAUDRATE);
    uart_set_hw_flow(uart_id, false, false);
    uart_set_fifo_enabled(uart_id, false); // Set FIFO size to 1.
    uart_set_format(uart_id, 8, 1, UART_PARITY_NONE);
    gpio_set_function(HARP_CLKOUT_PIN, GPIO_FUNC_UART);
// If we enable debug msgs, we cannot use the slow output.
#if defined(DEBUG)
    stdio_uart_init_full(SLOW_SYNC_UART, 921600, UART_TX_PIN, -1); // TX only.
    printf("Hello, from an RP2040!\r\n");
#else
    // Setup UART TX for slow ephys clock output.
    uart_inst_t* slow_uart_id = SLOW_SYNC_UART;
    uart_init(slow_uart_id, SLOW_SYNC_BAUDRATE);
    uart_set_hw_flow(slow_uart_id, false, false);
    uart_set_fifo_enabled(slow_uart_id, false); // Set FIFO size to 1.
    uart_set_format(slow_uart_id, 8, 1, UART_PARITY_NONE);
    gpio_set_function(SLOW_SYNC_CLKOUT_PIN, GPIO_FUNC_UART);
#endif
// Setup Outgoing msg double buffer;
    dispatch_buffer = (&harp_time_msg[0]);
    load_buffer = (&harp_time_msg[1]);
// Setup Harp CLKOUT periodic outgoing time message.
    clkout_alarm_num = hardware_alarm_claim_unused(true); // true --> required
    clkout_irq_number = TIMER_IRQ_0 + clkout_alarm_num; // harware_alarm_irq_number() in sdk.
    // Attach interrupt to function and eneable alarm to generate interrupt.
    irq_set_exclusive_handler(clkout_irq_number, dispatch_harp_clkout);
    irq_set_enabled(clkout_irq_number, true);
    // Compute start time for Harp CLKOUT msg.
    // Get the next whole second.
    uint32_t curr_harp_seconds = HarpCore::harp_time_s();
    uint32_t next_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 1'000'000UL;
    // Offset such that the start of last byte occurs on the whole second per:
    // https://harp-tech.org/protocol/SynchronizationClock.html#serial-configuration
    next_msg_harp_time_us_32 += HARP_SYNC_START_OFFSET_US;
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    uint32_t alarm_time_us = HarpCore::harp_to_system_us_32(next_msg_harp_time_us_32);
    timer_hw->inte |= (1u << clkout_alarm_num); // enable alarm to trigger interrupt.
    timer_hw->alarm[clkout_alarm_num] = alarm_time_us; // write start time & arm alarm.
#if !defined(DEBUG)
// Setup SLOW CLKOUT periodic outgoing time message.
    slow_clkout_alarm_num = hardware_alarm_claim_unused(true); // true --> required
    slow_clkout_irq_number = TIMER_IRQ_0 + clkout_alarm_num; // harware_alarm_irq_number() in sdk.
    // Attach interrupt to function and eneable alarm to generate interrupt.
    irq_set_exclusive_handler(slow_clkout_irq_number, dispatch_slow_sync_clkout);
    irq_set_enabled(slow_clkout_irq_number, true);
    // Compute start time for slow CLKOUT msg (next whole second).
    uint32_t next_slow_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 1'000'000UL;
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    alarm_time_us = HarpCore::harp_to_system_us_32(next_slow_msg_harp_time_us_32);
    timer_hw->inte |= (1u << slow_clkout_alarm_num); // enable alarm to trigger interrupt.
    timer_hw->alarm[slow_clkout_alarm_num] = alarm_time_us; // write start time & arm alarm.
#endif
    while(true)
    {}
}

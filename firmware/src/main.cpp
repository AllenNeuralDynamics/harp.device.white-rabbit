#include <cstring>
#include <harp_synchronizer.h>
#include <harp_core.h>
#include <harp_c_app.h>
#include <core_registers.h>
#include <reg_types.h>
#include <config.h>
#include <hardware/dma.h>
#include <hardware/timer.h>
#include <utility>

// Harp App Setup.
const uint16_t who_am_i = HARP_DEVICE_ID;
const uint8_t hw_version_major = 0;
const uint8_t hw_version_minor = 0;
const uint8_t assembly_version = 0;
const uint8_t harp_version_major = 0;
const uint8_t harp_version_minor = 0;
const uint8_t fw_version_major = 0;
const uint8_t fw_version_minor = 0;
const uint16_t serial_number = 0;

const uint8_t reg_count = 0;

// Alarm number used to trigger dispatch of periodic time msg.
int32_t harp_clkout_alarm_num = -1;
uint32_t harp_clkout_irq_number;

int32_t slow_clkout_alarm_num = -1;
uint32_t slow_clkout_irq_number;

volatile uint harp_clkout_dma_chan;
volatile uint slow_clkout_dma_chan;

// Double-buffered outgoing msg buffer.
volatile uint8_t harp_time_msg [2][6] = {{0xAA, 0xAF, 0, 0, 0, 0},
                                         {0xAA, 0xAF, 0, 0, 0, 0}};
volatile uint8_t (*dispatch_buffer)[6];
volatile uint8_t (*load_buffer)[6];

/**
 * \brief nonblocking way to dispatch uart characters.
 * \details assumes a global DMA channel has already been assigned.
 */
void __not_in_flash_func(dispatch_uart_stream)(uint dma_chan, uart_inst_t* uart,
                         uint8_t* starting_address, size_t word_count)
{
    // DMA channel will write data to the uart, paced by DREQ_TX.
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
}

//inline void disable_led() {gpio_put(LED_PIN, 0);}

/**
 * \brief Dispatch the time to 16x outputs.
 * \warning called inside of interrupt. Do not block.
 */
void __not_in_flash_func(dispatch_harp_clkout)()
{
#if defined(DEBUG)
    printf("entered interrupt!\r\n");
#endif
    // Dispatch the previously-configured time.
    dispatch_uart_stream(harp_clkout_dma_chan, HARP_UART,
                         (uint8_t*)&dispatch_buffer, 4);
    // Clear the latched hardware interrupt.
    timer_hw->intr |= (1u << harp_clkout_alarm_num);

    // Calculate next whole harp time second. +2 bc we wake up *before* the
    // elapse of the next whole second when we are supposed to emit the msg.
    uint32_t curr_harp_seconds = HarpCore::harp_time_s();
    uint32_t next_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 2'000'000UL;
    // Offset such that the start of last byte occurs on the whole second per:
    // https://harp-tech.org/protocol/SynchronizationClock.html#serial-configuration
    // Offset such that the start of last byte occurs on the whole second per:
    // https://harp-tech.org/protocol/SynchronizationClock.html#serial-configuration
    next_msg_harp_time_us_32 += HARP_SYNC_START_OFFSET_US;
    // Update time contents in the next message.
    memcpy((void*)(load_buffer + 2), (void*)(&curr_harp_seconds), 4);
    // Toggle ping-pong buffers.
    std::swap(load_buffer, dispatch_buffer);
    gpio_put(LED_PIN, !gpio_get(LED_PIN)); // toggle LED.
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    uint32_t alarm_time_us = HarpCore::harp_to_system_us_32(next_msg_harp_time_us_32);
    // Enable alarm to trigger interrupt.
    timer_hw->inte |= (1u << harp_clkout_alarm_num);
    // Arm alarm by writing the alarm time.
    timer_hw->alarm[harp_clkout_alarm_num] = alarm_time_us;
}

void __not_in_flash_func(dispatch_slow_sync_clkout)()
{
    // TODO: implement this.
//    dispatch_uart_stream(SLOW_SYNC_UART, (uint8_t*)&dispatch_buffer, 4);
}

// Define Harp App registers.
struct app_regs_t
{
// No regs yet.
} app_regs;

// Define "specs" per register.
RegSpecs app_reg_specs[reg_count]
{
};

// Define read/write reg handler functions.
RegFnPair reg_handler_fns[reg_count]
{

};

void update_app_state()
{
}

void reset_app()
{
// no reg handler funcs yet.
}

// Create Harp App.
HarpCApp& app = HarpCApp::init(who_am_i, hw_version_major, hw_version_minor,
                               assembly_version,
                               harp_version_major, harp_version_minor,
                               fw_version_major, fw_version_minor,
                               serial_number, "White Rabbit",
                               &app_regs, app_reg_specs,
                               reg_handler_fns, reg_count, update_app_state,
                               reset_app);

// Core0 main.
int main()
{
// Setup DMA.
    harp_clkout_dma_chan = dma_claim_unused_channel(true);
    slow_clkout_dma_chan = dma_claim_unused_channel(true);
// Setup GPIO pin.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
    // Init Synchronizer.
    HarpSynchronizer& sync = HarpSynchronizer::init(HARP_UART, HARP_CLKIN_PIN);
    app.set_synchronizer(&sync);
    // Setup UART TX for periodic transmission of the time at 100KBaud.
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
    // Setup UART TX for periodic transmission of time at 1KBaud.
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
    // Leverage RP2040 ALARMs to dispatch periodically via interrupt handler.
    harp_clkout_alarm_num = hardware_alarm_claim_unused(true); // true --> required
    harp_clkout_irq_number = TIMER_IRQ_0 + harp_clkout_alarm_num; // harware_alarm_irq_number() in sdk.
    // Attach interrupt to function and eneable alarm to generate interrupt.
    irq_set_exclusive_handler(harp_clkout_irq_number, dispatch_harp_clkout);
    irq_set_enabled(harp_clkout_irq_number, true);
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
    // Enable alarm to trigger interrupt.
    timer_hw->inte |= (1u << harp_clkout_alarm_num);
    // Arm alarm by writing the alarm time.
    timer_hw->alarm[harp_clkout_alarm_num] = alarm_time_us;
#if defined(DEBUG)
    printf("Curr Harp seconds: %lu[s] | 1st alarm (Harp) time %lu[us] | 1st alarm (sys time) %lu[us].\r\n",
           curr_harp_seconds, next_msg_harp_time_us_32, alarm_time_us);
    printf("Scheduled first alarm.\r\n");
#else
/*
// Setup SLOW CLKOUT periodic outgoing time message.
// Leverage RP2040 ALARMs to dispatch periodically via interrupt handler.
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
    // Enable alarm to trigger interrupt.
    timer_hw->inte |= (1u << slow_clkout_alarm_num);
    // Arm alarm by writing the alarm time.
    timer_hw->alarm[slow_clkout_alarm_num] = alarm_time_us;
*/
#endif
    while(true)
        app.run();
}

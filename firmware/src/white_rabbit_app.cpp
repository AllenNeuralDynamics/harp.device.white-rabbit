#include <white_rabbit_app.h>

// Apply starting values.
uint32_t counter_interval_us = 0;
uint32_t last_msg_emit_time_us;
bool was_synced = false;

app_regs_t app_regs;

// Harp CLKout Double Buffer Setup
volatile uint __not_in_flash("double_buffers") harp_clkout_dma_chan;

int32_t __not_in_flash("double_buffers") harp_clkout_alarm_num = -1;
uint32_t __not_in_flash("double_buffers") harp_clkout_irq_number;

volatile uint8_t __not_in_flash("double_buffers") harp_time_msg_a[6] =
    {0xAA, 0xAF, 0x00, 0x00, 0x00, 0x00};
volatile uint8_t __not_in_flash("double_buffers") harp_time_msg_b[6] =
    {0xAA, 0xAF, 0x00, 0x00, 0x00, 0x00};

volatile uint8_t __not_in_flash("double_buffers") *dispatch_buffer;
volatile uint8_t __not_in_flash("double_buffers") *load_buffer;

// Slow CLKout Double Buffer Setup
volatile uint __not_in_flash("double_buffers") slow_clkout_dma_chan;

int32_t __not_in_flash("double_buffers") slow_clkout_alarm_num = -1;
uint32_t __not_in_flash("double_buffers") slow_clkout_irq_number;

volatile uint32_t __not_in_flash("double_buffers") slow_clkout_seconds_a;
volatile uint32_t __not_in_flash("double_buffers") slow_clkout_seconds_b;

volatile uint32_t __not_in_flash("double_buffers") *dispatch_second;
volatile uint32_t __not_in_flash("double_buffers") *load_second;

// PPS Alarm/IRQ resources
int32_t __not_in_flash("double_buffers") pps_output_alarm_num = -1;
uint32_t __not_in_flash("double_buffers") pps_output_irq_number;

void setup_harp_clkout()
{
    // Setup DMA.
    harp_clkout_dma_chan = dma_claim_unused_channel(true);
    // Setup UART TX for periodic transmission of the time at 100KBaud.
    uart_inst_t* uart_id = HARP_UART;
    uart_init(uart_id, HARP_SYNC_BAUDRATE);
    uart_set_hw_flow(uart_id, false, false);
    uart_set_fifo_enabled(uart_id, false); // Set FIFO size to 1.
    uart_set_format(uart_id, 8, 1, UART_PARITY_NONE);
    gpio_set_function(HARP_CLKOUT_PIN, GPIO_FUNC_UART);
    // Setup Outgoing msg double buffer;
    dispatch_buffer = &(harp_time_msg_a[0]);
    load_buffer = &(harp_time_msg_b[0]);
    // Setup Harp CLKOUT periodic outgoing time message.
    // Leverage RP2040 ALARMs to dispatch periodically via interrupt handler.
    harp_clkout_alarm_num = hardware_alarm_claim_unused(true); // true --> required
    // TODO: harp_clkout_irq_number = TIMER_IRQ_NUM(timer_hw, harp_clkout_alarm_num);
    harp_clkout_irq_number = TIMER_IRQ_0 + harp_clkout_alarm_num;
    // Attach interrupt to function and enable alarm to generate interrupt.
    irq_set_exclusive_handler(harp_clkout_irq_number,
                              dispatch_and_reschedule_harp_clkout);
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
}

void __not_in_flash_func(dispatch_and_reschedule_harp_clkout)()
{
    // Dispatch the previously-configured time.
    dispatch_uart_stream(harp_clkout_dma_chan, HARP_UART, (uint8_t*)dispatch_buffer, 6);
    // Clear the latched hardware interrupt.
    timer_hw->intr |= (1u << harp_clkout_alarm_num);
    // Get the next whole harp time second. +2 bc we wake up *before* the
    // elapse of the next whole second when we are supposed to emit the msg.
    uint32_t curr_harp_seconds = HarpCore::harp_time_s();
    uint32_t next_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 2'000'000UL;
#if defined(DEBUG)
    printf("Sending: %x %x %x %x %x %x\r\n", dispatch_buffer[0],
           dispatch_buffer[1], dispatch_buffer[2], dispatch_buffer[3],
           dispatch_buffer[4], dispatch_buffer[5]);
#endif
    // Offset such that the start of last byte occurs on the whole second per:
    // https://harp-tech.org/protocol/SynchronizationClock.html#serial-configuration
    next_msg_harp_time_us_32 += HARP_SYNC_START_OFFSET_US;
    // Update time contents in the next message.
    memcpy((void*)(load_buffer + 2), (void*)(&curr_harp_seconds),
           sizeof(curr_harp_seconds));
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

void setup_slow_clkout()
{
    // Setup DMA.
    slow_clkout_dma_chan = dma_claim_unused_channel(true);
    // Setup GPIO Pins.
    // Setup UART TX for periodic transmission of time at 1KBaud.
    uart_inst_t* slow_uart_id = SLOW_SYNC_UART;
    uart_init(slow_uart_id, SLOW_SYNC_BAUDRATE);
    uart_set_hw_flow(slow_uart_id, false, false);
    uart_set_fifo_enabled(slow_uart_id, false); // Set FIFO size to 1.
    uart_set_format(slow_uart_id, 8, 1, UART_PARITY_NONE);
    gpio_set_function(SLOW_SYNC_CLKOUT_PIN, GPIO_FUNC_UART);
    // Setup SLOW CLKOUT periodic outgoing time message.
    // Setup Outgoing msg double buffer;
    dispatch_second = &slow_clkout_seconds_a;
    load_second = &slow_clkout_seconds_b;
    // Setup SLOW CLKOUT periodic outgoing time message.
    // Leverage RP2040 ALARMs to dispatch periodically via interrupt handler.
    slow_clkout_alarm_num = hardware_alarm_claim_unused(true); // true --> required
    slow_clkout_irq_number = TIMER_IRQ_0 + slow_clkout_alarm_num; // TIMER_IRQ_NUM(TIMER_IRQ_0, slow_clkout_alarm_num);
    // Attach interrupt to function and enable alarm to generate interrupt.
    irq_set_exclusive_handler(slow_clkout_irq_number,
                              dispatch_and_reschedule_slow_clkout);
    irq_set_enabled(slow_clkout_irq_number, true);
    // Compute start time for Slow CLKOUT msg.
    // Get the next whole second.
    uint32_t curr_harp_seconds = HarpCore::harp_time_s();
    uint32_t next_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 1'000'000UL;
    // Apply offset if any.
    next_msg_harp_time_us_32 += SLOW_SYNC_START_OFFSET_US;
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    uint32_t alarm_time_us = HarpCore::harp_to_system_us_32(next_msg_harp_time_us_32);
    // Enable alarm to trigger interrupt.
    timer_hw->inte |= (1u << slow_clkout_alarm_num);
    // Arm alarm by writing the alarm time.
    timer_hw->alarm[slow_clkout_alarm_num] = alarm_time_us;
}

void __not_in_flash_func(dispatch_and_reschedule_slow_clkout)()
{
    // Dispatch the previously-configured time.
    dispatch_uart_stream(slow_clkout_dma_chan, SLOW_SYNC_UART,
                         (uint8_t*)dispatch_second, sizeof(dispatch_second));
    // Clear the latched hardware interrupt.
    timer_hw->intr |= (1u << slow_clkout_alarm_num);
    // Get the next whole harp time second.
    uint32_t curr_harp_seconds = HarpCore::harp_time_s();
    uint32_t next_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 1'000'000UL;
    // Account for extra second if we wake up *before* the elapse of the next
    // whole second.
    if (SLOW_SYNC_START_OFFSET_US < 0)
        next_msg_harp_time_us_32 += 1'000'000UL;
    // Add offset (if any).
    next_msg_harp_time_us_32 += SLOW_SYNC_START_OFFSET_US;
    // Update time contents in the next message.
    memcpy((void*)(load_second + 2), (void*)(&curr_harp_seconds),
           sizeof(load_second));
    // Toggle ping-pong buffers.
    std::swap(load_second, dispatch_second);
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    uint32_t alarm_time_us = HarpCore::harp_to_system_us_32(next_msg_harp_time_us_32);
    // Enable alarm to trigger interrupt.
    timer_hw->inte |= (1u << slow_clkout_alarm_num);
    // Arm alarm by writing the alarm time.
    timer_hw->alarm[slow_clkout_alarm_num] = alarm_time_us;

}

void cleanup_slow_clkout()
{
    // Disarm alarm.
    timer_hw->armed |= (1u << slow_clkout_alarm_num);
    // Clear the latched hardware interrupt (if latched).
    timer_hw->intr |= (1u << slow_clkout_alarm_num);
    // Unreserve Alarm and IRQ.
    hardware_alarm_unclaim(slow_clkout_alarm_num);
    irq_remove_handler(slow_clkout_irq_number,
                       dispatch_and_reschedule_slow_clkout);
    gpio_deinit(SLOW_SYNC_CLKOUT_PIN);
}

void setup_pps_output()
{
    // Setup GPIO pins.
    gpio_init(SLOW_SYNC_CLKOUT_PIN); // shared with PPS.
    gpio_set_dir(SLOW_SYNC_CLKOUT_PIN, GPIO_OUT);
    gpio_put(SLOW_SYNC_CLKOUT_PIN, 0);
    // Setup Harp CLKOUT periodic outgoing time message.
    // Leverage RP2040 ALARMs to dispatch periodically via interrupt handler.
    pps_output_alarm_num = hardware_alarm_claim_unused(true); // true --> required
    // TODO: pps_output_irq_number = TIMER_IRQ_NUM(timer_hw, harp_clkout_alarm_num);
    pps_output_irq_number = TIMER_IRQ_0 + harp_clkout_alarm_num;
    // Attach interrupt to function and enable alarm to generate interrupt.
    irq_set_exclusive_handler(pps_output_irq_number, update_pps_output);
    irq_set_enabled(pps_output_irq_number, true);
    // Compute start time for Slow CLKOUT msg.
    // Get the next whole second.
    uint32_t curr_harp_seconds = HarpCore::harp_time_s();
    uint32_t next_msg_harp_time_us_32 = (curr_harp_seconds * 1000000UL) + 1'000'000UL;
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    uint32_t alarm_time_us = HarpCore::harp_to_system_us_32(next_msg_harp_time_us_32);
    // Enable alarm to trigger interrupt.
    timer_hw->inte |= (1u << pps_output_alarm_num);
    // Arm alarm by writing the alarm time.
    timer_hw->alarm[pps_output_alarm_num] = alarm_time_us;
}

void update_pps_output()
{
    gpio_xor_mask(1 << SLOW_SYNC_CLKOUT_PIN); // Toggle GPIO Pin.
    // Schedule next update.
    // Get the next whole harp time **half**-second.
    uint32_t curr_harp_us = HarpCore::harp_time_us_32();
    uint32_t next_msg_harp_time_us_32 = curr_harp_us + 500'000UL;
    // Schedule next time msg dispatch in system time.
    // Low-level interface (fast!) to re-schedule this function.
    uint32_t alarm_time_us = HarpCore::harp_to_system_us_32(next_msg_harp_time_us_32);
    // Enable alarm to trigger interrupt.
    timer_hw->inte |= (1u << pps_output_alarm_num);
    // Arm alarm by writing the alarm time.
    timer_hw->alarm[pps_output_alarm_num] = alarm_time_us;
}

void cleanup_pps_output()
{
    // Disarm alarm.
    timer_hw->armed |= (1u << pps_output_alarm_num);
    // Clear the latched hardware interrupt (if latched).
    timer_hw->intr |= (1u << pps_output_alarm_num);
    // Unreserve Alarm and IRQ.
    hardware_alarm_unclaim(pps_output_alarm_num);
    irq_remove_handler(pps_output_irq_number, update_pps_output);
    gpio_deinit(SLOW_SYNC_CLKOUT_PIN); // shared with PPS.
}

void write_counter_frequency_hz(msg_t& msg)
{
    HarpCore::copy_msg_payload_to_register(msg);
    // Cap maximum value.
    if (app_regs.CounterFrequencyHz > MAX_EVENT_FREQUENCY_HZ)
    {
        app_regs.CounterFrequencyHz = MAX_EVENT_FREQUENCY_HZ;
        // Update pre-computed interval.
        counter_interval_us = div_u32u32(1'000'000UL,
                                         app_regs.CounterFrequencyHz);
        last_msg_emit_time_us = time_us_32(); // reset interval.
        HarpCore::send_harp_reply(WRITE_ERROR, msg.header.address);
        return;
    }
    // Update pre-computed interval.
    counter_interval_us = div_u32u32(1'000'000UL, app_regs.CounterFrequencyHz);
    last_msg_emit_time_us = HarpCore::harp_time_us_32();
    if (!HarpCore::is_muted())
        HarpCore::send_harp_reply(WRITE, msg.header.address);
}

void write_aux_port_fn(msg_t& msg)
{
    uint8_t old_aux_fn = app_regs.AuxPortFn;
    HarpCore::copy_msg_payload_to_register(msg);
    if (app_regs.AuxPortFn > 2)
    {
        if (!HarpCore::is_muted())
            HarpCore::send_harp_reply(WRITE_ERROR, msg.header.address);
        app_regs.AuxPortFn = old_aux_fn; // Restore original setting.
        return;
    }
    reset_aux_fn();
    switch (app_regs.AuxPortFn)
    {
        case 0: // Clear behaviors.
            break;
        case 1: // Slow CLKout.
            setup_slow_clkout();
            break;
        case 2: // PPS Ouput.
            setup_slow_clkout();
            break;
    }
    if (!HarpCore::is_muted())
        HarpCore::send_harp_reply(WRITE, msg.header.address);
}

void update_app_state()
{
    // Update the state of ConnectedDevices.
    uint16_t old_port_raw = app_regs.ConnectedDevices;
    uint32_t port_raw = gpio_get_all();
    // CHAN[7:0] = GPIO[23:16]; CHAN[8:15] = GPIO[15:8]. See schematic.
    port_raw >>= 8;
    port_raw = ((port_raw & 0x000000FF) << 8) | ((port_raw & 0x0000FF00) >> 8);
    app_regs.ConnectedDevices = uint16_t(port_raw);
    // If port state changed, dispatch event from ConnectedDevices app reg (32).
    // TODO: add hysteresis.
    if ((old_port_raw != app_regs.ConnectedDevices) && !HarpCore::is_muted())
        HarpCore::send_harp_reply(EVENT, APP_REG_START_ADDRESS);

    // Nothing to do if we're not instructed to emit periodic msgs.
    if (app_regs.CounterFrequencyHz == 0)
        return;
    uint32_t curr_time_us = HarpCore::harp_time_us_32();
    // Handle edge-case where we went from not-synced to synced.
    if (HarpSynchronizer::has_synced() && !was_synced)
    {
        was_synced = true;
        last_msg_emit_time_us = HarpCore::harp_time_us_32(); // reset counter.
    }
    // Handle periodic counter msg dispatch.
    if (int32_t(curr_time_us - last_msg_emit_time_us) >= counter_interval_us)
    {
        last_msg_emit_time_us += counter_interval_us;
        app_regs.Counter += 1;
        // Issue EVENT from Counter register.
        if (!HarpCore::is_muted())
            HarpCore::send_harp_reply(EVENT, APP_REG_START_ADDRESS + 1);
    }
}

void reset_aux_fn()
{
    cleanup_pps_output();
    cleanup_slow_clkout(); // Cleanup lingering SLOW CLKout behavior.
}

void reset_app()
{
    counter_interval_us = 0;
    app_regs.Counter = 0;
    app_regs.CounterFrequencyHz = 0;
    app_regs.AuxPortFn = 1; // Start with SLOW CLKout fn enabled.
    // Note: external sync state does not change.
    reset_aux_fn();
    setup_harp_clkout();
    setup_slow_clkout(); // Start with SLOW CLKout fn enabled.
}

// Define "specs" per-register
RegSpecs app_reg_specs[REG_COUNT]
{
    {(uint8_t*)&app_regs.ConnectedDevices, sizeof(app_regs.ConnectedDevices), U16}, // 32
    {(uint8_t*)&app_regs.Counter, sizeof(app_regs.Counter), U32}, // 33
    {(uint8_t*)&app_regs.CounterFrequencyHz, sizeof(app_regs.CounterFrequencyHz), U16}, // 34
    {(uint8_t*)&app_regs.AuxPortFn, sizeof(app_regs.AuxPortFn), U8}, // 35
    // More specs here if we add additional registers.
};

RegFnPair reg_handler_fns[REG_COUNT]
{
    {HarpCore::read_reg_generic, HarpCore::write_reg_generic},          // 32
    {HarpCore::read_reg_generic, HarpCore::write_reg_generic},          // 33
    {HarpCore::read_reg_generic, write_counter_frequency_hz},           // 34
    {HarpCore::read_reg_generic, write_aux_port_fn},                    // 35
    // More handler function pairs here if we add additional registers.
};


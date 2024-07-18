#include <white_rabbit_app.h>

// Apply starting values.
uint32_t counter_interval_us = 0;
uint32_t last_msg_emit_time_us;
bool was_synced = false;

app_regs_t app_regs;

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

void reset_app()
{
    counter_interval_us = 0;
    app_regs.Counter = 0;
    app_regs.CounterFrequencyHz = 0;
    app_regs.AuxPortFn = 0;
    // Note: external sync state does not change.
}

// Define "specs" per-register
RegSpecs app_reg_specs[REG_COUNT]
{
    {(uint8_t*)&app_regs.ConnectedDevices, sizeof(app_regs.ConnectedDevices), U16}, // 32
    {(uint8_t*)&app_regs.Counter, sizeof(app_regs.Counter), U32}, // 33
    {(uint8_t*)&app_regs.CounterFrequencyHz, sizeof(app_regs.CounterFrequencyHz), U32}, // 34
    {(uint8_t*)&app_regs.AuxPortFn, sizeof(app_regs.AuxPortFn), U32}, // 34
    // More specs here if we add additional registers.
};

RegFnPair reg_handler_fns[REG_COUNT]
{
    {HarpCore::read_reg_generic, HarpCore::write_reg_generic},          // 32
    {HarpCore::read_reg_generic, HarpCore::write_reg_generic},          // 33
    {HarpCore::read_reg_generic, write_counter_frequency_hz},           // 34
    {HarpCore::read_reg_generic, HarpCore::write_reg_generic},          // 35
    // More handler function pairs here if we add additional registers.
};


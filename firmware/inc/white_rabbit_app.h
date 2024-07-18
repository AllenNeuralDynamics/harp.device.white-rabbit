#ifndef WHITE_RABBIT_APP_H
#define WHITE_RABBIT_APP_H
#include <pico/stdlib.h>
#include <cstring>
#include <config.h>
#include <harp_message.h>
#include <harp_core.h>
#include <reg_types.h>
#include <harp_c_app.h>
#include <harp_synchronizer.h>
#include <core_registers.h>
#include <pico/divider.h> // for fast hardware division: b/a=div_u32u32(b, a)
#ifdef DEBUG
    #include <stdio.h>
    #include <cstdio> // for printf
#endif
//#include <hardware/dma.h>
//#include <hardware/timer.h>
//#include <uart_nonblocking.h>
//#include <utility>
//#include <cstring>


// Create device name.
extern const uint16_t who_am_i;
extern const uint8_t hw_version_major;
extern const uint8_t hw_version_minor;
extern const uint8_t assembly_version;
extern const uint8_t harp_version_major;
extern const uint8_t harp_version_minor;
extern const uint8_t fw_version_major;
extern const uint8_t fw_version_minor;
extern const uint16_t serial_number;

// Setup for Harp App
const size_t REG_COUNT{4};

// pre-computed value for when to emit periodic counter msgs.
extern uint32_t counter_interval_us;
extern uint32_t last_msg_emit_time_us;
extern bool was_synced;

#pragma pack(push, 1)
struct app_regs_t
{
    volatile uint16_t ConnectedDevices; // bitmask where each bit represents
                                         // whether a device is
                                         // connected (1) or disconnected (0)
                                         // to the corresponding clock output
                                         // channel.
    volatile uint32_t Counter;
    volatile uint16_t CounterFrequencyHz;
    volatile uint8_t AuxPortFn; // bit[0] = 1 --> slow uart
                                // bit[1] = 1 --> PPS
    // More app "registers" here.
};
#pragma pack(pop)

extern app_regs_t app_regs;
extern RegSpecs app_reg_specs[REG_COUNT];
extern RegFnPair reg_handler_fns[REG_COUNT];
//extern HarpCApp& app;

void read_from_port(uint8_t reg_address);

/**
 * \brief update the app state. Called in a loop in the Harp App.
 */
void update_app_state();

/**
 * \brief reset the app.
 */
void reset_app();

#endif // WHITE_RABBIT_APP_H

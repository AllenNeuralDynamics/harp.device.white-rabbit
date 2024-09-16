#ifndef WHITE_RABBIT_APP_H
#define WHITE_RABBIT_APP_H
#include <pico/stdlib.h>
#include <cstring>
#include <utility>
#include <config.h>
#include <harp_message.h>
#include <harp_core.h>
#include <reg_types.h>
#include <harp_c_app.h>
#include <harp_synchronizer.h>
#include <uart_nonblocking.h>
#include <core_registers.h>
#include <pico/divider.h> // for fast hardware division.
#ifdef DEBUG
    #include <stdio.h>
    #include <cstdio> // for printf
#endif


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
const size_t REG_COUNT{5};

// pre-computed value for when to emit periodic counter msgs.
extern uint32_t counter_interval_us;
extern uint64_t last_msg_emit_time_us;
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
    volatile uint8_t AuxPortFn; // 0 --> no behavior.
                                // 1 --> output auxiliary uart time msg
                                //       (4-bytes, reset behavior) with baud
                                //       rate specified in AuxBaudRate register.
                                //       Falling edge of the message starting
                                //       bit happens on the start of the second
                                //       encoded in the msg.
                                // 2 --> output PPS signal.
    uint32_t AuxBaudRate;   // Set baud rate (in bps) for auxiliary UART.
    // More app "registers" here.
};
#pragma pack(pop)


// Note: literals, arrays, and functions accessed inside interrupt are placed in
//  RAM to avoid delay associated with flash access.
extern app_regs_t app_regs;
extern RegSpecs app_reg_specs[REG_COUNT];
extern RegFnPair reg_handler_fns[REG_COUNT];

// Harp CLKout Double Buffer Setup
extern volatile int harp_clkout_dma_chan;

extern int32_t harp_clkout_alarm_num;
extern uint32_t harp_clkout_irq_number;

// Ping-Pong Buffer for Harp CLKout message.
extern volatile uint8_t harp_time_msg_a[6];
extern volatile uint8_t harp_time_msg_b[6];

// Pointers for swapping buffers.
extern volatile uint8_t *dispatch_buffer;
extern volatile uint8_t *load_buffer;

// AUX CLKout Double Buffer Setup
extern volatile int aux_clkout_dma_chan;

extern int32_t aux_clkout_alarm_num;
extern uint32_t aux_clkout_iqr_number;

// Ping-Pong Buffer for AUX Clkout.
extern volatile uint32_t aux_clkout_seconds_a;
extern volatile uint32_t aux_clkout_seconds_b;

// Pointers for swapping buffers.
extern volatile uint32_t *dispatch_second;
extern volatile uint32_t *load_second;

// PPS Alarm/IRQ resources.
extern int32_t pps_output_alarm_num;
extern uint32_t pps_output_iqr_number;

/**
 * \brief Setup periodic Harp Clkout dispatch.
 */
void setup_harp_clkout();

/*
 * \brief Dispatch the time message to all 16 output channels and reschedule
 *  the next periodic dispatch.
 * \warning called inside of an interrupt.
 */
void dispatch_and_reschedule_harp_clkout();

/**
 * \brief Setup AuxFn behavior where we dispatch the current time once per
 *  second at the start of the whole second at a baud rate specified in the app
    registers.
*/
void setup_aux_clkout();

/*
 * \brief Dispatch the time on the Auxiliary output and reschedule the next
 *  dispatch.
 * \warning called inside of an interrupt.
 */
void dispatch_and_reschedule_aux_clkout();

/*
 * \brief unclaim resources to produce the slow clkout signal.
 */
void cleanup_aux_clkout();

/*
 * \brief Setup AuxFn behavior to toggle the AuxPort GPIO pin on the whole
 *  second (in Harp time).
 *  dispatch.
 * \warning called inside of an interrupt.
 */
void setup_pps_output();

/*
 * \brief Toggle the GPIO pin on the whole second (in Harp time) and reschedule
 *   the next dispatch.
 * \warning called inside of an interrupt.
 */
void update_pps_output();

/*
 * \brief unclaim resources to generate the PPS Signal.
 */
void cleanup_pps_output();

void reset_aux_fn();

void write_counter_frequency_hz(msg_t& msg);

void write_aux_port_fn(msg_t& msg);

void write_aux_baud_rate(msg_t& msg);

/**
 * \brief update the app state. Called in a loop in the Harp App.
 */
void update_app_state();

/**
 * \brief reset the app.
 */
void reset_app();

#endif // WHITE_RABBIT_APP_H

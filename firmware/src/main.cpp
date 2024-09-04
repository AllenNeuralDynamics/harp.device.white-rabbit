#include <harp_synchronizer.h>
#include <harp_core.h>
#include <harp_c_app.h>
#include <core_registers.h>
#include <reg_types.h>
#include <config.h>
#include <uart_nonblocking.h>
#include <white_rabbit_app.h>
#include <cstring>
#include <pico/unique_id.h>

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

// Core0 main.
int main()
{
    // Create Harp App.
    HarpCApp& app = HarpCApp::init(who_am_i, hw_version_major, hw_version_minor,
                                   assembly_version,
                                   harp_version_major, harp_version_minor,
                                   fw_version_major, fw_version_minor,
                                   serial_number, "White Rabbit",
                                   (const uint8_t*)GIT_HASH, // in CMakeLists.txt.
                                   &app_regs, app_reg_specs,
                                   reg_handler_fns, REG_COUNT, update_app_state,
                                   reset_app);
    // Setup GPIO pin.
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
    // Init Synchronizer.
    HarpSynchronizer& sync = HarpSynchronizer::init(HARP_UART, HARP_CLKIN_PIN);
    app.set_synchronizer(&sync);
    // Setup Harp CLKout periodic dispatch.
    setup_harp_clkout();
    // If we enable debug msgs, we cannot use the slow output.
#if defined(DEBUG)
#warning "Auxilary Port functionality will be overwritten to dispatch UART DEBUG msgs at 921600bps."
    stdio_uart_init_full(SLOW_SYNC_UART, 921600, SLOW_SYNC_CLKOUT_PIN, -1); // TX only.
    printf("Hello, from an RP2040!\r\n");
    printf("Curr Harp seconds: %lu[s] | 1st alarm (Harp) time %lu[us] | 1st alarm (sys time) %lu[us].\r\n",
           curr_harp_seconds, next_msg_harp_time_us_32, alarm_time_us);
    printf("Scheduled first alarm.\r\n");
#endif
    while(true)
        app.run();
}


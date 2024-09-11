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
const uint8_t assembly_version = 0;
const uint8_t harp_version_major = 0;
const uint8_t harp_version_minor = 0;
const uint16_t serial_number = 0;

// Core0 main.
int main()
{
#if defined(DEBUG)
#warning "Auxilary Port functionality will be overwritten to dispatch UART DEBUG msgs at 921600bps."
    stdio_uart_init_full(AUX_SYNC_UART, 921600, AUX_PIN, -1); // TX only.
    printf("Hello, from an RP2040!\r\n");
#endif
    // Init Synchronizer. Do this first since the WhiteRabbit app will attempt
    // to initialize the same hardware (HARP_UART) and skip if already
    // initialized.
    HarpSynchronizer& sync = HarpSynchronizer::init(HARP_UART, HARP_CLKIN_PIN);
    // Create Harp App.
    HarpCApp& app = HarpCApp::init(HARP_DEVICE_ID,
                                   HW_VERSION_MAJOR, HW_VERSION_MINOR,
                                   assembly_version,
                                   harp_version_major, harp_version_minor,
                                   FW_VERSION_MAJOR, FW_VERSION_MINOR,
                                   serial_number, "White Rabbit",
                                   (const uint8_t*)GIT_HASH, // in CMakeLists.txt.
                                   &app_regs, app_reg_specs,
                                   reg_handler_fns, REG_COUNT, update_app_state,
                                   reset_app);
    app.set_synchronizer(&sync);
    // TODO: try waiting until synchronized.
    // If we enable debug msgs, we cannot use the slow output.
    reset_app();
    while(true)
        app.run();
}


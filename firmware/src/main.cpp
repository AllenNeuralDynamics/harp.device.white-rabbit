#include "config.h"
#include "core_registers.h"
#include "harp_core.h"
#include "harp_c_app.h"
#include "harp_synchronizer.h"
#include "reg_types.h"
#include "pico/unique_id.h"
#include "uart_nonblocking.h"
#include "white_rabbit_app.h"
#include <cstring>

const uint8_t interface_hash[20] = INTERFACE_HASH;

void set_harp_core_led(bool led_state)
{gpio_put(HARP_CORE_LED_PIN, led_state);}

bool get_harp_core_led_state()
{ return gpio_get(HARP_CORE_LED_PIN);}

// Core0 main.
int main()
{
#if defined(DEBUG)
#warning "Auxilary Port functionality will be overwritten to dispatch UART DEBUG msgs at 921600bps."
    stdio_uart_init_full(AUX_SYNC_UART, 921600, AUX_PIN, -1); // TX only.
    printf("Hello, from an RP2040!\r\n");
#endif
    // Setup OP_LED
    gpio_init(HARP_CORE_LED_PIN);
    gpio_set_dir(HARP_CORE_LED_PIN, GPIO_OUT);
    gpio_put(HARP_CORE_LED_PIN, 0);
    // Init Synchronizer. Do this first since the WhiteRabbit app will attempt
    // to initialize the same hardware (HARP_UART) and skip if already
    // initialized.
    HarpSynchronizer& sync = HarpSynchronizer::init(HARP_UART, HARP_CLKIN_PIN);
    // Create Harp App.
    HarpCApp& app = HarpCApp::init(HARP_DEVICE_ID, FW_VERSION, HW_VERSION,
                                   "White Rabbit",
                                   (uint8_t*)GIT_HASH, interface_hash,
                                   app_reg_specs, APP_REG_COUNT,
                                   update_app_state, reset_app);
    app.set_op_led_fns(set_harp_core_led, get_harp_core_led_state);
    app.set_is_clock_generator(true);

    app.set_synchronizer(&sync);
    // TODO: try waiting until synchronized.
    // If we enable debug msgs, we cannot use the slow output.
    reset_app();
    while(true)
        app.run();
}

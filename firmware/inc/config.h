#ifndef CONFIG_H
#define CONFIG_H


#define UART_TX_PIN (0)
#define HARP_CORE_LED_PIN (25)

#define HARP_DEVICE_ID (1404)

#define HARP_UART (uart1)
// Baud rate and offset defined per spec.
// https://harp-tech.org/protocol/SynchronizationClock.html
#define HARP_SYNC_BAUDRATE (100000UL)
#define HARP_CLKOUT_PIN (4)
#define HARP_CLKIN_PIN (5)
#define HARP_SYNC_START_OFFSET_US (-572) // Offset from spec to account for
                                         // transmission time.

#define MAX_EVENT_FREQUENCY_HZ (1000)

#define SLOW_SYNC_UART (uart0)
#define SLOW_SYNC_BAUDRATE (1000UL)
#define SLOW_SYNC_CLKOUT_PIN (0)
#define SLOW_SYNC_START_OFFSET_US (0) // Offset from spec to account for
                                      // transmission time.

#define LED_PIN (25)


// Doesnt work yet:
//#define USBD_MANUFACTURER "The Allen Institute for Neural Dynamics"
//#define USBD_PRODUCT "Harp.Device.Lickety-Split"

#endif // CONFIG_H

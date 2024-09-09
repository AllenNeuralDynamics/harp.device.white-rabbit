#ifndef CONFIG_H
#define CONFIG_H


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

#define AUX_SYNC_UART (uart0)
#define AUX_SYNC_DEFAULT_BAUDRATE (1000UL)
#define MIN_AUX_SYNC_BAUDRATE (40U) // Aux Baud rate should be faster than this
                                    // minimum baud rate, or we will not have
                                    // enough time to emit a full 4-byte
                                    // (+1 start and 1 stop bit) message at 1Hz.
#define AUX_PIN (0)
#define AUX_SYNC_START_OFFSET_US (0) // Offset from spec to account for
                                      // transmission time.

#define LED0_PIN (24)
#define LED1_PIN (25)


// Doesnt work yet:
//#define USBD_MANUFACTURER "The Allen Institute for Neural Dynamics"
//#define USBD_PRODUCT "Harp.Device.Lickety-Split"

#endif // CONFIG_H

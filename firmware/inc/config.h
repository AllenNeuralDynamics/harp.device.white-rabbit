#ifndef CONFIG_H
#define CONFIG_H
#include <cstdint>
#include "core_registers.h" // for semver_t

inline constexpr semver_t HW_VERSION = {1, 0, 0};
inline constexpr semver_t FW_VERSION = {0, 1, 4};

inline constexpr uint32_t HARP_CORE_LED_PIN = 25;

inline constexpr size_t HARP_DEVICE_ID = 1404;

#define HARP_UART (uart1)
// Baud rate and offset defined as constants in the core.
// https://harp-tech.org/protocol/SynchronizationClock.html
#define HARP_CLKOUT_PIN (4)
#define HARP_CLKIN_PIN (5)
#define HARP_SYNC_START_OFFSET_US (-1172) // Offset from spec to account for
                                          // transmission time.
                                          // Spec is: next whole second occurs
                                          // 672 us after the start of the last
                                          // byte. We offset another 500us
                                          // (5[bytes]*100[kbps]) to make the
                                          // above statement meet the spec.

inline constexpr size_t MAX_EVENT_FREQUENCY_HZ = 1000;

#define AUX_SYNC_UART (uart0)
inline constexpr size_t AUX_SYNC_DEFAULT_BAUDRATE  = 1000;

// Aux Baud rate should be faster than this minimum baud rate, or we will not
// have enough time to emit a full 4-byte (+1 start and 1 stop bit) message at 1Hz.
inline constexpr size_t MIN_AUX_SYNC_BAUDRATE = 40;
// Aux Baud rate should be slower than this value.
inline constexpr size_t MAX_AUX_SYNC_BAUDRATE = 1'000'000;

inline constexpr uint32_t AUX_PIN = 0;
inline constexpr uint32_t AUX_SYNC_START_OFFSET_US = 0;

#define LED0_PIN (24)
#define LED1_PIN (25)

#endif // CONFIG_H

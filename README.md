An RP2040-based 16-output [Harp](https://github.com/harp-tech/protocol)-compliant Clock Synchronizer. 

This device can serve as a drop-in replacement for the [Harp-Tech Clock Synchronizer](https://github.com/harp-tech/device.clocksynchronizer) with added capabilities.

![PCB](./assets/pics/white_rabbit.png)

## Features
* 16 output channels for distributing clocks to other devices
* 1 input channel for receiving and synchronizing to another clock source
* PPS output
* serial output of the time at user-specifiable baud-rate
* "Used channel detection." Device can identify which channels are in use.

## Auxiliary Output
This device features an auxiliary output that can either produce a *pulse-per-second* (PPS) or UART message at the start of the whole second.
This external signa enables Harp devices to additionally further synchronize with *non*-Harp devices.

### PPS Output
This device optionally outputs a 1[Hz] signal with a 50% duty cycle on the whole second.
![PPS](./assets/pics/pps.png)
Error from the nominal Harp time is < 1[us].

This feature is available on the AUX Port (3-pin terminal block).

### AUX UART Output
This device optionally outputs a the current time in seconds at the start of the whole second.
![AUX_UART](./assets/pics/aux_uart.png)
Error from the nominal Harp time is < 3[us].

![AUX_UART_ERROR](./assets/pics/aux_uart_specs.png)

This feature is available on the AUX Port (3-pin terminal block), and the baud rate is configurable via Harp Protocol (U32 in Register 36).

## PCBA Enclosure
For the enclosure design, see the companion [OnShape project](https://cad.onshape.com/documents/e58143a7c9dd2652647e9623/w/90e72faf89a0a2a445ca0911/e/b03806c0bc46a31dc8d5c2c5?renderMode=0&uiState=67be1ef78ee27a5b150b11dd).

## Developer Notes
In the firmware's **CMakeLists.txt**, adding (or uncommenting) the debug message below:
````cmake
add_definitions(-DDEBUG)
````
will *override* the Auxiliary port behavior to use the auxiliary uart for `printf`-style debug messages.
`PPS` and `AUX UART` features will not be available in debug mode.

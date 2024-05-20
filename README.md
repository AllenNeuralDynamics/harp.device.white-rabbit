An RP2040-based 16-output Harp Clock Synchronizer

This device can serve as a drop-in replacement for the [Harp-Tech Clock Synchronizer](https://github.com/harp-tech/device.clocksynchronizer) with added capabilities.

## Features
* 16 output channels for distributing clocks to other devices
* 1 input channel for receiving and synchronizing to another clock source
* PPS output
* serial output of the time at user-specifiable baud-rate
* "Used channel detection." Device can identify which channels are in use.

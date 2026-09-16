#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "harp>=0.5.0",
# ]
# ///

"""Enable periodic Counter events at a fixed frequency and print them as they arrive."""

import numpy as np
from harp import serial
from harp.device import core

import device as wr

# Use "COMx" on Windows, "/dev/ttyACM0" (or similar) on Linux.
PORT = "/dev/ttyACM0"

COUNTER_FREQUENCY_HZ = 60

with serial.open_device(wr, port=PORT) as device:

    print(f"Enabling periodic counter msgs at {COUNTER_FREQUENCY_HZ} Hz.")
    device.write(wr.CounterFrequencyHz, np.uint16(COUNTER_FREQUENCY_HZ))

    print("Waiting for events. Press CTRL-C to exit.")
    with device.subscribe(wr.Counter, lambda msg: print(msg)):
        try:
            while True:
                pass
        except KeyboardInterrupt:
            pass

    print("Disabling periodic counter msgs.")
    device.write(wr.CounterFrequencyHz, np.uint16(0))

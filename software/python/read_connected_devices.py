#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "harp>=0.5.0",
# ]
# ///

"""Read and print the currently-connected White Rabbit output channels."""

from harp import serial

import device as wr

# Use "COMx" on Windows, "/dev/ttyACM0" (or similar) on Linux.
PORT = "/dev/ttyACM0"

# Open the device. Passing the ``wr`` module validates the WHO_AM_I on open.
with serial.open_device(wr, port=PORT) as device:
    print("Reading connected devices.")
    connected_devices = device.read(wr.ConnectedDevices).payload
    print(f"Connected devices (bin): {int(connected_devices):016b}")

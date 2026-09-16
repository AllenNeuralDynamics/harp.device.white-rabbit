#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "harp>=0.5.0",
# ]
# ///

"""Configure the auxiliary port to emit a PPS (pulse-per-second) signal."""
import time

from harp import serial

import device as wr

# Use "COMx" on Windows, "/dev/ttyACM0" (or similar) on Linux.
PORT = "/dev/ttyACM0"

with serial.open_device(wr, port=PORT) as device:
    print("Enabling PPS. Press CTRL-C to exit.")
    device.write(wr.AuxPortMode, wr.AuxPortModePayload(wr.AuxPortModeConfig.PPS))
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        print("Disabling auxiliary port.")
        device.write(
            wr.AuxPortMode, wr.AuxPortModePayload(wr.AuxPortModeConfig.DISABLED)
        )

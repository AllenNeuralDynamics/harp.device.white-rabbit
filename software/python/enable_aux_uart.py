#!/usr/bin/env -S uv run --script
#
# /// script
# requires-python = ">=3.11"
# dependencies = [
#     "harp>=0.5.0",
# ]
# ///

"""Configure the auxiliary port as a Harp-clock UART output at a given baud rate."""

import numpy as np
from harp import serial
from harp.device import core

import device as wr

# Use "COMx" on Windows, "/dev/ttyACM0" (or similar) on Linux.
PORT = "/dev/ttyACM0"

BAUDRATE = 115200

with serial.open_device(wr, port=PORT) as device:
    print(f"Setting Aux UART baud rate to {BAUDRATE} bps.")
    device.write(wr.AuxPortBaudRate, np.uint32(BAUDRATE))

    print("Enabling Aux UART.")
    device.write(
        wr.AuxPortMode, wr.AuxPortModePayload(wr.AuxPortModeConfig.HARP_CLOCK)
    )

    print("Enabling Heartbeat. Press CTRL-C to exit.")
    device.write(
        core.OperationControl,
        core.OperationControlPayload(
            operation_mode=core.OperationMode.ACTIVE,
            heartbeat=core.EnableFlag.ENABLED,
        ),
    )
    try:
        with device.subscribe_all(lambda msg: print(msg)):
            while True:
                pass
    except KeyboardInterrupt:
        pass
    finally:
        print("Disabling Aux UART.")
        device.write(
            wr.AuxPortMode, wr.AuxPortModePayload(wr.AuxPortModeConfig.DISABLED)
        )

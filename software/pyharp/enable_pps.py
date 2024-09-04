#!/usr/bin/env python3
from enum import Enum
from pyharp.device import Device, DeviceMode
from pyharp.messages import HarpMessage
from pyharp.messages import MessageType
from pyharp.messages import CommonRegisters as Regs
from struct import *
import os
from time import sleep, perf_counter

class Regs(Enum):
    ConnectedDevices = 32   # U16
    Counter = 33            # U32
    CounterFrequencyHz = 34 # U16
    AuxPortFn = 35          # U8


# Open the device and print the info on screen
# Open serial connection and save communication to a file
if os.name == 'posix': # check for Linux.
    #device = Device("/dev/harp_device_00", "ibl.bin")
    device = Device("/dev/ttyACM0", "ibl.bin")
else: # assume Windows.
    device = Device("COM95", "ibl.bin")
start_time = perf_counter()
# Enable sensor message events at 1 Hz
print("Enabling PPS. Press CTRL-C to exit.")
reply = device.send(HarpMessage.WriteU16(Regs.AuxPortFn.value, 0b10).frame)
try:
    while True:
        pass
except KeyboardInterrupt:
    print("Disabling slow Uart.")
    reply = device.send(HarpMessage.WriteU16(Regs.AuxPortFn.value, 0).frame)
    device.disconnect()

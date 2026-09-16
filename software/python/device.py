# This file was automatically generated and should not be edited directly.
# To make changes, edit the device metadata and regenerate the interface.

import enum
from typing import Any, ClassVar

import numpy as np
from harp.protocol import (
    AnonymousPayload,
    BitMask,
    GroupMask,
    PayloadType,
    RegisterBase,
    RegisterU16,
    RegisterU32,
)
from harp.device.core import REGISTER_MAP as _CORE_REGISTER_MAP


__all__ = [
    "DEVICE_NAME",
    "WHO_AM_I",
    "ClockOutChannels",
    "AuxPortModeConfig",
    "ConnectedDevicesPayload",
    "AuxPortModePayload",
    "ConnectedDevices",
    "Counter",
    "CounterFrequencyHz",
    "AuxPortMode",
    "AuxPortBaudRate",
    "REGISTER_MAP",
]

DEVICE_NAME: str = "WhiteRabbit"
WHO_AM_I: int = 1404


class ClockOutChannels(enum.IntFlag):
    """Clock output channels"""

    CHANNEL0 = 0x1
    CHANNEL1 = 0x2
    CHANNEL2 = 0x4
    CHANNEL3 = 0x8
    CHANNEL4 = 0x10
    CHANNEL5 = 0x20
    CHANNEL6 = 0x40
    CHANNEL7 = 0x80
    CHANNEL8 = 0x100
    CHANNEL9 = 0x200
    CHANNEL10 = 0x400
    CHANNEL11 = 0x800
    CHANNEL12 = 0x1000
    CHANNEL13 = 0x2000
    CHANNEL14 = 0x4000
    CHANNEL15 = 0x8000


class AuxPortModeConfig(enum.IntEnum):
    """Auxiliary port available configuration"""

    DISABLED = 0
    HARP_CLOCK = 1
    PPS = 2


class ConnectedDevicesPayload(AnonymousPayload[np.uint16]):
    """Represents the payload of the ConnectedDevices register."""

    __value__: ClockOutChannels = BitMask(enum=ClockOutChannels)


class AuxPortModePayload(AnonymousPayload[np.uint8]):
    """Represents the payload of the AuxPortMode register."""

    __value__: AuxPortModeConfig = GroupMask(enum=AuxPortModeConfig, mask=0xFF)


class ConnectedDevices(RegisterBase[ClockOutChannels]):
    """The currently connected output channels. An event will be generated when any of the channels are connected or disconnected."""

    address: ClassVar[int] = 32
    payload_type: ClassVar[PayloadType] = PayloadType.U16
    payload_class = ConnectedDevicesPayload


class Counter(RegisterU32):
    """The counter value. This value is incremented at the frequency specified by CounterFrequencyHz. Write to force a counter value."""

    address: ClassVar[int] = 33


class CounterFrequencyHz(RegisterU16):
    """The frequency at which the counter is incremented. A value of 0 disables the counter."""

    address: ClassVar[int] = 34


class AuxPortMode(RegisterBase[AuxPortModeConfig]):
    """The function of the auxiliary port."""

    address: ClassVar[int] = 35
    payload_type: ClassVar[PayloadType] = PayloadType.U8
    payload_class = AuxPortModePayload


class AuxPortBaudRate(RegisterU32):
    """The baud rate, in bps, of the auxiliary port when in HarpClock mode."""

    address: ClassVar[int] = 36


REGISTER_MAP: dict[int, type[RegisterBase[Any]]] = {
    **_CORE_REGISTER_MAP,
    32: ConnectedDevices,
    33: Counter,
    34: CounterFrequencyHz,
    35: AuxPortMode,
    36: AuxPortBaudRate,
}

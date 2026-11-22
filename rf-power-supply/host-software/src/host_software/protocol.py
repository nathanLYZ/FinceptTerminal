from __future__ import annotations

from dataclasses import dataclass, field
from enum import IntEnum
from typing import Protocol


class RfState(IntEnum):
    IDLE = 0
    RAMP = 1
    RUN = 2
    FAULT = 3


@dataclass
class Telemetry:
    state: RfState = RfState.IDLE
    pf_w: float = 0.0
    pr_w: float = 0.0
    vswr: float = 1.0
    temp_pa_c: float = 25.0
    fault: int = 0
    cs_pos: int = 0
    cp_pos: int = 0


@dataclass
class Setpoints:
    power_w: float = 0.0
    rf_on: bool = False
    match_auto: bool = True
    clear_fault: bool = False


class RfTransport(Protocol):
    def apply(self, sp: Setpoints) -> None: ...
    def read_telemetry(self) -> Telemetry: ...
    def close(self) -> None: ...


@dataclass
def build_fc06(slave: int, reg: int, value: int) -> bytes:
    """Build a Modbus RTU Write-Single-Register frame (with CRC)."""
    body = bytes([(slave & 0xFF), 0x06, (reg >> 8) & 0xFF, reg & 0xFF,
                  (value >> 8) & 0xFF, value & 0xFF])
    crc = 0xFFFF
    for b in body:
        crc ^= b
        for _ in range(8):
            crc = (crc >> 1) ^ 0xA001 if (crc & 1) else (crc >> 1)
    return body + bytes([crc & 0xFF, (crc >> 8) & 0xFF])


class SimulatedPlant:
    """First-order plant used when no hardware is connected."""

    sp: Setpoints = field(default_factory=Setpoints)
    tm: Telemetry = field(default_factory=Telemetry)
    ramp_w: float = 0.0
    vbus: float = 0.0

    def apply(self, sp: Setpoints) -> None:
        self.sp = sp
        if sp.clear_fault:
            self.tm.fault = 0
            if self.tm.state == RfState.FAULT:
                self.tm.state = RfState.IDLE
            self.sp.clear_fault = False

    def step(self, dt: float = 0.1) -> Telemetry:
        if self.tm.fault:
            self.tm.state = RfState.FAULT
            self.ramp_w = 0.0
            self.tm.pf_w = 0.0
            self.tm.pr_w = 0.0
            return self.tm

        if not self.sp.rf_on or self.sp.power_w <= 0:
            self.tm.state = RfState.IDLE
            self.ramp_w = 0.0
            self.tm.pf_w = 0.0
            self.tm.pr_w = 0.0
            self.tm.vswr = 1.0
            return self.tm

        ramp = 200.0 * dt
        if self.ramp_w < self.sp.power_w:
            self.ramp_w = min(self.sp.power_w, self.ramp_w + ramp)
            self.tm.state = RfState.RAMP
        else:
            self.ramp_w = self.sp.power_w
            self.tm.state = RfState.RUN

        if self.sp.match_auto:
            self.tm.cs_pos += 1 if self.tm.cs_pos < 512 else 0
            self.tm.cp_pos += 1 if self.tm.cp_pos < 512 else 0

        mismatch = abs(self.tm.cs_pos - 512) + abs(self.tm.cp_pos - 512)
        gamma = min(0.95, mismatch / 800.0)
        self.tm.pf_w = self.ramp_w * (1.0 - gamma * 0.3)
        self.tm.pr_w = self.ramp_w * gamma * 0.3
        g = (self.tm.pr_w / self.tm.pf_w) ** 0.5 if self.tm.pf_w > 1 else 1.0
        self.tm.vswr = (1 + g) / max(1e-6, (1 - g))
        self.tm.temp_pa_c = 30.0 + self.tm.pf_w * 0.04
        return self.tm

    def read_telemetry(self) -> Telemetry:
        return self.step()

    def close(self) -> None:
        return None

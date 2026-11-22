from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path

# Allow running without install: python -m host_software.cli from src/
_SRC = Path(__file__).resolve().parents[1]
if str(_SRC) not in sys.path:
    sys.path.insert(0, str(_SRC))

from host_software.protocol import RfState, Setpoints, SimulatedPlant  # noqa: E402


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="RF-1K host console")
    p.add_argument("--sim", action="store_true", help="Use simulated plant")
    p.add_argument("--port", default="", help="Serial port for Modbus-RTU")
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--slave", type=int, default=1)
    p.add_argument("--tcp", default="", help="host:port for JSON TCP")
    return p


def print_status(tm) -> None:
    print(
        f"state={tm.state.name:5s}  Pf={tm.pf_w:7.1f} W  Pr={tm.pr_w:6.1f} W  "
        f"VSWR={tm.vswr:5.2f}  Tpa={tm.temp_pa_c:5.1f} C  fault=0x{tm.fault:04X}  "
        f"Cs={tm.cs_pos} Cp={tm.cp_pos}"
    )


def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if not args.sim and not args.port and not args.tcp:
        args.sim = True
        print("No interface specified — starting simulation mode.")

    if args.port or args.tcp:
        print("Hardware transports are scaffolded; use --sim until HAL/Modbus is wired.")
        print("Falling back to simulation.")
        args.sim = True

    plant = SimulatedPlant()
    sp = Setpoints(match_auto=True)

    print("RF-1K console. Commands: on <W>, set <W>, off, status, clear, match auto, quit")
    while True:
        try:
            line = input("> ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            break
        if not line:
            continue
        parts = line.split()
        cmd = parts[0].lower()

        if cmd in ("quit", "exit", "q"):
            break
        if cmd == "on":
            watts = float(parts[1]) if len(parts) > 1 else 100.0
            sp.power_w = watts
            sp.rf_on = True
        elif cmd == "set":
            sp.power_w = float(parts[1])
        elif cmd == "off":
            sp.rf_on = False
            sp.power_w = 0.0
        elif cmd == "clear":
            sp.clear_fault = True
        elif cmd == "match" and len(parts) > 1 and parts[1] == "auto":
            sp.match_auto = True
        elif cmd == "status":
            pass
        else:
            print("unknown command")
            continue

        plant.apply(sp)
        # Advance plant a few steps so ramp is visible
        tm = plant.tm
        for _ in range(5):
            tm = plant.read_telemetry()
            time.sleep(0.02)
        print_status(tm)
        sp.clear_fault = False

    plant.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

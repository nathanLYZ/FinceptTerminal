#!/usr/bin/env python3
"""Tiny CLI wrapping the same search as firmware (for bring-up scripts)."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "sim"))
from match_demo import cost, plant, try_axis, vswr  # noqa: E402


def run(cs: int, cp: int, target: float = 0.05) -> int:
    step = 32
    print(f"start Cs={cs} Cp={cp} |G|={cost(*plant(cs, cp)):.3f}")
    for i in range(80):
        g = cost(*plant(cs, cp))
        if g <= target:
            print(f"OK i={i} Cs={cs} Cp={cp} |G|={g:.3f} VSWR={vswr(g):.2f}")
            return 0
        cs, cp, g = try_axis(cs, cp, "cs", step)
        cs, cp, g = try_axis(cs, cp, "cp", step)
        if g < 0.15 and step > 2:
            step //= 2
        elif g > 0.4 and step < 64:
            step *= 2
        print(f"i={i:02d} |G|={g:.3f} VSWR={vswr(g):.2f} Cs={cs} Cp={cp} step={step}")
    print("FAIL timeout")
    return 1


def main() -> int:
    p = argparse.ArgumentParser(description="RF matcher software demo")
    p.add_argument("--cs", type=int, default=200)
    p.add_argument("--cp", type=int, default=800)
    p.add_argument("--target", type=float, default=0.05)
    args = p.parse_args()
    return run(args.cs, args.cp, args.target)


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""Coordinate-search matcher demo against a simple plant model."""

from __future__ import annotations


def plant(cs: int, cp: int, p_avail: float = 100.0) -> tuple[float, float]:
    err = (cs - 512) ** 2 + (cp - 512) ** 2
    gamma = err / (err + 50000.0)
    pf = p_avail * (1.0 - gamma)
    pr = p_avail * gamma
    return pf, pr


def cost(pf: float, pr: float) -> float:
    s = pf + pr
    if s < 1.0:
        return 1.0
    g2 = pr / s
    return g2**0.5


def vswr(g: float) -> float:
    if g >= 0.999:
        return 99.0
    return (1 + g) / (1 - g)


def try_axis(cs: int, cp: int, axis: str, step: int) -> tuple[int, int, float]:
    def at(a: int, b: int) -> float:
        return cost(*plant(a, b))

    c0 = at(cs, cp)
    if axis == "cs":
        cp_ = at(cs + step, cp)
        cm_ = at(cs - step, cp)
        if c0 <= cp_ and c0 <= cm_:
            return cs, cp, c0
        if cp_ <= cm_:
            return cs + step, cp, cp_
        return cs - step, cp, cm_
    else:
        cp_ = at(cs, cp + step)
        cm_ = at(cs, cp - step)
        if c0 <= cp_ and c0 <= cm_:
            return cs, cp, c0
        if cp_ <= cm_:
            return cs, cp + step, cp_
        return cs, cp - step, cm_


def main() -> None:
    cs, cp = 200, 800
    step = 32
    print(f"start Cs={cs} Cp={cp} |G|={cost(*plant(cs, cp)):.3f}")
    for i in range(60):
        g0 = cost(*plant(cs, cp))
        if g0 < 0.05:
            print(f"converged i={i} Cs={cs} Cp={cp} |G|={g0:.3f} VSWR={vswr(g0):.2f}")
            return
        cs, cp, _ = try_axis(cs, cp, "cs", step)
        cs, cp, g = try_axis(cs, cp, "cp", step)
        if g < 0.15 and step > 2:
            step //= 2
        elif g > 0.4 and step < 64:
            step *= 2
        if i % 5 == 0:
            print(f"i={i:02d} |G|={g:.3f} VSWR={vswr(g):.2f} Cs={cs} Cp={cp} step={step}")
    print("timeout", cs, cp, cost(*plant(cs, cp)))


if __name__ == "__main__":
    main()

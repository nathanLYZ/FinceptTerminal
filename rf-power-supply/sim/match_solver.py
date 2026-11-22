#!/usr/bin/env python3
"""Match a complex load at 13.56 MHz with an LC L-network."""

from __future__ import annotations

import math

F0 = 13.56e6
W = 2 * math.pi * F0
Z0 = 50.0


def series_l_shunt_c(l_h: float, c_f: float, zl: complex) -> complex:
    """RF_in -- L --+-- ZL
                    C
                   gnd
    """
    xl = 1j * W * l_h
    xc = 1.0 / (1j * W * c_f)
    return xl + 1.0 / (1.0 / xc + 1.0 / zl)


def series_c_shunt_c(cs: float, cp: float, zl: complex) -> complex:
    xs = 1.0 / (1j * W * cs)
    xp = 1.0 / (1j * W * cp)
    return xs + 1.0 / (1.0 / xp + 1.0 / zl)


def shunt_c_series_l(c_f: float, l_h: float, zl: complex) -> complex:
    """RF_in --+-- L -- ZL
               C
    """
    xl = 1j * W * l_h
    xc = 1.0 / (1j * W * c_f)
    return 1.0 / (1.0 / xc + 1.0 / (xl + zl))


def gamma(zin: complex, z0: float = Z0) -> complex:
    return (zin - z0) / (zin + z0)


def solve(zl: complex) -> None:
    best: tuple[float, str, float, float] = (1.0, "", 0.0, 0.0)

    # Topology A: series L + shunt C (common for low-R plasma loads)
    for i in range(80):
        l_h = (0.1e-6) + (8e-6 - 0.1e-6) * i / 79
        for j in range(80):
            c_f = (10e-12) + (1000e-12 - 10e-12) * j / 79
            g = abs(gamma(series_l_shunt_c(l_h, c_f, zl)))
            if g < best[0]:
                best = (g, "series-L + shunt-C", l_h, c_f)

    # Topology B: shunt C + series L
    for i in range(80):
        c_f = (10e-12) + (1000e-12 - 10e-12) * i / 79
        for j in range(80):
            l_h = (0.1e-6) + (8e-6 - 0.1e-6) * j / 79
            g = abs(gamma(shunt_c_series_l(c_f, l_h, zl)))
            if g < best[0]:
                best = (g, "shunt-C + series-L", c_f, l_h)

    # Topology C: dual vacuum capacitors (narrower coverage)
    for i in range(80):
        cs = (5e-12) + (1000e-12 - 5e-12) * i / 79
        for j in range(80):
            cp = (5e-12) + (1000e-12 - 5e-12) * j / 79
            g = abs(gamma(series_c_shunt_c(cs, cp, zl)))
            if g < best[0]:
                best = (g, "series-C + shunt-C", cs, cp)

    g, topo, a, b = best
    print(f"Load ZL = {zl} ohm @ {F0/1e6:.2f} MHz")
    print(f"Best topology: {topo}")
    print(f"|Γ| = {g:.4f}, VSWR ≈ {(1+g)/max(1e-6, 1-g):.3f}")
    if "L" in topo and "series-L + shunt-C" == topo:
        print(f"L = {a*1e6:.2f} µH, C = {b*1e12:.1f} pF")
        zin = series_l_shunt_c(a, b, zl)
    elif topo == "shunt-C + series-L":
        print(f"C = {a*1e12:.1f} pF, L = {b*1e6:.2f} µH")
        zin = shunt_c_series_l(a, b, zl)
    else:
        print(f"Cs = {a*1e12:.1f} pF, Cp = {b*1e12:.1f} pF")
        zin = series_c_shunt_c(a, b, zl)
    print(f"Zin = {zin.real:.2f}{zin.imag:+.2f}j ohm")


def main() -> None:
    solve(20 - 80j)
    print("---")
    solve(35 - 40j)


if __name__ == "__main__":
    main()

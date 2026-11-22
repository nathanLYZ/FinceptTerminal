# RF-1K KiCad Project

Open `rf1k.kicad_pro` with **KiCad 8+**.

## Sheets

| Sheet | Content |
|-------|---------|
| Root | Hierarchy overview |
| PSU | AC → PFC → LLC → 48–60 V |
| OSC | DDS + gate drive |
| PA | GaN PA + coupler + detectors |
| MATCH | Auto-match network |
| CTRL | MCU / protection / comm |
| HMI | Front panel |

Each child sheet currently exposes hierarchical labels for board interconnects and a design note text block. Draw circuitry against `../../schematics/block-notes.md` and the BOM.

## Suggested first drawing pass

1. CTRL: MCU minimal system + ADC front-ends
2. OSC+PA: bring-up at ≤50 W into 50 Ω dummy load
3. PSU power stage
4. MATCH mechanics + RF path

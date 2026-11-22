# 原理图分块说明（进入 EDA 前的设计笔记）

建议用 KiCad 8 / Altium 分六张原理图页：`PSU` `OSC` `PA` `MATCH` `CTRL` `HMI`。

## PSU.Sch

- 输入：保险丝 F1 10 A、压敏、X/Y 安规电容、共模电感。
- PFC：电感、升压 MOSFET、二极管、390 V 母线电容（注意纹波电流）。
- LLC：原边全桥或半桥、谐振腔、变压器、次级同步整流、输出滤波。
- 反馈：光耦 + TL431 / 隔离放大器至 LLC 控制器；CTRL 可通过 DAC 叠加微调参考。

## OSC.Sch

- TCXO → DDS；SPI 来自 CTRL。
- DDS 方波/正弦 → 比较整形 → 死区 CPLD 或 MCU 定时器逻辑。
- 隔离电源模块（+6 V/−3 V ×2）供上下管驱动。

## PA.Sch

- 半桥 GaN：去耦网络、自举/隔离、源极 Kelvin。
- 输出：串联谐振或低通 → 定向耦合器 → N 座。
- 检波：耦合器耦出口 → 衰减 → AD8307 → RC → CTRL ADC。

## MATCH.Sch

- RF 路径尽量短；Cs 串联、Cp 并联到地。
- 步进驱动放数字侧，长线用屏蔽双绞；编码器反馈差分。

## CTRL.Sch

- MCU 最小系统、ETH、RS-485、模拟调理（差分运放）、比较器保护锁存、接触器驱动。

## HMI.Sch

- 4.3" TFT SPI/RGB、EC11 编码器、急停、状态 LED。

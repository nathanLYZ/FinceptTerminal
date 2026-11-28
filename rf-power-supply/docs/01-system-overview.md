# 01 — 系统总览

## 1. 目标规格

| 项 | 指标 |
|----|------|
| 工作频率 | 13.56 MHz ±0.05% |
| 正向功率 | 0–1000 W，分辨率 1 W |
| 输出阻抗 | 50 Ω |
| PA 效率 | ≥ 85%（匹配良好） |
| 反射保护 | 可设阈值（默认 200 W，仿真放宽至 400 W） |
| 匹配 | 自动 L 型网络（电机驱动） |
| 控制 | STM32H743 + FreeRTOS，**软件全 C++17** |
| 通信 | RS-485 Modbus-RTU（可扩展以太网） |

## 2. 能量与控制流

```
AC 220V
   │
   ▼
┌──────────┐  48–60 VDC   ┌──────────┐  13.56 MHz   ┌──────────┐
│   PSU    │─────────────▶│    PA    │─────────────▶│  MATCH   │──▶ Load
│ PFC+LLC  │              │ GaN HB   │              │ L-network│
└──────────┘              └────▲─────┘              └────▲─────┘
                               │                         │
                          ┌────┴─────┐              步进位置
                          │   OSC    │                   │
                          │ DDS+Drv  │                   │
                          └────▲─────┘                   │
                               │                         │
                          ┌────┴─────────────────────────┴────┐
                          │     rf1k::System (C++ / FreeRTOS) │
                          │  tickPower / tickMatch / tickComm │
                          └───────────────────────────────────┘
```

## 3. 三层闭环

| 优先级 | 环路 | 节拍 | 软件入口 |
|--------|------|------|----------|
| 最高 | 硬保护 | μs | 硬件比较器 → GPIO 锁存（`IHardware::faultLatched`） |
| 高 | 功率环 | 1 kHz | `System::tickPower` |
| 中 | 匹配环 | 20–50 Hz | `System::tickMatch` → `match::Search::tick` |

## 4. 状态机

```
Idle ──rf_on && Pset>0──▶ Ramp ──到设定──▶ Run
 ▲                         │                │
 │                      fault            fault / off
 │                         ▼                ▼
 └────────────────────── Fault ◀────────────┘
                          │ clear
                          ▼
                        Idle
```

对应类型：`rf1k::State`（`include/rf1k/types.hpp`）。

## 5. 文档索引

| 文档 | 内容 |
|------|------|
| [02-software-architecture.md](02-software-architecture.md) | C++ 模块、任务、目录 |
| [03-hardware-design.md](03-hardware-design.md) | 分板与关键电路 |
| [04-modbus-protocol.md](04-modbus-protocol.md) | 寄存器与 RTU |
| [match-algorithm.md](match-algorithm.md) | 自动匹配算法 |

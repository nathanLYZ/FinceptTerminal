# RF-1K 系统概览（C++ 工程）

## 规格

| 项 | 值 |
|----|-----|
| 频率 | 13.56 MHz |
| 功率 | 0–1000 W |
| 输出 | 50 Ω |
| 控制 | STM32 + FreeRTOS（C++） |
| 通信 | Modbus-RTU / 可扩展 TCP |

## 软件分层

1. **Hard protection**（硬件比较器，μs）  
2. **System::tickPower** — 状态机 + PID 功率环（1 kHz）  
3. **System::tickMatch** — `match::Search` 自动匹配（20–50 Hz）  
4. **System::tickComm** — Modbus holding ↔ setpoints / telemetry  

## 状态机

`Idle → Ramp → Run`，故障 → `Fault`（清故障后回 Idle）。

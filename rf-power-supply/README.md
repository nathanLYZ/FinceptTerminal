# RF-1K — 13.56 MHz / 1 kW 射频电源参考设计

工业级 **13.56 MHz ISM** 射频电源软硬件参考设计，面向等离子体刻蚀、溅射、感应耦合等负载。本仓库给出可落地的系统架构、硬件分板、固件控制环与上位机协议，便于直接进入原理图与样机阶段。

## 目标规格

| 项目 | 指标 |
|------|------|
| 工作频率 | 13.56 MHz ±0.05%（可软件微调） |
| 正向功率 | 0–1000 W，连续可调，分辨率 1 W |
| 输出阻抗 | 50 Ω，N 型 / 7/16 DIN |
| 谐波抑制 | 二次谐波 ≤ −40 dBc（带后置滤波器） |
| 效率（PA 级） | ≥ 85% @ 额定功率、匹配良好 |
| 反射功率保护 | 可设定阈值（默认 20% 或 200 W） |
| 匹配 | 自动 L 型匹配网络（电机驱动真空电容） |
| 控制接口 | RS-485 Modbus-RTU、以太网 TCP、模拟 0–10 V / 4–20 mA |
| 冷却 | 强制风冷；可选水冷冷板 |
| 供电 | 单相 AC 220 V ±10%，50/60 Hz |

## 目录结构

```
rf-power-supply/
├── docs/                 # 系统设计文档
├── hardware/             # 原理图分块、BOM、PCB 约束
├── firmware/             # STM32H743 固件骨架
├── host-software/        # Python 上位机 / 仿真控制台
└── sim/                  # 匹配网络与功率环仿真脚本
```

## 快速阅读顺序

1. [系统总览](docs/01-system-overview.md) — 分板、信号流、安全链
2. [硬件设计](docs/02-hardware-design.md) — 各板电路要点与关键器件
3. [固件设计](docs/03-firmware-design.md) — 任务划分、功率环、匹配算法
4. [通信协议](docs/05-interfaces-protocol.md) — Modbus 寄存器与 TCP JSON
5. [上位机](docs/04-host-software.md) — 控制台与联调流程
6. [STM32 移植](docs/06-stm32-porting.md) — FreeRTOS 任务 / CubeMX / KiCad

## 主机验证

```bash
cd firmware && make test          # 仿真 + Modbus + STM32 mock 编译
cd firmware && make stm32-check   # 仅 STM32 驱动链接检查
```

STM32：打开 `firmware/target/stm32/RF1K.ioc`（CubeMX）→ 见该目录 README。  
KiCad 8+：`hardware/kicad/rf1k/rf1k.kicad_pro`

## 硬件分板

| 板卡 | 功能 |
|------|------|
| PSU | PFC + LLC / 全桥隔离 DC 总线（约 48–60 V） |
| OSC | DDS 激励、死区驱动、门极隔离 |
| PA | GaN 半桥 Class-D、低通滤波、定向耦合器 |
| MATCH | 双真空电容 L 网络、步进电机、位置反馈 |
| CTRL | STM32H743、ADC、保护逻辑、通信、HMI |
| HMI | 前面板 TFT + 旋钮 / 急停 |

## 软件栈

- **固件**：C11，FreeRTOS，CMSIS-DSP（功率环 / 匹配）
- **上位机**：Python 3.11+（Modbus / TCP 客户端 + CLI）
- **仿真**：Python（反射系数圆图、L 匹配求解、功率环阶跃响应）

## 安全声明

本设计为工程参考，涉及高压、射频与等离子体负载。量产前必须完成：绝缘耐压、泄漏电流、EMC（CISPR 11 / IEC 61000）、射频防护与急停联锁验证。未经验证不得接入真人可接触环境或产线。

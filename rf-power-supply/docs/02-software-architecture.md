# 02 — 软件架构（C++17）

本工程**不再使用 C 固件或 Python 上位机**；主机仿真与板级逻辑均为 C++。

## 1. 目录

```
rf-power-supply/
├── include/rf1k/
│   ├── types.hpp              # State / Fault / Setpoints / Telemetry / Pid
│   ├── metrics.hpp            # Γ、VSWR
│   ├── pid.hpp
│   ├── modbus.hpp
│   ├── system.hpp             # 总控 System
│   ├── match/                 # 自动匹配 Search / IActuator
│   └── platform/
│       ├── hw_port.hpp        # IHardware
│       └── freertos_app.hpp   # AppTasks
├── src/
│   ├── system.cpp, pid.cpp, metrics.cpp, modbus.cpp
│   ├── match/                 # Search 实现
│   ├── platform/              # StubPlant、AppTasks
│   └── apps/                  # rf1k_sim、rf1k_cli、test_modbus
├── docs/
├── Makefile
└── CMakeLists.txt
```

## 2. 核心类

```
IHardware  ◀── StubPlant / BoardHw
                │
match::IActuator◀──┘ (可同一对象)
                │
                ▼
           rf1k::System
           ├─ tickPower(dt)   状态机 + PID → 母线/DDS/RF_EN
           ├─ tickMatch()     match::Search
           └─ tickComm()      ModbusMap ↔ Setpoints/Telemetry
```

| 类 | 职责 |
|----|------|
| `rf1k::System` | 电源总控 |
| `rf1k::ModbusMap` | Holding/Input 与 RTU 帧 |
| `rf1k::match::Search` | 坐标交替自动匹配 |
| `rf1k::IHardware` | ADC/DAC/DDS/步进/联锁 |
| `rf1k::AppTasks` | FreeRTOS 创建 Power/Match/Comm 任务 |

## 3. FreeRTOS 任务映射

| 任务 | 周期 | 调用 |
|------|------|------|
| Power | 1 ms | `sys.tickPower(0.001f)` |
| Match | 20 ms | `sys.tickMatch()` |
| Comm | 10 ms | `sys.tickComm()` + UART 收发 |

主机无调度器时用 `AppTasks::runHost(sys, ms)` 或 `rf1k_sim` 内嵌循环。

编译宏：

- 主机：默认，不定义 `RF_USE_FREERTOS`
- 板级：`-DRF_USE_FREERTOS`，链接真实 FreeRTOS，实现 `IHardware`

## 4. 功率环要点

- 执行器：母线电压（主）+ DDS 幅度（辅，近满幅）
- 误差：`e = P_ramp - P_f`
- 软启动：`P_ramp` 按 `ramp_w_s` 爬升
- 高反射：`P_r` 超阈 → `Fault::HighRefl`；匹配过程中可对 `P_ramp` 软折返

实现：`src/system.cpp`、`src/pid.cpp`。

## 5. 匹配环要点

见 [match-algorithm.md](match-algorithm.md)。由 `System::tickMatch` 在 Ramp/Run 且 `match_auto` 时驱动。

## 6. 测试用例

| 目标 | 文件 | 覆盖 |
|------|------|------|
| `test_modbus` | `src/apps/test_modbus.cpp` | RTU 写 Holding / 读 Input + CRC |
| `test_unit` | `src/apps/test_unit.cpp` | Γ/VSWR、PID、匹配收敛、System 500 W、反射故障 |
| `rf1k_sim` | `src/apps/rf1k_sim.cpp` | 整机 8 s 冒烟（匹配 + 稳功率） |

```bash
cd rf-power-supply && make test
# → test_modbus + test_unit + rf1k_sim
```

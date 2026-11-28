# RF-1K — 13.56 MHz / 1 kW 射频电源（纯 C++）

整包软件为 **C++17**：功率环、状态机、故障、Modbus、自动匹配、FreeRTOS 任务与主机仿真。  
设计文档已与代码对齐，见下方「文档」。

## 架构

```
AC → PSU → PA → MATCH → Load
              ↑
         rf1k::System (C++)
    tickPower @ 1 kHz
    tickMatch @ 50 Hz
    tickComm  @ 100 Hz
```

| 模块 | 路径 |
|------|------|
| 类型 / PID / 指标 | `include/rf1k/` |
| 系统总控 | `include/rf1k/system.hpp` · `src/system.cpp` |
| Modbus RTU | `include/rf1k/modbus.hpp` · `src/modbus.cpp` |
| 匹配器 | `include/rf1k/match/` |
| 硬件端口 | `include/rf1k/platform/hw_port.hpp` · `src/platform/stub_plant.hpp` |
| FreeRTOS 任务 | `include/rf1k/platform/freertos_app.hpp` |

## 文档

| 文档 | 说明 |
|------|------|
| [docs/01-system-overview.md](docs/01-system-overview.md) | 规格、能量流、三层闭环 |
| [docs/02-software-architecture.md](docs/02-software-architecture.md) | C++ 类图、任务、目录 |
| [docs/03-hardware-design.md](docs/03-hardware-design.md) | 分板与匹配硬件 |
| [docs/04-modbus-protocol.md](docs/04-modbus-protocol.md) | Holding / Input 寄存器 |
| [docs/match-algorithm.md](docs/match-algorithm.md) | 自动匹配算法 |

## 构建

```bash
cd rf-power-supply
make test
```

```bash
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++
cmake --build build && ctest --test-dir build
```

## 主机仿真

```bash
./build/rf1k_sim          # 500 W + 自动匹配
./build/rf1k_cli          # 交互：on 300 / status / off
```

## FreeRTOS 板上接入

```cpp
static rf1k::BoardHw hw;          // 实现 IHardware
static rf1k::BoardMatch match_hw; // 或同一对象实现 match::IActuator
static rf1k::System sys(hw, match_hw);

void start() {
  sys.init();
  rf1k::AppTasks::start(sys);     // 需 -DRF_USE_FREERTOS
  vTaskStartScheduler();
}
```

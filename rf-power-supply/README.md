# RF-1K — 13.56 MHz / 1 kW 射频电源（纯 C++）

整包软件为 **C++17**：功率环、状态机、故障、Modbus、自动匹配、FreeRTOS 任务骨架与主机仿真。

## 架构

```
AC → PSU → PA → MATCH → Load
              ↑
         System (C++)
    tickPower @ 1 kHz
    tickMatch @ 50 Hz
    tickComm  @ 100 Hz
```

| 模块 | 路径 |
|------|------|
| 类型 / PID / 指标 | `include/rf1k/` |
| 系统总控 | `system.hpp` / `src/system.cpp` |
| Modbus RTU | `modbus.hpp` / `src/modbus.cpp` |
| 匹配器 | `include/rf1k/match/` |
| 硬件端口 | `platform/hw_port.hpp` + `stub_plant.hpp` |
| FreeRTOS 任务 | `platform/freertos_app.hpp` |

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

## 文档

- `docs/overview.md` — 系统与分板
- `docs/match-algorithm.md` — 匹配算法

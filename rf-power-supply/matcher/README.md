# 射频匹配器（纯 C++ / FreeRTOS）

整个匹配器工程为 **C++17**。算法跑在 `Search` 类里；板上用 FreeRTOS 任务周期调用 `tick()`。

## 目录

```
matcher/
├── include/match/     # 头文件：types / metrics / search / freertos_task
├── src/
│   ├── search.cpp
│   ├── freertos_task.cpp
│   ├── stub_actuator.hpp
│   ├── board_actuator_example.cpp
│   └── apps/
│       ├── match_demo.cpp   # 主机冒烟
│       └── match_cli.cpp    # 命令行联调
├── docs/algorithm.md
├── Makefile
└── CMakeLists.txt
```

## 算法（一句话）

每 20 ms：读 Pf/Pr → 算 \(|\Gamma|\) → 对 Cs/Cp 做 ±Δ 试探 → 留下更好位置 → 自适应步长。

## FreeRTOS 接入

```cpp
#include "match/search.hpp"
#include "match/freertos_task.hpp"

static BoardActuator hw;                 // 实现 IActuator
static rf::match::Search search(hw, {});

void app_start() {
  search.setEnable(true);
  rf::match::MatchTask::start(search, 20, "Match", 3, 512);
}
```

定义 `RF_USE_FREERTOS` 后链接真实 FreeRTOS；主机演示不需要该宏。

## 构建 / 测试

```bash
cd rf-power-supply/matcher
make test
# 或
cmake -S . -B build-cmake -DCMAKE_CXX_COMPILER=g++
cmake --build build-cmake && ctest --test-dir build-cmake
```

```bash
./build/match_cli --cs 180 --cp 820
```

## 板上要做的事

实现 `IActuator`（见 `src/board_actuator_example.cpp`）：

- `readPower()` — ADC / 检波器 → Pf、Pr  
- `move()` — STEP/DIR  
- `hitLimit()` — 限位开关  
- `delayMs()` — `vTaskDelay` 等待电机停稳  

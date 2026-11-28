# 匹配器 C++ / FreeRTOS 实现

## 结构

```
include/match/
  types.hpp          # Axis / State / Config / IActuator
  metrics.hpp        # Γ、VSWR
  search.hpp/.cpp    # 坐标交替搜索类 Search
  freertos_task.hpp  # MatchTask::start() 周期 tick
src/
  main.cpp           # 主机演示
  stub_actuator.hpp  # 仿真负载
  board_actuator_example.cpp  # 板上接口示例
```

## FreeRTOS 上怎么挂

```cpp
#include "match/search.hpp"
#include "match/freertos_task.hpp"

static BoardActuator hw;
static rf::match::Search search(hw, rf::match::Config{});

void app_start() {
  search.setEnable(true);
  rf::match::MatchTask::start(search, /*period_ms=*/20, "Match", /*prio=*/3, 512);
}
```

任务里每 20 ms 调一次 `Search::tick()`；功率环、通信仍是别的任务。

## 主机验证

```bash
cd rf-power-supply/matcher/firmware_cpp && make test
```

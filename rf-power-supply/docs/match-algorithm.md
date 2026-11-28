# 匹配算法（C++）

实现：`rf1k::match::Search`（`include/rf1k/match/search.hpp`，`src/match/search.cpp`）。  
由 `rf1k::System::tickMatch()` 在 Ramp/Run 且 `match_auto` 时周期调用。

## 1. 观测量

\[
|\Gamma|\approx\sqrt{\frac{P_r}{P_f+P_r}},\quad
\mathrm{VSWR}=\frac{1+|\Gamma|}{1-|\Gamma|}
\]

**不要用 `Pr/Pf` 直接开方作梯度**：严重失配时 `Pr>Pf` 会饱和。代码用 `Pr/(Pf+Pr)`（见 `match::gammaFromPower` / `rf1k::gammaMag`）。

## 2. 坐标交替搜索

对每个轴（Cs、Cp）：

1. 测当前位置 cost \(c_0\)
2. 试探 \(+\Delta\)、\(-\Delta\)
3. 停在 \(\{0,+,-\}\) 中 cost 最小处

然后按 cost 调整 \(\Delta\)：失配大则放大，接近匹配则减小。

收敛判据：默认 \(|\Gamma| < 0.05\)（约 VSWR &lt; 1.1）→ `State::Track`。

## 3. 状态

`Idle → Preset → Coarse → Fine → Track`  
超时 / 限位 → `Fault`（`match::Fault`）。

## 4. FreeRTOS

推荐走总控任务，而不是单独再挂一套：

```cpp
// AppTasks 内 20 ms:
sys.tickMatch();
```

若仅测匹配器，仍可用：

```cpp
rf1k::match::MatchTask::start(search, 20, "Match", 3, 512);
```

## 5. 板上接口

实现 `rf1k::match::IActuator`：

- `readPower()` → Pf/Pr  
- `move(axis, delta)` → STEP/DIR  
- `hitLimit` / `delayMs`（建议 `vTaskDelay` 等待停稳）

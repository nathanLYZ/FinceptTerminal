# 匹配算法（C++）

## 观测量

\[
|\Gamma|\approx\sqrt{\frac{P_r}{P_f+P_r}},\quad
\mathrm{VSWR}=\frac{1+|\Gamma|}{1-|\Gamma|}
\]

代价用 `Pr/(Pf+Pr)`，高反射时仍有梯度。

## 坐标交替搜索

`Search::tick()` 对 Cs、Cp 各试探 ±Δ，保留 cost 更小的位置；按 cost 放大/缩小 Δ。

## FreeRTOS

```cpp
MatchTask::start(search, /*period_ms=*/20, "Match", /*prio=*/3, 512);
```

状态：`Idle → Preset → Coarse → Fine → Track`（超时/限位 → `Fault`）。

# 匹配算法说明

## 1. 观测量

定向耦合器给出正向功率 \(P_f\)、反射功率 \(P_r\)。

\[
\gamma^2 = \frac{P_r}{P_f+P_r},\quad
|\Gamma|\approx\sqrt{\gamma^2},\quad
\mathrm{VSWR}=\frac{1+|\Gamma|}{1-|\Gamma|}
\]

软件里把 **cost = |\Gamma|**（或 \(\gamma^2\)）作为最小化目标。

## 2. 坐标交替搜索（首选，鲁棒）

对每个可调轴（Cs、Cp，或 L、C）：

```
g0 = measure()
gp = step(+Δ); measure(); step 回退逻辑见代码
gm = step(-Δ); measure()
移到 {0, +, -} 中 cost 最小者
```

两轴轮流做一轮 = 一次匹配迭代。  
\(|\Gamma| < 0.05\)（约 VSWR&lt;1.1）可认为收敛。

优点：实现简单、对噪声和迟滞友好。  
缺点：比真正的 2D 梯度慢一点，工业匹配器够用。

## 3. 步长调度

- 失配大（cost &gt; 0.4）：Δ 放大（粗搜）
- 接近匹配（cost &lt; 0.15）：Δ 减半（精搜）
- 连续 N 次无改善：宣布收敛或报 MATCH_TIMEOUT

## 4. 状态机（建议）

```
IDLE → PRESET(记忆点) → COARSE(低功率) → FINE → TRACK → (失配)回 COARSE
                              ↘ FAULT(限位/超时)
```

## 5. C++ 形态（FreeRTOS）

```cpp
class Search {
 public:
  void tick();  // 每 20ms 由 MatchTask 调用
 private:
  float tryMove(Axis, int delta);
  // ...
};

// FreeRTOS:
MatchTask::start(search, /*period_ms=*/20, "Match", /*prio=*/3, 512);
```

完整代码：`firmware_cpp/`。

# 射频匹配器软件怎么写

匹配器软件 = **测反射 → 试探步进 → 留下更好的位置**，循环直到 VSWR / \(|\Gamma|\) 足够小。

**推荐实现：C++ + FreeRTOS** → [`firmware_cpp/`](firmware_cpp/)  
（`firmware/` 下保留同算法的 C 参考实现。）

## 最小闭环

```
每 20–100 ms:
  1. 读 Pf、Pr  → 算 cost = |Γ| 或 Pr/(Pf+Pr)
  2. 对轴 Cs（或 L）做 ±Δ 试探，选 cost 更小的位置
  3. 对轴 Cp 同样做一次
  4. 改善很小则缩小 Δ；碰限位则反向
```

## 推荐写法（C++）

| 层 | 职责 | 文件 |
|----|------|------|
| 接口 | `IActuator` 读功率 / 走电机 | `types.hpp` |
| 传感 | Pf/Pr → Γ、VSWR | `metrics.hpp` |
| 搜索 | `Search::tick()` | `search.hpp/.cpp` |
| 任务 | FreeRTOS `MatchTask::start` | `freertos_task.hpp` |
| 仿真 | 无硬件 | `stub_actuator.hpp` + `make test` |

## 关键设计点

1. **代价函数不要用 `Pr/Pf` 直接开方当梯度**  
   高反射时 `Pr>Pf` 会饱和。用 `Pr/(Pf+Pr)` 才有连续梯度。
2. **低功率先粗匹配，再爬功率**（点火模式），避免高功率失配打火。
3. **电机未停稳不要采样**（blanking 10–50 ms）。
4. **记忆点**：按 recipe 存上次 Cs/Cp，下次预置，大幅缩短收敛。
5. **与功率环解耦**：匹配慢（Hz～几十 Hz），功率快（kHz）；匹配恶化时可对功率做软折返。

## 快速跑通

```bash
# C++ / FreeRTOS 风格（推荐）
cd firmware_cpp && make test

# Python 仿真
python3 sim/match_demo.py

# C 参考实现
cd firmware && make test
```

详细算法见 [docs/algorithm.md](docs/algorithm.md)。  
板上接入见 [firmware_cpp/README.md](firmware_cpp/README.md)。

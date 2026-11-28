# 射频匹配器软件怎么写

匹配器软件 = **测反射 → 试探步进 → 留下更好的位置**，循环直到 VSWR / \(|\Gamma|\) 足够小。

## 最小闭环

```
每 20–100 ms:
  1. 读 Pf、Pr  → 算 cost = |Γ| 或 Pr/(Pf+Pr)
  2. 对轴 Cs（或 L）做 ±Δ 试探，选 cost 更小的位置
  3. 对轴 Cp 同样做一次
  4. 改善很小则缩小 Δ；碰限位则反向
```

## 推荐写法（三层）

| 层 | 职责 | 本目录 |
|----|------|--------|
| 传感 | ADC → Pf/Pr，算 \(\Gamma\)、VSWR | `firmware/match_metrics.*` |
| 搜索 | 坐标交替 / 梯度试探 | `firmware/match_search.*` |
| 执行 | 步进电机 STEP/DIR、限位 | `firmware/match_actuator.*`（接口） |
| 任务 | FreeRTOS 周期调用 | `firmware/match_task.c` |
| 仿真 | 无硬件验证收敛 | `sim/match_demo.py` |

## 关键设计点

1. **代价函数不要用 `Pr/Pf` 直接开方当梯度**  
   高反射时 `Pr>Pf` 会饱和。用 `Pr/(Pf+Pr)` 才有连续梯度。
2. **低功率先粗匹配，再爬功率**（点火模式），避免高功率失配打火。
3. **电机未停稳不要采样**（blanking 10–50 ms）。
4. **记忆点**：按 recipe 存上次 Cs/Cp，下次预置，大幅缩短收敛。
5. **与功率环解耦**：匹配慢（Hz～几十 Hz），功率快（kHz）；匹配恶化时可对功率做软折返。

## 快速跑通

```bash
# 算法仿真（无硬件）
python3 sim/match_demo.py

# C 模块主机测试
cd firmware && make test
```

详细算法见 [docs/algorithm.md](docs/algorithm.md)。

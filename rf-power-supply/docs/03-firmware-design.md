# 03 — 固件设计

## 1. 软件架构

目标 MCU：STM32H743，FreeRTOS。

```
┌─────────────────────────────────────────────────┐
│                  Application                     │
│  StateMachine │ PowerLoop │ MatchLoop │ HMI      │
├─────────────────────────────────────────────────┤
│  Services: FaultMgr │ Calib │ Logger │ Comm      │
├─────────────────────────────────────────────────┤
│  Drivers: ADC │ DAC │ DDS │ Stepper │ ETH │ 485  │
├─────────────────────────────────────────────────┤
│              HAL / CMSIS / FreeRTOS              │
└─────────────────────────────────────────────────┘
```

### 任务划分

| 任务 | 周期 / 触发 | 优先级 | 职责 |
|------|-------------|--------|------|
| `FaultTask` | 事件 + 1 ms | 最高 | 锁存保护、强制 RF OFF |
| `PowerLoopTask` | 200 µs–1 ms | 高 | 读 Pf/Pr，PID 跟踪功率 |
| `MatchLoopTask` | 20–100 ms | 中 | 最小化反射系数 |
| `CommTask` | 事件驱动 | 中 | Modbus / TCP |
| `HmiTask` | 50 ms | 低 | 显示与按键 |
| `TelemetryTask` | 100 ms | 低 | 日志与统计 |

## 2. 功率控制环

控制量：总线电压设定 \(V_{\mathrm{bus}}^*\)（主）+ DDS 幅度 \(A\)（辅）。

测量：

\[
P_f = f_{\mathrm{cal}}(V_{\mathrm{det},f}),\quad
P_r = f_{\mathrm{cal}}(V_{\mathrm{det},r}),\quad
\mathrm{VSWR}=\frac{1+\lvert\Gamma\rvert}{1-\lvert\Gamma\rvert},\quad
\lvert\Gamma\rvert=\sqrt{P_r/P_f}
\]

离散 PID（增量式）：

\[
\Delta u[k]=K_p(e[k]-e[k-1])+K_i e[k]+K_d(e[k]-2e[k-1]+e[k-2])
\]

其中 \(e = P_{\mathrm{set}} - P_f\)。

软启动：\(P_{\mathrm{set, ramp}}\) 按斜率限制，避免点火瞬间过冲。

反射保护逻辑：

1. \(P_r > P_{r,\mathrm{trip}}\) 持续 \(T_{\mathrm{debounce}}\) → 立即 RF OFF，FAULT。
2. \(\mathrm{VSWR} > \mathrm{VSWR}_{\mathrm{warn}}\) → 降功率至安全点，触发匹配加速。

## 3. 自动匹配算法

采用 **坐标交替搜索**（实现简单、对噪声鲁棒）：

```
while running and not converged:
    for axis in (Cs, Cp):
        measure Γ0
        step +Δ on axis; measure Γ+
        step -2Δ; measure Γ-
        move to best of {0, +, -}; restore Δ schedule
    shrink Δ if improvement < ε
```

增强项：

- **点火模式**：先以低功率（如 20 W）粗匹配，再爬升。
- **记忆点**：按 recipe ID 存储上次 \(C_s,C_p\)，下次预置。
- **边界**：碰到限位则反转搜索方向并报警。

仿真脚本：`sim/match_solver.py`。

## 4. 故障码

| 码 | 名称 | 说明 |
|----|------|------|
| 0x0001 | OC | 过流 |
| 0x0002 | OV | 过压 |
| 0x0004 | HIGH_REFL | 反射过高 |
| 0x0008 | OT_PA | PA 过温 |
| 0x0010 | OT_PSU | 电源过温 |
| 0x0020 | MATCH_TIMEOUT | 匹配超时 |
| 0x0040 | INTERLOCK | 联锁断开 |
| 0x0080 | ARC | 弧光检测 |
| 0x0100 | COMM_WATCHDOG | 通信看门狗 |

## 5. 校准与配置

参数存放于外部 Flash / MCU OTP 区：

- 功率检波两点校准
- PID \(K_p,K_i,K_d\)
- 匹配步进当量（pF/step）
- 通信地址、IP

出厂流程：假负载 50 Ω → 功率校准 → 开路/短路匹配边界检查 → 老化。

## 6. 代码入口

见 `firmware/`：

- `app/state_machine.c` — 状态机
- `app/power_loop.c` — 功率环
- `app/match_loop.c` — 匹配环
- `app/fault_manager.c` — 故障管理
- `inc/rf_types.h` — 公共类型

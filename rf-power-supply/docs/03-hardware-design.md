# 03 — 硬件设计要点

软件已全部 C++ 化；硬件分板目标不变。

## 分板

| 板卡 | 功能 | 与软件接口 |
|------|------|------------|
| PSU | PFC + LLC → 48–60 V | `setBusVoltage`（DAC 微调 LLC 参考） |
| OSC | DDS + 死区 + 隔离驱动 | `setDdsAmplitude`、频率固定 13.56 MHz |
| PA | GaN 半桥、滤波、定向耦合器、检波 | `readPfW` / `readPrW` |
| MATCH | L 网络 + 步进真空电容 | `match::IActuator` |
| CTRL | STM32H743 | 跑 `rf1k::System` |
| HMI | 屏 / 急停 | 急停硬断接触器；设定可写 Modbus |

## 匹配网络

等离子体低阻容性负载优先：

```
RF_in ── L ──┬── Load
             C
            GND
```

备选双真空电容 Cs/Cp。软件轴名：`match::Axis::Cs` / `Cp`（第二轴可映射为可调 L 或 Cp）。

## 保护

比较器链（过流 / 过压 / 高反射 / 弧光）锁存关断门极，MCU 读 `faultLatched()`，软件置 `rf1k::Fault` 位。

## PCB 提示

- RF 50 Ω 微带 + 完整地
- GaN 驱动环最小；数字/RF 分腔
- 真空电容高压净空与接地

（量产前完成安规、EMC、射频防护验证。）

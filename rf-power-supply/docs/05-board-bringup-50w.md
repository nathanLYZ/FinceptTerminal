# 05 — 板级 IHardware + ≤50 W 假负载联调

## 目标

在 **50 Ω 假负载** 上用真实（或 mock）`BoardHw` 完成首次点火，功率硬限制 **≤ 50 W**。

## 硬件准备

1. PA 输出接 **50 Ω / ≥100 W** 假负载（N 型），同轴良好拧紧  
2. 联锁回路闭合；急停可随时切断母线接触器  
3. 水冷/风冷按设计开启；确认 PA 温度探头可读  
4. CTRL 烧录带 `-DRF_TARGET_STM32` 的固件；CubeMX 引脚见 `board_config.hpp` / `pins::*`

## 软件对象

| 类 | 作用 |
|----|------|
| `rf1k::BoardHw` | 真驱动：ADC 检波、DAC 母线、DDS 幅度、RF_EN、步进、联锁 |
| `rf1k::applyBringup50W` / `startBringupRf` | 限功率 50 W、慢爬升、低反射跳闸 |
| `rf1k::System` | 功率环 + 匹配 + Modbus |

## 联调步骤

### 0. 编译检查（无板）

```bash
cd rf-power-supply && make board-check bringup50
```

### 1. 上电自检（RF 关闭）

- `interlockOk() == true`
- `faultLatched() == false`
- 读 `readTempPaC()`、`readVbusV()` 合理
- `setBusVoltage(0)`、`rfEnable(false)`

### 2. 检波器粗标定（假负载）

在已知功率点（例如定向耦合器后接功率计）记录 ADC → 更新 `BoardCal.det`（`pf_v0/pf_k`）。  
未标定前只信相对变化，绝对瓦数可能偏差大。

### 3. 10 W 点火 + 匹配

```cpp
BoardHw hw(cal);
hw.init();
System sys(hw, hw);
sys.init();
startBringupRf(sys, 10.f, cal.bringup);  // limit=50, ramp=20 W/s, PrefTrip=15
// FreeRTOS: AppTasks::start(sys);
```

观察：VSWR 下降、`match::Search` 进入 Track、无 Fault。

### 4. 爬升至 50 W

将 `power_set_w` 提到 50（或故意写 200，确认被 `power_limit_w` 钳住）。  
确认：`Pf ≈ 50`、`Pr` 低、壳温可接受、无电弧/异响。

### 5. 保护抽检

- 短暂断开联锁 → `Fault::Interlock`，RF 关  
- 人为失配（若安全）→ 反射跳闸  
- 急停 → 母线接触器硬断

### 6. 退出 bring-up

标定完成后：`bringup_50w=false`，`power_limit_w=1000`，提高 `prefl_trip_w` / `ramp_w_s`，再做百瓦级试验。

## 安全

- 首次上电禁止空载/开路打满功率  
- 假负载散热不足时缩短 CW 时间  
- 所有门极使能默认关闭；软件故障与硬件锁存双路径  

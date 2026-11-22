# 05 — 接口与通信协议

## 1. Modbus-RTU（RS-485）

- 波特率：115200 8N1（可配置）
- 从站地址：1–247，默认 1
- 字节序：Holding Register 大端；32 位值为高字在前

### Holding Registers（可写）

| Addr | 名称 | 单位 | 说明 |
|------|------|------|------|
| 0x0000 | CMD_WORD | bitfield | bit0=RF_ON, bit1=CLEAR_FAULT, bit2=MATCH_AUTO |
| 0x0001 | POWER_SET_W | W | 0–1000 |
| 0x0002 | RAMP_W_S | W/s | 默认 200 |
| 0x0003 | PREFL_TRIP_W | W | 反射跳闸阈值 |
| 0x0004 | CS_PRESET | step | 串联电容预置 |
| 0x0005 | CP_PRESET | step | 并联电容预置 |

### Input Registers（只读）

| Addr | 名称 | 单位 |
|------|------|------|
| 0x0100 | STATE | 0=IDLE,1=RAMP,2=RUN,3=FAULT |
| 0x0101 | PF_W | W ×1 |
| 0x0102 | PR_W | W ×1 |
| 0x0103 | VSWR_X100 | VSWR×100 |
| 0x0104 | VBUS_MV | mV |
| 0x0105 | TEMP_PA_C | °C ×10 |
| 0x0106 | FAULT_LATCH | bitfield |
| 0x0107 | CS_POS | step |
| 0x0108 | CP_POS | step |

## 2. 以太网 TCP JSON（端口 5020）

请求 / 响应均为一行 JSON（`\n` 结尾）。

```json
{"cmd":"set_power","w":500}
{"cmd":"rf","on":true}
{"cmd":"get_status"}
{"cmd":"clear_fault"}
```

`get_status` 响应示例：

```json
{
  "state": "RUN",
  "pf_w": 498.2,
  "pr_w": 12.1,
  "vswr": 1.37,
  "temp_pa_c": 62.5,
  "fault": 0
}
```

## 3. 模拟接口

| 信号 | 范围 | 映射 |
|------|------|------|
| SET 输入 | 0–10 V | 0–1000 W |
| PF 输出 | 0–10 V | 0–1000 W |
| PR 输出 | 0–10 V | 0–200 W |
| RF_ON 输入 | 24 V 光耦 | 高有效 |
| FAULT 输出 | 继电器 | 故障闭合 |

## 4. 联锁脚

`INTERLOCK+ / INTERLOCK-`：干接点常闭。断开则硬件禁止 RF，并置故障 `0x0040`。

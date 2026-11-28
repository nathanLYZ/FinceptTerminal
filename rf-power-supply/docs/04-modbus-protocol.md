# 04 — Modbus 协议

实现类：`rf1k::ModbusMap`（`include/rf1k/modbus.hpp`）。

## 链路

- RS-485，115200 8N1（可配）
- 从站地址默认 1
- RTU：PDU + CRC16（多项式 0xA001）

## Holding（可写）

| 地址 | 枚举 | 含义 |
|------|------|------|
| 0 | `HCmdWord` | bit0=RF_ON, bit1=CLEAR_FAULT, bit2=MATCH_AUTO |
| 1 | `HPowerSetW` | 功率设定 W |
| 2 | `HRampWS` | 爬升斜率 W/s |
| 3 | `HPreflTripW` | 反射跳闸 W |
| 4 | `HCsPreset` | Cs 预置步 |
| 5 | `HCpPreset` | Cp 预置步 |

## Input（只读，协议绝对地址 0x0100+）

| 地址 | 枚举 | 含义 |
|------|------|------|
| 0x0100 | `IState` | 0 Idle / 1 Ramp / 2 Run / 3 Fault |
| 0x0101 | `IPfW` | 正向功率 W |
| 0x0102 | `IPrW` | 反射功率 W |
| 0x0103 | `IVswrX100` | VSWR×100 |
| 0x0104 | `IVbusMv` | 母线 mV |
| 0x0105 | `ITempPaX10` | PA 温度 ×10 °C |
| 0x0106 | `IFault` | 故障位图 |
| 0x0107 | `ICsPos` | Cs 位置 |
| 0x0108 | `ICpPos` | Cp 位置 |

## 软件数据流

```
UART RX 完整 RTU 帧
  → ModbusMap::processRtu
  → System::tickComm 里 applyHolding / exportTelemetry
```

单测：`src/apps/test_modbus.cpp`（`make test`）。

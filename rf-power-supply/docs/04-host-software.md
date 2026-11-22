# 04 — 上位机软件

## 目标

提供工艺主机 / 工程师联调用的 Python 控制台：

- 通过 Modbus-RTU（串口）或 TCP JSON 控制 RF-1K
- 设定功率、启停、清除故障、读写匹配位置
- 实时显示 \(P_f, P_r, \mathrm{VSWR}\)、温度与状态

## 依赖

```bash
pip install -r host-software/requirements.txt
```

## 使用

```bash
# 仿真模式（无硬件）
python -m host_software.cli --sim

# 串口 Modbus
python -m host_software.cli --port /dev/ttyUSB0 --baud 115200 --slave 1

# 以太网
python -m host_software.cli --tcp 192.168.1.50:5020
```

常用命令（CLI 内）：

| 命令 | 作用 |
|------|------|
| `on 500` | 以 500 W 开启 |
| `set 300` | 改设定功率 |
| `off` | 关闭 RF |
| `status` | 打印遥测 |
| `clear` | 清除故障 |
| `match auto` | 启动自动匹配 |
| `quit` | 退出 |

## 与工艺主机集成

建议工艺主机只写：

1. `ENABLE` 联锁
2. `PowerSetpoint`
3. `RF_ON`

并周期读 `Status` / `Fault`。匹配默认由电源内部自动完成；仅在特殊 recipe 下写预置电容位置。

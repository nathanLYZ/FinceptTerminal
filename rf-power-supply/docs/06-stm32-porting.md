# 06 — STM32 / FreeRTOS 移植与下一里程碑

## 本迭代已完成

- 主机侧 **FreeRTOS shim** + 五任务调度（Fault / Power / Match / Comm / Telemetry）
- **Modbus 寄存器图**与 FC03/04/06/10 PDU 处理（`services/modbus_regs.c`）
- **STM32H743 引脚草案**（`firmware/bsp/stm32h743/pinmap.h`）
- **KiCad 分层原理图工程骨架**（`hardware/kicad/rf1k/`）

## 板级下一步（建议顺序）

1. 用 CubeMX 按 `pinmap.h` 生成工程，接入真实 FreeRTOS 任务。
2. 在 KiCad 中打开 `rf1k.kicad_pro`，按各 sheet 注释补齐器件与网络。
3. 先做 **PA + OSC 小功率验证**（假负载 ≤ 50 W），再合入 PSU / MATCH。
4. 标定 AD8307 功率曲线，写入 Flash 校准表。

## 验证命令

```bash
cd rf-power-supply/firmware && make test
```

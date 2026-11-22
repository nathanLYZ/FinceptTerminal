# STM32H743 移植指南

## 推荐流程

1. 打开已预配置的 CubeMX 工程：[`../../target/stm32/RF1K.ioc`](../../target/stm32/RF1K.ioc)。
2. Generate Code 后，按 [`../../target/stm32/README.md`](../../target/stm32/README.md) 把 `Drivers/RF1K` 与应用层链入，并定义 `RF_TARGET_STM32`。
3. `RF1K_AppStart()` 创建五任务；`hw_stm32.c` 已实现 `rf_api.h` 硬件钩子。
4. 主机无工具链时先跑：`make stm32-check`。

## 任务映射

| Shim 任务 | FreeRTOS 周期 | 优先级建议 |
|-----------|---------------|------------|
| TaskFault | 1 ms / EXTI | AboveNormal |
| TaskPowerLoop | 1 ms | High |
| TaskMatchLoop | 20 ms | Normal |
| TaskComm | 10 ms + UART RX | Normal |
| TaskTelemetry | 50–100 ms | BelowNormal |

## 主机验证

无需板级即可验证算法与 Modbus：

```bash
cd firmware && make run
make test
```

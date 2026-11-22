# STM32H743 移植指南

## 推荐流程

1. CubeMX 新建 **STM32H743VITx**，启用：
   - ADC1/2（多通道 DMA 循环）
   - DAC1
   - SPI1（DDS）或 SPI4（若以太网占默认脚）
   - USART2 + RS485 DE
   - ETH RMII + LwIP（可选）
   - FreeRTOS CMSIS-V2
   - TIM6 作为 1 ms 节拍旁路检查
2. 按 `pinmap.h` 分配引脚；若与 ETH 冲突，将 DDS 迁到 SPI4。
3. 用本目录外的 `app/` + `services/` 源文件替换 Cube 生成的 `Core/Src/main.c` 用户段：
   - 在 `MX_FREERTOS_Init` 中调用 `rf_app_init` + 创建与 `rf_tasks.c` 对应的真实任务。
4. 实现 `drivers/hw_*.c` 真 HAL（当前 `drivers/hw_stub.c` 仅主机仿真）。

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

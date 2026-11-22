# STM32H743 目标工程

## 目录

```
target/stm32/
├── RF1K.ioc                 # CubeMX 工程（引脚已按 pinmap 预填）
├── Core/Src/
│   ├── main_stm32.c         # main 参考 / mock 入口
│   └── freertos_app.c       # xTaskCreate 五任务
├── Drivers/RF1K/
│   ├── hw_stm32.c           # rf_api.h 的真实 HAL 实现
│   ├── dds_ad9951.c
│   ├── stepper_tmc.c
│   ├── modbus_uart.c        # USART2 RS-485 + IDLE 成帧
│   └── board_gpio_adc.c     # ADC DMA / DAC / 联锁
└── mock_hal/                # 主机侧语法编译用
```

## 接入 CubeMX（推荐流程）

1. 用 **STM32CubeMX 6.12+** 打开 `RF1K.ioc`，核对引脚与 IP，Generate Code（STM32CubeIDE 或 Makefile）。
2. 将仓库应用层加入工程源文件：
   - `firmware/app/**/*.c`
   - `firmware/services/*.c`
   - `firmware/target/stm32/Drivers/RF1K/*.c`
   - `firmware/target/stm32/Core/Src/freertos_app.c`
3. Include paths：`firmware/inc`、`firmware/app/tasks`、`Drivers/RF1K`。
4. 预定义宏：`RF_TARGET_STM32`。
5. **不要**链接 `drivers/hw_stub.c` / `platform/freertos_shim.c`。
6. 在 Cube 生成的 `main.c` USER CODE 中，`osKernelStart()` 前调用 `RF1K_AppStart()`。
7. 在 `USART2_IRQHandler` 的 IDLE 分支调用 `RF1K_UART_IdleHandler()`。

## 主机编译检查（无需工具链）

```bash
cd firmware && make stm32-check
```

用 `mock_hal` 编译 STM32 驱动 + Modbus RTU 冒烟，验证接口可链接。

## 板级调试顺序

1. 假负载 50 Ω，RF ≤ 50 W  
2. 读 ADC 原始值 → 标定 `g_det_cal`  
3. Modbus 读写 holding/input  
4. 匹配电机点动 → 自动匹配  
5. 爬升至额定功率  

/**
 * STM32 entry outline. After CubeMX generation:
 *  1. Keep MX_*_Init() from generated main.c
 *  2. Call RF1K_AppStart() before osKernelStart() / vTaskStartScheduler()
 *
 * This file is the reference main used by `make stm32-check` (mock HAL).
 */
#include "board.h"
#include "rf_tasks.h"

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
#include "main.h"
#include "cmsis_os.h"

extern void RF1K_AppStart(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC1_Init();
    MX_DAC1_Init();
    MX_SPI4_Init();
    MX_USART2_UART_Init();
    MX_FREERTOS_Init(); /* Cube: creates idle/timer; we add RF tasks in RF1K_AppStart */
    RF1K_AppStart();
    osKernelStart();
    while (1) {
    }
}

#else
/* Mock / documentation build — not a full firmware image */
#include <stdio.h>
#include "board_config.h"
#include "modbus_rtu.h"
#include "rf_api.h"

extern float hw_read_pf_w(void);
extern float hw_read_pr_w(void);

int main(void)
{
    board_init();
    printf("RF1K STM32 mock-HAL compile check\n");
    printf("Pf=%.3f Pr=%.3f interlock=%d\n", hw_read_pf_w(), hw_read_pr_w(),
           board_interlock_ok());

    /* Smoke Modbus RTU framing */
    modbus_map_t map;
    modbus_map_init(&map);
    map.holding[1] = 123;
    uint8_t req[8] = {1, 0x03, 0x00, 0x01, 0x00, 0x01, 0, 0};
    uint16_t crc = modbus_crc16(req, 6);
    req[6] = (uint8_t)(crc & 0xff);
    req[7] = (uint8_t)(crc >> 8);
    board_modbus_on_frame(req, 8);
    uint8_t frame[64];
    int n = board_modbus_pop_request(frame, (int)sizeof(frame));
    uint8_t resp[64];
    int rn = modbus_rtu_process_frame(&map, frame, n, resp, (int)sizeof(resp));
    printf("modbus rtu resp_len=%d\n", rn);
    return rn > 0 ? 0 : 1;
}
#endif

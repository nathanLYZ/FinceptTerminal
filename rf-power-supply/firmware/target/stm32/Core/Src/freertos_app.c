/**
 * FreeRTOS task creation for STM32 (drop into CubeMX freertos.c USER sections
 * or compile as this translation unit instead of the shim starter).
 */
#include "rf_tasks.h"
#include "board.h"
#include "board_config.h"

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
#include "FreeRTOS.h"
#include "task.h"

static rf_app_ctx_t s_ctx;

static void FaultTaskEntry(void *arg)
{
    for (;;) {
        TaskFault(arg);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
static void PowerTaskEntry(void *arg)
{
    for (;;) {
        TaskPowerLoop(arg);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
static void MatchTaskEntry(void *arg)
{
    for (;;) {
        TaskMatchLoop(arg);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
static void CommTaskEntry(void *arg)
{
    for (;;) {
        TaskComm(arg);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
static void TelemTaskEntry(void *arg)
{
    for (;;) {
        TaskTelemetry(arg);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void RF1K_AppStart(void)
{
    board_init();
    rf_app_init(&s_ctx);

    xTaskCreate(FaultTaskEntry, "Fault", 256, &s_ctx, 5, NULL);
    xTaskCreate(PowerTaskEntry, "Power", 512, &s_ctx, 4, NULL);
    xTaskCreate(MatchTaskEntry, "Match", 512, &s_ctx, 3, NULL);
    xTaskCreate(CommTaskEntry, "Comm", 512, &s_ctx, 3, NULL);
    xTaskCreate(TelemTaskEntry, "Telem", 256, &s_ctx, 1, NULL);
}

#else
/* Host / mock: see app/main.c + freertos_shim */
void RF1K_AppStart(void) {}
#endif

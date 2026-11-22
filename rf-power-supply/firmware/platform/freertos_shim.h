/**
 * Minimal cooperative FreeRTOS-like shim for host bring-up.
 * On STM32, replace includes with real FreeRTOS headers and delete this shim.
 */
#ifndef FREERTOS_SHIM_H
#define FREERTOS_SHIM_H

#include <stddef.h>
#include <stdint.h>

typedef void (*TaskFunction_t)(void *);

typedef struct {
    const char *name;
    TaskFunction_t fn;
    void *param;
    uint32_t period_ms;
    uint32_t last_run_ms;
    int priority; /* higher runs first within the same tick */
} ShimTask_t;

#define configTICK_RATE_HZ 1000u

void vTaskDelay(uint32_t ticks);
uint32_t xTaskGetTickCount(void);

int shim_task_register(const char *name, TaskFunction_t fn, void *param,
                       uint32_t period_ms, int priority);
void shim_scheduler_run(uint32_t duration_ms);

/* Compatibility macros used by app tasks */
#define pdMS_TO_TICKS(ms) (ms)
#define portMAX_DELAY 0xffffffffu

#endif /* FREERTOS_SHIM_H */

#ifndef TASK_H_MOCK
#define TASK_H_MOCK
#include <stdint.h>
typedef void (*TaskFunction_t)(void *);
typedef void *TaskHandle_t;
#define pdPASS 1
static inline int xTaskCreate(TaskFunction_t fn, const char *n, unsigned stack,
                              void *arg, unsigned prio, TaskHandle_t *h)
{
    (void)fn;
    (void)n;
    (void)stack;
    (void)arg;
    (void)prio;
    (void)h;
    return pdPASS;
}
static inline void vTaskDelay(uint32_t t) { (void)t; }
static inline uint32_t xTaskGetTickCount(void) { return 0; }
#endif

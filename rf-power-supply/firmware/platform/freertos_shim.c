#include "freertos_shim.h"

#include <stdio.h>
#include <string.h>

#define SHIM_MAX_TASKS 16

static ShimTask_t s_tasks[SHIM_MAX_TASKS];
static int s_ntasks;
static uint32_t s_tick_ms;
static uint32_t s_delay_until_ms;

void vTaskDelay(uint32_t ticks)
{
    s_delay_until_ms = s_tick_ms + ticks;
}

uint32_t xTaskGetTickCount(void)
{
    return s_tick_ms;
}

int shim_task_register(const char *name, TaskFunction_t fn, void *param,
                       uint32_t period_ms, int priority)
{
    if (s_ntasks >= SHIM_MAX_TASKS) {
        return -1;
    }
    s_tasks[s_ntasks].name = name;
    s_tasks[s_ntasks].fn = fn;
    s_tasks[s_ntasks].param = param;
    s_tasks[s_ntasks].period_ms = period_ms ? period_ms : 1u;
    s_tasks[s_ntasks].last_run_ms = 0u;
    s_tasks[s_ntasks].priority = priority;
    s_ntasks++;
    return 0;
}

static int cmp_prio(const void *a, const void *b)
{
    const ShimTask_t *ta = a;
    const ShimTask_t *tb = b;
    return tb->priority - ta->priority;
}

void shim_scheduler_run(uint32_t duration_ms)
{
    /* Simple bubble by priority once at start */
    for (int i = 0; i < s_ntasks; ++i) {
        for (int j = i + 1; j < s_ntasks; ++j) {
            if (s_tasks[j].priority > s_tasks[i].priority) {
                ShimTask_t tmp = s_tasks[i];
                s_tasks[i] = s_tasks[j];
                s_tasks[j] = tmp;
            }
        }
    }
    (void)cmp_prio;

    const uint32_t end = s_tick_ms + duration_ms;
    while (s_tick_ms < end) {
        for (int i = 0; i < s_ntasks; ++i) {
            ShimTask_t *t = &s_tasks[i];
            if ((s_tick_ms - t->last_run_ms) >= t->period_ms) {
                t->last_run_ms = s_tick_ms;
                t->fn(t->param);
            }
        }
        s_tick_ms += 1u; /* 1 ms tick */
    }
}

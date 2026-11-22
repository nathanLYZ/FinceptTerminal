#ifndef RF_TASKS_H
#define RF_TASKS_H

#include "modbus_regs.h"
#include "rf_types.h"

typedef struct {
    rf_system_t sys;
    modbus_map_t mb;
    int log_every_ms;
} rf_app_ctx_t;

void rf_app_init(rf_app_ctx_t *ctx);
void rf_tasks_start(rf_app_ctx_t *ctx);

void TaskFault(void *param);
void TaskPowerLoop(void *param);
void TaskMatchLoop(void *param);
void TaskComm(void *param);
void TaskTelemetry(void *param);

#endif /* RF_TASKS_H */

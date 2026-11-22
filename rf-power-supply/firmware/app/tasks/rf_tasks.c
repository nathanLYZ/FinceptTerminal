#include "rf_tasks.h"

#include <stdio.h>

#include "freertos_shim.h"
#include "rf_api.h"

void rf_app_init(rf_app_ctx_t *ctx)
{
    rf_system_init(&ctx->sys);
    modbus_map_init(&ctx->mb);
    ctx->log_every_ms = 800;
}

void TaskFault(void *param)
{
    rf_app_ctx_t *ctx = param;
    if (!hw_interlock_ok() && ctx->sys.state != RF_STATE_FAULT) {
        rf_fault_on_event(&ctx->sys, FAULT_INTERLOCK);
    }
}

void TaskPowerLoop(void *param)
{
    rf_app_ctx_t *ctx = param;
    const float dt = 0.001f;
    rf_state_machine_tick(&ctx->sys, dt);
    rf_power_loop_tick(&ctx->sys, dt);
}

void TaskMatchLoop(void *param)
{
    rf_app_ctx_t *ctx = param;
    rf_match_loop_tick(&ctx->sys);
}

void TaskComm(void *param)
{
    rf_app_ctx_t *ctx = param;
    /* Host/STM32 UART ISR would push frames here.
       For now: push holding → setpoints, publish telemetry → input regs. */
    modbus_apply_holding(&ctx->mb, &ctx->sys);
    modbus_export_telemetry(&ctx->mb, &ctx->sys);
}

void TaskTelemetry(void *param)
{
    rf_app_ctx_t *ctx = param;
    static int last_log;
    int now = (int)xTaskGetTickCount();
    if (now - last_log < ctx->log_every_ms) {
        return;
    }
    last_log = now;
    printf("t=%4.1fs state=%d Pf=%6.1f Pr=%5.1f VSWR=%4.2f Vbus=%5.1f Cs=%ld Cp=%ld\n",
           now / 1000.0f, (int)ctx->sys.state, ctx->sys.tm.pf_w, ctx->sys.tm.pr_w,
           ctx->sys.tm.vswr, ctx->sys.tm.vbus_v, (long)hw_stepper_pos(0),
           (long)hw_stepper_pos(1));
}

void rf_tasks_start(rf_app_ctx_t *ctx)
{
    /* Priorities mirror docs/03-firmware-design.md */
    shim_task_register("Fault", TaskFault, ctx, 1, 40);
    shim_task_register("Power", TaskPowerLoop, ctx, 1, 30);
    shim_task_register("Match", TaskMatchLoop, ctx, 20, 20);
    shim_task_register("Comm", TaskComm, ctx, 10, 15);
    shim_task_register("Telem", TaskTelemetry, ctx, 50, 5);
}

#include "rf_tasks.h"

#include <stdio.h>

#include "board_config.h"
#include "modbus_rtu.h"
#include "os_port.h"
#include "rf_api.h"

#if defined(RF_TARGET_STM32)
#include "board.h"
#endif

void rf_app_init(rf_app_ctx_t *ctx)
{
    board_cal_load_defaults();
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
#if defined(RF_TARGET_STM32)
    if (board_fault_latched() && ctx->sys.state != RF_STATE_FAULT) {
        rf_fault_on_event(&ctx->sys, FAULT_OC | FAULT_OV | FAULT_HIGH_REFL);
    }
    if (ctx->sys.sp.clear_fault) {
        board_fault_clear();
    }
#else
    (void)ctx;
#endif
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

#if defined(RF_TARGET_STM32)
    uint8_t req[RF1K_MB_FRAME_MAX];
    uint8_t resp[RF1K_MB_FRAME_MAX];
    int n = board_modbus_pop_request(req, (int)sizeof(req));
    if (n > 0) {
        int rn = modbus_rtu_process_frame(&ctx->mb, req, n, resp, (int)sizeof(resp));
        if (rn > 0) {
            board_modbus_send(resp, (uint16_t)rn);
        }
    }
#endif

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
#if !defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
    printf("t=%4.1fs state=%d Pf=%6.1f Pr=%5.1f VSWR=%4.2f Vbus=%5.1f Cs=%ld Cp=%ld\n",
           now / 1000.0f, (int)ctx->sys.state, ctx->sys.tm.pf_w, ctx->sys.tm.pr_w,
           ctx->sys.tm.vswr, ctx->sys.tm.vbus_v, (long)hw_stepper_pos(0),
           (long)hw_stepper_pos(1));
#else
    (void)ctx;
#endif
}

void rf_tasks_start(rf_app_ctx_t *ctx)
{
#if !defined(RF_TARGET_STM32)
    shim_task_register("Fault", TaskFault, ctx, 1, 40);
    shim_task_register("Power", TaskPowerLoop, ctx, 1, 30);
    shim_task_register("Match", TaskMatchLoop, ctx, 20, 20);
    shim_task_register("Comm", TaskComm, ctx, 10, 15);
    shim_task_register("Telem", TaskTelemetry, ctx, 50, 5);
#else
    (void)ctx; /* STM32: RF1K_AppStart() in freertos_app.c creates tasks */
#endif
}

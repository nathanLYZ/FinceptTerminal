/**
 * Host simulation entry using FreeRTOS shim + Modbus register map.
 * On STM32: keep rf_app_init/rf_tasks_start logic; replace shim with real RTOS.
 */
#include <stdio.h>

#include "freertos_shim.h"
#include "modbus_regs.h"
#include "rf_tasks.h"

static void seed_setpoints_via_modbus(rf_app_ctx_t *ctx)
{
    ctx->mb.holding[MB_H_POWER_SET_W] = 500;
    ctx->mb.holding[MB_H_RAMP_W_S] = 200;
    ctx->mb.holding[MB_H_PREFL_TRIP_W] = 400;
    ctx->mb.holding[MB_H_CMD_WORD] =
        (uint16_t)(MB_CMD_RF_ON | MB_CMD_MATCH_AUTO);
    modbus_apply_holding(&ctx->mb, &ctx->sys);
}

int main(void)
{
    rf_app_ctx_t ctx;
    rf_app_init(&ctx);
    seed_setpoints_via_modbus(&ctx);
    rf_tasks_start(&ctx);

    printf("RF-1K host sim (FreeRTOS shim + Modbus map)\n");
    shim_scheduler_run(8000); /* 8 s */

    printf("final: state=%d Pf=%.1f fault=0x%04X mb_pf=%u\n",
           (int)ctx.sys.state, ctx.sys.tm.pf_w, (unsigned)ctx.sys.fault_latch,
           ctx.mb.input[MB_I_PF_W]);
    return ctx.sys.state == RF_STATE_RUN && ctx.sys.tm.pf_w > 450.0f ? 0 : 1;
}

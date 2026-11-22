#include "board_config.h"

rf_det_cal_t g_det_cal;
rf_sense_cal_t g_sense_cal;

void board_cal_load_defaults(void)
{
    /* Rough AD8307 @ 13.56 MHz after ~30 dB atten from coupler */
    g_det_cal.pf_v0 = 0.55f;
    g_det_cal.pf_k = 0.025f; /* V per dB */
    g_det_cal.pr_v0 = 0.55f;
    g_det_cal.pr_k = 0.025f;

    g_sense_cal.vbus_gain = 66.0f; /* 3.3Vadc * divider ≈ 66 V FS */
    g_sense_cal.temp_pa_offset_c = -50.0f;
    g_sense_cal.temp_pa_gain = 100.0f;
}

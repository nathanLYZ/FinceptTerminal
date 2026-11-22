/**
 * STM32 implementation of rf_api.h hardware hooks.
 * Build with -DRF_TARGET_STM32 and link CubeMX HAL.
 */
#include "rf_api.h"
#include "board_config.h"
#include "board.h"

#include <math.h>

static float det_to_watts(float v, float v0, float k)
{
    /* P(W) = 10^((V-V0)/k) / 1000 */
    float dbm_like = (v - v0) / k;
    float mw = powf(10.0f, dbm_like / 10.0f);
    return mw / 1000.0f;
}

static float adc_to_volt(uint16_t raw)
{
    return (3.3f * (float)raw) / 65535.0f;
}

float hw_read_pf_w(void)
{
    float v = adc_to_volt(g_adc_raw.raw_pf);
    return det_to_watts(v, g_det_cal.pf_v0, g_det_cal.pf_k);
}

float hw_read_pr_w(void)
{
    float v = adc_to_volt(g_adc_raw.raw_pr);
    return det_to_watts(v, g_det_cal.pr_v0, g_det_cal.pr_k);
}

float hw_read_vbus_v(void)
{
    float frac = (float)g_adc_raw.raw_vbus / 65535.0f;
    return frac * g_sense_cal.vbus_gain;
}

float hw_read_temp_pa_c(void)
{
    float frac = (float)g_adc_raw.raw_temp_pa / 65535.0f;
    return g_sense_cal.temp_pa_offset_c + frac * g_sense_cal.temp_pa_gain;
}

bool hw_interlock_ok(void)
{
    return board_interlock_ok();
}

void hw_set_bus_voltage(float volts)
{
    if (volts < 0.0f) {
        volts = 0.0f;
    }
    if (volts > RF1K_VBUS_MAX_V) {
        volts = RF1K_VBUS_MAX_V;
    }
    board_set_vbus_dac(volts);
}

void hw_set_dds_amplitude(float norm_0_1)
{
    board_dds_set_amplitude(norm_0_1);
}

void hw_rf_enable(bool on)
{
    board_rf_enable(on);
}

void hw_stepper_move(int axis, int delta_steps)
{
    board_stepper_move(axis, delta_steps);
}

int32_t hw_stepper_pos(int axis)
{
    return board_stepper_pos(axis);
}

/**
 * Host-side / unit-test stubs. Replace with real HAL on STM32.
 */
#include "rf_api.h"

static float s_pf = 0.0f;
static float s_pr = 5.0f;
static float s_vbus = 0.0f;
static float s_amp = 0.0f;
static bool s_rf_on = false;
/* Start deliberately mistuned so the matcher has work to do */
static int32_t s_cs = 200;
static int32_t s_cp = 800;

float hw_read_pf_w(void)
{
    if (!s_rf_on || s_amp <= 0.0f) {
        s_pf = 0.0f;
        s_pr = 0.0f;
        return 0.0f;
    }
    /* Plant: 60 V * full DDS ≈ 1000 W delivered into a matched load */
    float match_err = (float)((s_cs - 512) * (s_cs - 512) + (s_cp - 512) * (s_cp - 512));
    float gamma = match_err / (match_err + 50000.0f);
    float p_avail = (s_vbus / 60.0f) * 1000.0f * s_amp;
    if (p_avail < 0.0f) {
        p_avail = 0.0f;
    }
    s_pf = p_avail * (1.0f - gamma);
    s_pr = p_avail * gamma;
    return s_pf;
}

float hw_read_pr_w(void)
{
    (void)hw_read_pf_w();
    return s_pr;
}

float hw_read_vbus_v(void) { return s_vbus; }
float hw_read_temp_pa_c(void) { return 35.0f + s_pf * 0.03f; }
bool hw_interlock_ok(void) { return true; }

void hw_set_bus_voltage(float volts) { s_vbus = volts; }
void hw_set_dds_amplitude(float norm_0_1) { s_amp = norm_0_1; }
void hw_rf_enable(bool on) { s_rf_on = on; }

void hw_stepper_move(int axis, int delta_steps)
{
    if (axis == 0) {
        s_cs += delta_steps;
    } else {
        s_cp += delta_steps;
    }
}

int32_t hw_stepper_pos(int axis) { return axis == 0 ? s_cs : s_cp; }

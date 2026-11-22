#include "rf_api.h"

void rf_power_loop_tick(rf_system_t *sys, float dt_s)
{
    sys->tm.pf_w = hw_read_pf_w();
    sys->tm.pr_w = hw_read_pr_w();
    sys->tm.vbus_v = hw_read_vbus_v();
    sys->tm.temp_pa_c = hw_read_temp_pa_c();

    float gamma = rf_gamma_mag(sys->tm.pf_w, sys->tm.pr_w);
    sys->tm.vswr = rf_vswr_from_gamma(gamma);

    if (!hw_interlock_ok()) {
        rf_fault_on_event(sys, FAULT_INTERLOCK);
        return;
    }

    if (sys->tm.pr_w > sys->sp.prefl_trip_w) {
        rf_fault_on_event(sys, FAULT_HIGH_REFL);
        return;
    }

    if (sys->tm.temp_pa_c > 95.0f) {
        rf_fault_on_event(sys, FAULT_OT_PA);
        return;
    }

    if (sys->state != RF_STATE_RAMP && sys->state != RF_STATE_RUN) {
        hw_set_bus_voltage(0.0f);
        hw_set_dds_amplitude(0.0f);
        return;
    }

    float target = sys->ramp_power_w;
    float error = target - sys->tm.pf_w;
    float vbus_cmd = rf_pid_step(&sys->power_pid, error, dt_s);

    /* Keep DDS near full scale; bus voltage is primary actuator */
    hw_set_bus_voltage(vbus_cmd);
    hw_set_dds_amplitude(0.95f);
    hw_rf_enable(true);

    /* Fold back only after matcher has had time / when reflection is extreme */
    if (sys->tm.pr_w > 0.35f * sys->tm.pf_w && target > 100.0f && sys->tm.pf_w > 20.0f) {
        sys->ramp_power_w *= 0.995f;
    }
}

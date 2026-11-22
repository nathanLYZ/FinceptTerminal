#include "rf_api.h"

void rf_system_init(rf_system_t *sys)
{
    sys->state = RF_STATE_IDLE;
    sys->fault_latch = 0u;
    sys->ramp_power_w = 0.0f;

    sys->sp.power_set_w = 0.0f;
    sys->sp.ramp_w_s = 200.0f;
    sys->sp.prefl_trip_w = 200.0f;
    sys->sp.vswr_warn = 2.0f;
    sys->sp.rf_on_cmd = false;
    sys->sp.match_auto = true;
    sys->sp.clear_fault = false;

    sys->power_pid.kp = 0.05f;
    sys->power_pid.ki = 0.25f;
    sys->power_pid.kd = 0.0f;
    sys->power_pid.out_min = 0.0f;
    sys->power_pid.out_max = 60.0f; /* bus volts */
    sys->power_pid.integr = 0.0f;
    sys->power_pid.prev_e = 0.0f;
    sys->power_pid.prev_prev_e = 0.0f;
}

void rf_state_machine_tick(rf_system_t *sys, float dt_s)
{
    if (sys->sp.clear_fault) {
        rf_fault_clear(sys);
    }

    switch (sys->state) {
    case RF_STATE_IDLE:
        hw_rf_enable(false);
        sys->ramp_power_w = 0.0f;
        if (sys->sp.rf_on_cmd && sys->sp.power_set_w > 0.0f && hw_interlock_ok()) {
            sys->state = RF_STATE_RAMP;
            sys->power_pid.integr = 0.0f;
        }
        break;

    case RF_STATE_RAMP:
        if (!sys->sp.rf_on_cmd) {
            sys->state = RF_STATE_IDLE;
            break;
        }
        sys->ramp_power_w += sys->sp.ramp_w_s * dt_s;
        if (sys->ramp_power_w >= sys->sp.power_set_w) {
            sys->ramp_power_w = sys->sp.power_set_w;
            sys->state = RF_STATE_RUN;
        }
        break;

    case RF_STATE_RUN:
        if (!sys->sp.rf_on_cmd) {
            sys->state = RF_STATE_IDLE;
            break;
        }
        /* Allow setpoint changes with limited slew */
        if (sys->ramp_power_w < sys->sp.power_set_w) {
            sys->ramp_power_w += sys->sp.ramp_w_s * dt_s;
            if (sys->ramp_power_w > sys->sp.power_set_w) {
                sys->ramp_power_w = sys->sp.power_set_w;
            }
        } else if (sys->ramp_power_w > sys->sp.power_set_w) {
            sys->ramp_power_w -= sys->sp.ramp_w_s * dt_s;
            if (sys->ramp_power_w < sys->sp.power_set_w) {
                sys->ramp_power_w = sys->sp.power_set_w;
            }
        }
        break;

    case RF_STATE_FAULT:
        hw_rf_enable(false);
        break;
    }
}

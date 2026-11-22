#include "rf_api.h"

void rf_fault_on_event(rf_system_t *sys, uint32_t fault_bits)
{
    if (fault_bits == 0u) {
        return;
    }
    sys->fault_latch |= fault_bits;
    sys->state = RF_STATE_FAULT;
    sys->sp.rf_on_cmd = false;
    sys->ramp_power_w = 0.0f;
    hw_rf_enable(false);
    hw_set_dds_amplitude(0.0f);
}

void rf_fault_clear(rf_system_t *sys)
{
    if (!hw_interlock_ok()) {
        return;
    }
    sys->fault_latch = 0u;
    sys->sp.clear_fault = false;
    if (sys->state == RF_STATE_FAULT) {
        sys->state = RF_STATE_IDLE;
    }
}

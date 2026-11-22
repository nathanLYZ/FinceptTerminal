#ifndef RF_API_H
#define RF_API_H

#include "rf_types.h"

void rf_system_init(rf_system_t *sys);
void rf_state_machine_tick(rf_system_t *sys, float dt_s);
void rf_power_loop_tick(rf_system_t *sys, float dt_s);
void rf_match_loop_tick(rf_system_t *sys);
void rf_fault_on_event(rf_system_t *sys, uint32_t fault_bits);
void rf_fault_clear(rf_system_t *sys);

float rf_pid_step(pid_t *pid, float error, float dt_s);
float rf_gamma_mag(float pf_w, float pr_w);
float rf_vswr_from_gamma(float gamma);

/* Board HAL hooks — implement per target */
float hw_read_pf_w(void);
float hw_read_pr_w(void);
float hw_read_vbus_v(void);
float hw_read_temp_pa_c(void);
bool hw_interlock_ok(void);
void hw_set_bus_voltage(float volts);
void hw_set_dds_amplitude(float norm_0_1);
void hw_rf_enable(bool on);
void hw_stepper_move(int axis, int delta_steps);
int32_t hw_stepper_pos(int axis);

#endif /* RF_API_H */

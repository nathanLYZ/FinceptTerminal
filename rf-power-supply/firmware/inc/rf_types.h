#ifndef RF_TYPES_H
#define RF_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    RF_STATE_IDLE = 0,
    RF_STATE_RAMP = 1,
    RF_STATE_RUN = 2,
    RF_STATE_FAULT = 3
} rf_state_t;

enum {
    FAULT_OC = 1u << 0,
    FAULT_OV = 1u << 1,
    FAULT_HIGH_REFL = 1u << 2,
    FAULT_OT_PA = 1u << 3,
    FAULT_OT_PSU = 1u << 4,
    FAULT_MATCH_TIMEOUT = 1u << 5,
    FAULT_INTERLOCK = 1u << 6,
    FAULT_ARC = 1u << 7,
    FAULT_COMM_WATCHDOG = 1u << 8
};

typedef struct {
    float power_set_w;
    float ramp_w_s;
    float prefl_trip_w;
    float vswr_warn;
    int32_t cs_preset;
    int32_t cp_preset;
    bool rf_on_cmd;
    bool match_auto;
    bool clear_fault;
} rf_setpoints_t;

typedef struct {
    float pf_w;
    float pr_w;
    float vswr;
    float vbus_v;
    float idc_a;
    float temp_pa_c;
    float temp_psu_c;
    int32_t cs_pos;
    int32_t cp_pos;
} rf_telemetry_t;

typedef struct {
    float kp;
    float ki;
    float kd;
    float out_min;
    float out_max;
    float integr;
    float prev_e;
    float prev_prev_e;
} pid_t;

typedef struct {
    rf_state_t state;
    uint32_t fault_latch;
    rf_setpoints_t sp;
    rf_telemetry_t tm;
    pid_t power_pid;
    float ramp_power_w;
} rf_system_t;

#endif /* RF_TYPES_H */

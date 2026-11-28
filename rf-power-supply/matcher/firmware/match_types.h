#ifndef MATCH_TYPES_H
#define MATCH_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MATCH_AXIS_CS = 0, /* series C or series L actuator */
    MATCH_AXIS_CP = 1, /* shunt C */
    MATCH_AXIS_COUNT = 2
} match_axis_t;

typedef enum {
    MATCH_ST_IDLE = 0,
    MATCH_ST_PRESET,
    MATCH_ST_COARSE,
    MATCH_ST_FINE,
    MATCH_ST_TRACK,
    MATCH_ST_FAULT
} match_state_t;

enum {
    MATCH_FAULT_TIMEOUT = 1u << 0,
    MATCH_FAULT_LIMIT = 1u << 1,
    MATCH_FAULT_SENSOR = 1u << 2
};

typedef struct {
    float pf_w;
    float pr_w;
} match_power_t;

typedef struct {
    int32_t pos[MATCH_AXIS_COUNT];
    int32_t min_pos[MATCH_AXIS_COUNT];
    int32_t max_pos[MATCH_AXIS_COUNT];
    int32_t preset[MATCH_AXIS_COUNT];
} match_mech_t;

typedef struct {
    bool enable;
    float target_gamma;   /* e.g. 0.05 ≈ VSWR 1.1 */
    float coarse_gamma;   /* leave COARSE when below this */
    int step0;            /* initial step size in motor steps */
    int step_min;
    int step_max;
    uint32_t timeout_iters;
    uint32_t settle_ms;   /* wait after move before measure */
} match_cfg_t;

typedef struct {
    bool enable;
    match_state_t state;
    match_cfg_t cfg;
    match_mech_t mech;
    int step;
    uint32_t iters;
    uint32_t no_improve;
    uint32_t fault;
    float last_gamma;
    float last_vswr;
} match_t;

/* Board hooks — implement per platform */
match_power_t hw_match_read_power(void);
void hw_match_move(match_axis_t axis, int delta_steps);
void hw_match_delay_ms(uint32_t ms);
bool hw_match_hit_limit(match_axis_t axis, int direction);

#endif /* MATCH_TYPES_H */

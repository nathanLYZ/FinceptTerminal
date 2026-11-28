/**
 * Host plant model: reflection falls as actuators approach center (512,512).
 */
#include "match_types.h"

static float s_p_avail = 100.0f; /* watts available from PA */
static int32_t s_pos[2] = {200, 800};

match_power_t hw_match_read_power(void)
{
    float err = (float)((s_pos[0] - 512) * (s_pos[0] - 512) +
                        (s_pos[1] - 512) * (s_pos[1] - 512));
    float gamma = err / (err + 50000.0f);
    match_power_t p;
    p.pf_w = s_p_avail * (1.0f - gamma);
    p.pr_w = s_p_avail * gamma;
    return p;
}

void hw_match_move(match_axis_t axis, int delta_steps)
{
    if (axis == MATCH_AXIS_CS || axis == MATCH_AXIS_CP) {
        s_pos[axis] += delta_steps;
    }
}

void hw_match_delay_ms(uint32_t ms) { (void)ms; }

bool hw_match_hit_limit(match_axis_t axis, int direction)
{
    (void)direction;
    if (s_pos[axis] < 0 || s_pos[axis] > 1023) {
        return true;
    }
    return false;
}

/* Test helpers */
void hw_match_stub_set_pos(int cs, int cp)
{
    s_pos[0] = cs;
    s_pos[1] = cp;
}

void hw_match_stub_get_pos(int *cs, int *cp)
{
    *cs = s_pos[0];
    *cp = s_pos[1];
}

#include "rf_api.h"

#define AXIS_CS 0
#define AXIS_CP 1

static float measure_gamma(void)
{
    float pf = hw_read_pf_w();
    float pr = hw_read_pr_w();
    return rf_gamma_mag(pf, pr);
}

static float try_step(int axis, int delta)
{
    hw_stepper_move(axis, delta);
    /* In real firmware: wait motor settle + RF blanking window */
    return measure_gamma();
}

void rf_match_loop_tick(rf_system_t *sys)
{
    if (!sys->sp.match_auto) {
        return;
    }
    if (sys->state != RF_STATE_RAMP && sys->state != RF_STATE_RUN) {
        return;
    }

    static int step_size = 32;
    float g0 = measure_gamma();
    if (g0 < 0.05f) {
        /* |Γ| < 0.05 ≈ VSWR < 1.1 — good enough */
        step_size = 32;
        return;
    }

    const int axes[2] = {AXIS_CS, AXIS_CP};
    for (int i = 0; i < 2; ++i) {
        int axis = axes[i];
        float gp = try_step(axis, step_size);
        float gm = try_step(axis, -2 * step_size);
        /* After -2Δ we are at -Δ relative to start; choose best */
        if (g0 <= gp && g0 <= gm) {
            hw_stepper_move(axis, step_size); /* back to 0 */
        } else if (gp <= gm) {
            hw_stepper_move(axis, 2 * step_size); /* to +Δ */
            g0 = gp;
        } else {
            g0 = gm; /* stay at -Δ */
        }
    }

    if (g0 < 0.15f && step_size > 2) {
        step_size /= 2;
    } else if (g0 > 0.4f && step_size < 64) {
        step_size *= 2;
    }

    sys->tm.cs_pos = hw_stepper_pos(AXIS_CS);
    sys->tm.cp_pos = hw_stepper_pos(AXIS_CP);
}

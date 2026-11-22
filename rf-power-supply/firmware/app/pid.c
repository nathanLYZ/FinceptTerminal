#include <stdbool.h>

#include "rf_api.h"

float rf_pid_step(pid_t *pid, float error, float dt_s)
{
    if (dt_s <= 0.0f) {
        return 0.0f;
    }

    float d_err = (error - pid->prev_e) / dt_s;
    float p_term = pid->kp * error;
    float d_term = pid->kd * d_err;

    /* Integrate with conditional anti-windup */
    float u_unsat = p_term + pid->ki * pid->integr + d_term;
    bool blocking_high = (u_unsat >= pid->out_max && error > 0.0f);
    bool blocking_low = (u_unsat <= pid->out_min && error < 0.0f);
    if (!blocking_high && !blocking_low) {
        pid->integr += error * dt_s;
    }

    float u = p_term + pid->ki * pid->integr + d_term;
    if (u > pid->out_max) {
        u = pid->out_max;
    } else if (u < pid->out_min) {
        u = pid->out_min;
    }

    pid->prev_prev_e = pid->prev_e;
    pid->prev_e = error;
    return u;
}

float rf_gamma_mag(float pf_w, float pr_w)
{
    /* |Γ|^2 = Pr/Pf only holds for Pr<=Pf. Use Pr/(Pf+Pr) so the
       matcher still sees a gradient when the load is badly mistuned. */
    if (pf_w < 0.0f) {
        pf_w = 0.0f;
    }
    if (pr_w < 0.0f) {
        pr_w = 0.0f;
    }
    float sum = pf_w + pr_w;
    if (sum < 1.0f) {
        return 1.0f;
    }
    float g2 = pr_w / sum;
    if (g2 <= 0.0f) {
        return 0.0f;
    }
    /* sqrt without libm dependency for bare-metal portability */
    float y = g2;
    for (int i = 0; i < 8; ++i) {
        y = 0.5f * (y + g2 / y);
    }
    return y;
}

float rf_vswr_from_gamma(float gamma)
{
    if (gamma >= 0.999f) {
        return 99.0f;
    }
    return (1.0f + gamma) / (1.0f - gamma);
}

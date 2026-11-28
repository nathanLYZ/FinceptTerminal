#include "match_search.h"

#include "match_metrics.h"

static float measure(match_t *m)
{
    match_power_t p = hw_match_read_power();
    float g = match_cost_from_power(p);
    m->last_gamma = g;
    m->last_vswr = match_vswr(g);
    return g;
}

static bool in_limits(const match_t *m, match_axis_t axis, int32_t pos)
{
    return pos >= m->mech.min_pos[axis] && pos <= m->mech.max_pos[axis];
}

static float try_move(match_t *m, match_axis_t axis, int delta)
{
    int32_t next = m->mech.pos[axis] + delta;
    if (!in_limits(m, axis, next) || hw_match_hit_limit(axis, delta)) {
        m->fault |= MATCH_FAULT_LIMIT;
        return 2.0f; /* worse than any real gamma */
    }
    hw_match_move(axis, delta);
    m->mech.pos[axis] = next;
    hw_match_delay_ms(m->cfg.settle_ms);
    return measure(m);
}

static void adapt_step(match_t *m, float cost)
{
    if (cost < 0.15f && m->step > m->cfg.step_min) {
        m->step /= 2;
        if (m->step < m->cfg.step_min) {
            m->step = m->cfg.step_min;
        }
        m->state = MATCH_ST_FINE;
    } else if (cost > 0.4f && m->step < m->cfg.step_max) {
        m->step *= 2;
        if (m->step > m->cfg.step_max) {
            m->step = m->cfg.step_max;
        }
        m->state = MATCH_ST_COARSE;
    } else if (cost <= m->cfg.target_gamma) {
        m->state = MATCH_ST_TRACK;
        m->step = m->cfg.step0;
    }
}

void match_init(match_t *m, const match_cfg_t *cfg)
{
    m->cfg = *cfg;
    m->state = MATCH_ST_IDLE;
    m->step = cfg->step0;
    m->iters = 0;
    m->no_improve = 0;
    m->fault = 0;
    m->last_gamma = 1.0f;
    m->last_vswr = 99.0f;
    for (int i = 0; i < MATCH_AXIS_COUNT; ++i) {
        m->mech.min_pos[i] = 0;
        m->mech.max_pos[i] = 1023;
        m->mech.pos[i] = 512;
        m->mech.preset[i] = 512;
    }
}

void match_set_enable(match_t *m, bool on)
{
    m->enable = on;
    if (on && m->state == MATCH_ST_IDLE) {
        m->state = MATCH_ST_PRESET;
        m->iters = 0;
        m->no_improve = 0;
        m->fault = 0;
        m->step = m->cfg.step0;
    }
    if (!on) {
        m->state = MATCH_ST_IDLE;
    }
}

void match_load_preset(match_t *m)
{
    for (int i = 0; i < MATCH_AXIS_COUNT; ++i) {
        int delta = (int)(m->mech.preset[i] - m->mech.pos[i]);
        if (delta != 0) {
            hw_match_move((match_axis_t)i, delta);
            m->mech.pos[i] = m->mech.preset[i];
        }
    }
    hw_match_delay_ms(m->cfg.settle_ms);
    m->state = MATCH_ST_COARSE;
}

void match_tick(match_t *m)
{
    if (!m->enable || m->state == MATCH_ST_IDLE || m->state == MATCH_ST_FAULT) {
        return;
    }

    if (m->state == MATCH_ST_PRESET) {
        match_load_preset(m);
        return;
    }

    float c0 = measure(m);
    if (c0 <= m->cfg.target_gamma) {
        m->state = MATCH_ST_TRACK;
        m->no_improve = 0;
        return;
    }

    float best_before = c0;
    for (int ax = 0; ax < MATCH_AXIS_COUNT; ++ax) {
        match_axis_t axis = (match_axis_t)ax;
        int d = m->step;

        /* Probe +: move +d, remember cost, then go to -d relative to start
           by moving -2d, measure, then decide. */
        float cp = try_move(m, axis, d);
        float cm = try_move(m, axis, -2 * d);

        if (c0 <= cp && c0 <= cm) {
            /* Original was best — return to 0 relative start (= +d from here) */
            try_move(m, axis, d);
        } else if (cp <= cm) {
            /* + side better — from -d go to +d with +2d */
            try_move(m, axis, 2 * d);
            c0 = cp;
        } else {
            /* stay at -d */
            c0 = cm;
        }
    }

    m->iters++;
    if (c0 < best_before - 1e-4f) {
        m->no_improve = 0;
    } else {
        m->no_improve++;
    }

    adapt_step(m, c0);

    if (m->iters > m->cfg.timeout_iters && c0 > m->cfg.target_gamma) {
        m->fault |= MATCH_FAULT_TIMEOUT;
        m->state = MATCH_ST_FAULT;
    }
}

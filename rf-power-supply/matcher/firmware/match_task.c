/**
 * Example periodic task body — call from FreeRTOS every 20 ms.
 */
#include "match_search.h"

#include <stdio.h>

void hw_match_stub_set_pos(int cs, int cp);
void hw_match_stub_get_pos(int *cs, int *cp);

int main(void)
{
    match_t m;
    match_cfg_t cfg = {
        .target_gamma = 0.05f,
        .coarse_gamma = 0.15f,
        .step0 = 32,
        .step_min = 2,
        .step_max = 64,
        .timeout_iters = 200,
        .settle_ms = 0,
    };
    match_init(&m, &cfg);
    hw_match_stub_set_pos(200, 800);
    m.mech.pos[0] = 200;
    m.mech.pos[1] = 800;
    m.mech.preset[0] = 200; /* deliberately bad preset → must search */
    m.mech.preset[1] = 800;

    match_set_enable(&m, true);

    printf("matcher demo: start Cs=%ld Cp=%ld\n", (long)m.mech.pos[0],
           (long)m.mech.pos[1]);

    for (int i = 0; i < 80; ++i) {
        match_tick(&m);
        if ((i % 10) == 0 || m.state == MATCH_ST_TRACK || m.state == MATCH_ST_FAULT) {
            int cs, cp;
            hw_match_stub_get_pos(&cs, &cp);
            printf("i=%02d state=%d |G|=%.3f VSWR=%.2f Cs=%d Cp=%d step=%d\n", i,
                   (int)m.state, m.last_gamma, m.last_vswr, cs, cp, m.step);
        }
        if (m.state == MATCH_ST_TRACK || m.state == MATCH_ST_FAULT) {
            break;
        }
    }

    return (m.last_gamma <= cfg.target_gamma) ? 0 : 1;
}

/**
 * Desktop simulation entry — compiles with host GCC for algorithm bring-up.
 * On STM32, replace with CubeMX startup and FreeRTOS tasks calling the same ticks.
 */
#include <stdio.h>
#include <unistd.h>

#include "rf_api.h"

int main(void)
{
    rf_system_t sys;
    rf_system_init(&sys);

    sys.sp.power_set_w = 500.0f;
    sys.sp.prefl_trip_w = 400.0f; /* allow headroom while matcher converges */
    sys.sp.rf_on_cmd = true;
    sys.sp.match_auto = true;

    const float dt = 0.001f; /* 1 ms power tick */
    for (int n = 0; n < 8000; ++n) {
        rf_state_machine_tick(&sys, dt);
        rf_power_loop_tick(&sys, dt);
        if ((n % 20) == 0) {
            rf_match_loop_tick(&sys);
        }
        if ((n % 800) == 0) {
            printf("t=%4.1fs state=%d Pf=%6.1f Pr=%5.1f VSWR=%4.2f Vbus=%5.1f Cs=%ld Cp=%ld\n",
                   n * dt, (int)sys.state, sys.tm.pf_w, sys.tm.pr_w, sys.tm.vswr,
                   sys.tm.vbus_v, (long)hw_stepper_pos(0), (long)hw_stepper_pos(1));
        }
    }
    return 0;
}

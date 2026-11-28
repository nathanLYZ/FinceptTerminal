#ifndef MATCH_METRICS_H
#define MATCH_METRICS_H

#include "match_types.h"

float match_gamma(float pf_w, float pr_w);
float match_vswr(float gamma);
float match_cost_from_power(match_power_t p);

#endif /* MATCH_METRICS_H */

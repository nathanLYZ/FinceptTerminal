#include "match_metrics.h"

float match_gamma(float pf_w, float pr_w)
{
    /* Use Pr/(Pf+Pr) so cost stays graded when Pr > Pf (severe mismatch). */
    if (pf_w < 0.0f) {
        pf_w = 0.0f;
    }
    if (pr_w < 0.0f) {
        pr_w = 0.0f;
    }
    float sum = pf_w + pr_w;
    if (sum < 1.0f) {
        return 1.0f; /* no RF — treat as fully mismatched */
    }
    float g2 = pr_w / sum;
    if (g2 <= 0.0f) {
        return 0.0f;
    }
    /* Newton sqrt, no libm required */
    float y = g2;
    for (int i = 0; i < 8; ++i) {
        y = 0.5f * (y + g2 / y);
    }
    return y;
}

float match_vswr(float gamma)
{
    if (gamma >= 0.999f) {
        return 99.0f;
    }
    return (1.0f + gamma) / (1.0f - gamma);
}

float match_cost_from_power(match_power_t p)
{
    return match_gamma(p.pf_w, p.pr_w);
}

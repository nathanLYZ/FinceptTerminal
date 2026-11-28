#pragma once

#include "match/types.hpp"

namespace rf::match {

/** |Γ| from Pf/Pr using Pr/(Pf+Pr) so severe mismatch still has gradient. */
inline float gammaFromPower(float pf_w, float pr_w) {
  if (pf_w < 0.f) pf_w = 0.f;
  if (pr_w < 0.f) pr_w = 0.f;
  const float sum = pf_w + pr_w;
  if (sum < 1.f) return 1.f;
  float g2 = pr_w / sum;
  if (g2 <= 0.f) return 0.f;
  float y = g2;
  for (int i = 0; i < 8; ++i) {
    y = 0.5f * (y + g2 / y);
  }
  return y;
}

inline float vswrFromGamma(float g) {
  if (g >= 0.999f) return 99.f;
  return (1.f + g) / (1.f - g);
}

inline float cost(const PowerSample& p) {
  return gammaFromPower(p.pf_w, p.pr_w);
}

}  // namespace rf::match

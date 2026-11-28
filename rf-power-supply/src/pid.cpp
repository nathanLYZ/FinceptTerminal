#include "rf1k/pid.hpp"

namespace rf1k {

float pidStep(Pid& pid, float error, float dt_s) {
  if (dt_s <= 0.f) return 0.f;

  const float p_term = pid.kp * error;
  const float d_term = pid.kd * (error - pid.prev_e) / dt_s;
  const float u_unsat = p_term + pid.ki * pid.integr + d_term;

  const bool block_hi = (u_unsat >= pid.out_max && error > 0.f);
  const bool block_lo = (u_unsat <= pid.out_min && error < 0.f);
  if (!block_hi && !block_lo) {
    pid.integr += error * dt_s;
  }

  float u = p_term + pid.ki * pid.integr + d_term;
  if (u > pid.out_max) u = pid.out_max;
  if (u < pid.out_min) u = pid.out_min;

  pid.prev_e = error;
  return u;
}

}  // namespace rf1k

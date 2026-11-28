#pragma once

#include "rf1k/types.hpp"

namespace rf1k {

float pidStep(Pid& pid, float error, float dt_s);

}  // namespace rf1k

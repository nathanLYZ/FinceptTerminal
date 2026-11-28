#pragma once

#include "rf1k/platform/board_config.hpp"
#include "rf1k/system.hpp"

namespace rf1k {

/** Apply ≤50 W dummy-load bring-up setpoints to a System. */
inline void applyBringup50W(System& sys, const BringupLimits& lim = {}) {
  auto& sp = sys.setpoints();
  sp.bringup_50w = true;
  sp.power_limit_w = lim.power_limit_w;
  sp.ramp_w_s = lim.ramp_w_s;
  sp.prefl_trip_w = lim.prefl_trip_w;
  sp.vswr_warn = lim.vswr_warn;
  if (sp.power_set_w > lim.power_limit_w) {
    sp.power_set_w = lim.power_limit_w;
  }
  // Soften PID authority during first light
  // (bus clamp enforced in BoardHw::setBusVoltage via cal.bringup.max_vbus_v
  //  when bringup flag is observed by the app — System clamps power only.)
}

inline void startBringupRf(System& sys, float power_w, const BringupLimits& lim = {}) {
  applyBringup50W(sys, lim);
  auto& sp = sys.setpoints();
  sp.power_set_w = power_w > lim.power_limit_w ? lim.power_limit_w : power_w;
  sp.match_auto = true;
  sp.rf_on = true;
}

}  // namespace rf1k

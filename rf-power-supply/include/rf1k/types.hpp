#pragma once

#include <cstdint>

namespace rf1k {

enum class State : std::uint8_t {
  Idle = 0,
  Ramp = 1,
  Run = 2,
  Fault = 3
};

enum Fault : std::uint32_t {
  None = 0,
  Oc = 1u << 0,
  Ov = 1u << 1,
  HighRefl = 1u << 2,
  OtPa = 1u << 3,
  OtPsu = 1u << 4,
  MatchTimeout = 1u << 5,
  Interlock = 1u << 6,
  Arc = 1u << 7,
  CommWatchdog = 1u << 8
};

struct Setpoints {
  float power_set_w{0.f};
  float ramp_w_s{200.f};
  float prefl_trip_w{200.f};
  float vswr_warn{2.f};
  bool rf_on{false};
  bool match_auto{true};
  bool clear_fault{false};
};

struct Telemetry {
  float pf_w{0.f};
  float pr_w{0.f};
  float vswr{1.f};
  float vbus_v{0.f};
  float temp_pa_c{25.f};
  int cs_pos{512};
  int cp_pos{512};
};

struct Pid {
  float kp{0.05f};
  float ki{0.25f};
  float kd{0.f};
  float out_min{0.f};
  float out_max{60.f};  // bus volts
  float integr{0.f};
  float prev_e{0.f};
};

}  // namespace rf1k

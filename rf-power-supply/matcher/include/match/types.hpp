#pragma once

#include <cstdint>

namespace rf::match {

enum class Axis : std::uint8_t { Cs = 0, Cp = 1, Count = 2 };

enum class State : std::uint8_t {
  Idle = 0,
  Preset,
  Coarse,
  Fine,
  Track,
  Fault
};

enum Fault : std::uint32_t {
  None = 0,
  Timeout = 1u << 0,
  Limit = 1u << 1,
  Sensor = 1u << 2
};

struct PowerSample {
  float pf_w{0.f};
  float pr_w{0.f};
};

struct Config {
  float target_gamma{0.05f};  // ~VSWR 1.1
  float coarse_gamma{0.15f};
  int step0{32};
  int step_min{2};
  int step_max{64};
  std::uint32_t timeout_iters{200};
  std::uint32_t settle_ms{20};
  std::uint32_t period_ms{20};  // FreeRTOS task period
};

struct Mechanics {
  int pos[static_cast<int>(Axis::Count)]{512, 512};
  int min_pos[static_cast<int>(Axis::Count)]{0, 0};
  int max_pos[static_cast<int>(Axis::Count)]{1023, 1023};
  int preset[static_cast<int>(Axis::Count)]{512, 512};
};

/** Hardware port — implement on the board; stubbed on host. */
class IActuator {
 public:
  virtual ~IActuator() = default;
  virtual PowerSample readPower() = 0;
  virtual void move(Axis axis, int delta_steps) = 0;
  virtual void delayMs(std::uint32_t ms) = 0;
  virtual bool hitLimit(Axis axis, int direction) = 0;
};

}  // namespace rf::match

#pragma once

#include "match/types.hpp"

namespace rf::match {

/** Host plant: reflection falls as Cs/Cp approach 512/512. */
class StubActuator final : public IActuator {
 public:
  explicit StubActuator(float p_avail_w = 100.f) : p_avail_(p_avail_w) {}

  void setPos(int cs, int cp) {
    pos_[0] = cs;
    pos_[1] = cp;
  }

  int pos(Axis axis) const { return pos_[static_cast<int>(axis)]; }

  PowerSample readPower() override {
    const float err = static_cast<float>(
        (pos_[0] - 512) * (pos_[0] - 512) + (pos_[1] - 512) * (pos_[1] - 512));
    const float gamma = err / (err + 50000.f);
    return PowerSample{p_avail_ * (1.f - gamma), p_avail_ * gamma};
  }

  void move(Axis axis, int delta_steps) override {
    pos_[static_cast<int>(axis)] += delta_steps;
  }

  void delayMs(std::uint32_t) override {}

  bool hitLimit(Axis axis, int) override {
    const int p = pos_[static_cast<int>(axis)];
    return p < 0 || p > 1023;
  }

 private:
  float p_avail_;
  int pos_[2]{200, 800};
};

}  // namespace rf::match

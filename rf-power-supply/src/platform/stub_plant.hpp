#pragma once

#include "rf1k/match/types.hpp"
#include "rf1k/platform/hw_port.hpp"

namespace rf1k {

/**
 * Host plant: implements RF HW + match actuator on one simulated PA/match.
 * Matched center = (512,512); bus 60 V * amp ≈ 1000 W.
 */
class StubPlant final : public IHardware, public match::IActuator {
 public:
  float readPfW() override {
    update();
    return pf_;
  }
  float readPrW() override {
    update();
    return pr_;
  }
  float readVbusV() override { return vbus_; }
  float readTempPaC() override { return 35.f + pf_ * 0.03f; }
  bool interlockOk() override { return interlock_ok_; }
  bool faultLatched() override { return false; }

  void setBusVoltage(float volts) override { vbus_ = volts; }
  void setDdsAmplitude(float a) override { amp_ = a; }
  void rfEnable(bool on) override { rf_on_ = on; }
  void faultClear() override {}

  void stepperMove(int axis, int delta) override {
    if (axis >= 0 && axis < 2) pos_[axis] += delta;
  }
  int stepperPos(int axis) override {
    return (axis >= 0 && axis < 2) ? pos_[axis] : 0;
  }

  // match::IActuator
  match::PowerSample readPower() override {
    update();
    return {pf_, pr_};
  }
  void move(match::Axis axis, int delta_steps) override {
    stepperMove(static_cast<int>(axis), delta_steps);
  }
  void delayMs(std::uint32_t) override {}
  bool hitLimit(match::Axis axis, int) override {
    const int p = pos_[static_cast<int>(axis)];
    return p < 0 || p > 1023;
  }

  void setMatchPos(int cs, int cp) {
    pos_[0] = cs;
    pos_[1] = cp;
  }

 private:
  void update() {
    if (!rf_on_ || amp_ <= 0.f) {
      pf_ = 0.f;
      pr_ = 0.f;
      return;
    }
    const float err = static_cast<float>(
        (pos_[0] - 512) * (pos_[0] - 512) + (pos_[1] - 512) * (pos_[1] - 512));
    const float gamma = err / (err + 50000.f);
    const float pavail = (vbus_ / 60.f) * 1000.f * amp_;
    pf_ = pavail * (1.f - gamma);
    pr_ = pavail * gamma;
  }

  bool rf_on_{false};
  bool interlock_ok_{true};
  float vbus_{0.f};
  float amp_{0.f};
  float pf_{0.f};
  float pr_{0.f};
  int pos_[2]{200, 800};
};

}  // namespace rf1k

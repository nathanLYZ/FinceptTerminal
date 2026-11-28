#pragma once

#include "rf1k/match/types.hpp"

namespace rf1k::match {

/**
 * Coordinate-alternating auto-matcher.
 * Call tick() from a FreeRTOS task every Config::period_ms.
 */
class Search {
 public:
  Search(IActuator& hw, Config cfg = {});

  void setEnable(bool on);
  void setPreset(int cs, int cp);
  void setMechanicsPos(int cs, int cp);

  void tick();

  State state() const { return state_; }
  std::uint32_t fault() const { return fault_; }
  float lastGamma() const { return last_gamma_; }
  float lastVswr() const { return last_vswr_; }
  int step() const { return step_; }
  const Mechanics& mech() const { return mech_; }
  bool enabled() const { return enable_; }

 private:
  float measure();
  bool inLimits(Axis axis, int pos) const;
  float tryMove(Axis axis, int delta);
  void loadPreset();
  void adaptStep(float cost);

  IActuator& hw_;
  Config cfg_;
  Mechanics mech_{};
  bool enable_{false};
  State state_{State::Idle};
  int step_{32};
  std::uint32_t iters_{0};
  std::uint32_t no_improve_{0};
  std::uint32_t fault_{Fault::None};
  float last_gamma_{1.f};
  float last_vswr_{99.f};
};

}  // namespace rf1k::match

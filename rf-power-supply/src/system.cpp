#include "rf1k/system.hpp"

#include "rf1k/metrics.hpp"
#include "rf1k/pid.hpp"

namespace rf1k {
namespace {

match::Config defaultMatchCfg() {
  match::Config mc;
  mc.target_gamma = 0.05f;
  mc.settle_ms = 0;
  mc.step0 = 32;
  mc.step_min = 2;
  mc.step_max = 64;
  mc.timeout_iters = 400;
  return mc;
}

}  // namespace

System::System(IHardware& hw, match::IActuator& match_hw)
    : hw_(hw), matcher_(match_hw, defaultMatchCfg()) {}

void System::init() {
  mb_.init();
  state_ = State::Idle;
  fault_ = Fault::None;
  ramp_power_w_ = 0.f;
  power_pid_ = Pid{};
  matcher_.setEnable(false);
  matcher_.setMechanicsPos(hw_.stepperPos(0), hw_.stepperPos(1));
  matcher_.setPreset(hw_.stepperPos(0), hw_.stepperPos(1));
}

void System::onFault(std::uint32_t bits) {
  if (!bits) return;
  fault_ |= bits;
  state_ = State::Fault;
  sp_.rf_on = false;
  ramp_power_w_ = 0.f;
  hw_.rfEnable(false);
  hw_.setDdsAmplitude(0.f);
  matcher_.setEnable(false);
}

void System::clearFault() {
  if (!hw_.interlockOk()) return;
  hw_.faultClear();
  fault_ = Fault::None;
  if (state_ == State::Fault) state_ = State::Idle;
}

void System::stateMachine(float dt_s) {
  if (sp_.clear_fault) {
    clearFault();
    sp_.clear_fault = false;
  }

  switch (state_) {
    case State::Idle:
      hw_.rfEnable(false);
      ramp_power_w_ = 0.f;
      if (sp_.rf_on && sp_.power_set_w > 0.f && hw_.interlockOk()) {
        state_ = State::Ramp;
        power_pid_.integr = 0.f;
        if (sp_.match_auto) matcher_.setEnable(true);
      }
      break;
    case State::Ramp:
      if (!sp_.rf_on) {
        state_ = State::Idle;
        break;
      }
      ramp_power_w_ += sp_.ramp_w_s * dt_s;
      if (ramp_power_w_ >= sp_.power_set_w) {
        ramp_power_w_ = sp_.power_set_w;
        state_ = State::Run;
      }
      break;
    case State::Run:
      if (!sp_.rf_on) {
        state_ = State::Idle;
        break;
      }
      if (ramp_power_w_ < sp_.power_set_w) {
        ramp_power_w_ += sp_.ramp_w_s * dt_s;
        if (ramp_power_w_ > sp_.power_set_w) ramp_power_w_ = sp_.power_set_w;
      } else if (ramp_power_w_ > sp_.power_set_w) {
        ramp_power_w_ -= sp_.ramp_w_s * dt_s;
        if (ramp_power_w_ < sp_.power_set_w) ramp_power_w_ = sp_.power_set_w;
      }
      break;
    case State::Fault:
      hw_.rfEnable(false);
      break;
  }
}

void System::powerLoop(float dt_s) {
  tm_.pf_w = hw_.readPfW();
  tm_.pr_w = hw_.readPrW();
  tm_.vbus_v = hw_.readVbusV();
  tm_.temp_pa_c = hw_.readTempPaC();
  tm_.cs_pos = hw_.stepperPos(0);
  tm_.cp_pos = hw_.stepperPos(1);
  tm_.vswr = vswrFromGamma(gammaMag(tm_.pf_w, tm_.pr_w));

  if (!hw_.interlockOk()) {
    onFault(Fault::Interlock);
    return;
  }
  if (tm_.pr_w > sp_.prefl_trip_w) {
    onFault(Fault::HighRefl);
    return;
  }
  if (tm_.temp_pa_c > 95.f) {
    onFault(Fault::OtPa);
    return;
  }

  if (state_ != State::Ramp && state_ != State::Run) {
    hw_.setBusVoltage(0.f);
    hw_.setDdsAmplitude(0.f);
    return;
  }

  const float error = ramp_power_w_ - tm_.pf_w;
  const float vbus = pidStep(power_pid_, error, dt_s);
  hw_.setBusVoltage(vbus);
  hw_.setDdsAmplitude(0.95f);
  hw_.rfEnable(true);

  if (tm_.pr_w > 0.35f * tm_.pf_w && ramp_power_w_ > 100.f && tm_.pf_w > 20.f) {
    ramp_power_w_ *= 0.995f;
  }
}

void System::tickPower(float dt_s) {
  stateMachine(dt_s);
  powerLoop(dt_s);
}

void System::tickMatch() {
  if (sp_.match_auto && (state_ == State::Ramp || state_ == State::Run)) {
    if (!matcher_.enabled()) matcher_.setEnable(true);
    matcher_.tick();
    tm_.cs_pos = matcher_.mech().pos[0];
    tm_.cp_pos = matcher_.mech().pos[1];
  }
}

void System::tickComm() {
  mb_.applyHolding(sp_);
  mb_.exportTelemetry(state_, tm_, fault_);
}

}  // namespace rf1k

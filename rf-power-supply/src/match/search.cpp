#include "rf1k/match/search.hpp"

#include "rf1k/match/metrics.hpp"

namespace rf1k::match {

Search::Search(IActuator& hw, Config cfg) : hw_(hw), cfg_(cfg) {
  step_ = cfg_.step0;
}

void Search::setEnable(bool on) {
  enable_ = on;
  if (on && state_ == State::Idle) {
    state_ = State::Preset;
    iters_ = 0;
    no_improve_ = 0;
    fault_ = Fault::None;
    step_ = cfg_.step0;
  }
  if (!on) {
    state_ = State::Idle;
  }
}

void Search::setPreset(int cs, int cp) {
  mech_.preset[0] = cs;
  mech_.preset[1] = cp;
}

void Search::setMechanicsPos(int cs, int cp) {
  mech_.pos[0] = cs;
  mech_.pos[1] = cp;
}

float Search::measure() {
  const PowerSample p = hw_.readPower();
  last_gamma_ = cost(p);
  last_vswr_ = vswrFromGamma(last_gamma_);
  return last_gamma_;
}

bool Search::inLimits(Axis axis, int pos) const {
  const int i = static_cast<int>(axis);
  return pos >= mech_.min_pos[i] && pos <= mech_.max_pos[i];
}

float Search::tryMove(Axis axis, int delta) {
  const int i = static_cast<int>(axis);
  const int next = mech_.pos[i] + delta;
  if (!inLimits(axis, next) || hw_.hitLimit(axis, delta)) {
    fault_ |= Fault::Limit;
    return 2.f;
  }
  hw_.move(axis, delta);
  mech_.pos[i] = next;
  hw_.delayMs(cfg_.settle_ms);
  return measure();
}

void Search::loadPreset() {
  for (int i = 0; i < static_cast<int>(Axis::Count); ++i) {
    const int delta = mech_.preset[i] - mech_.pos[i];
    if (delta != 0) {
      hw_.move(static_cast<Axis>(i), delta);
      mech_.pos[i] = mech_.preset[i];
    }
  }
  hw_.delayMs(cfg_.settle_ms);
  state_ = State::Coarse;
}

void Search::adaptStep(float c) {
  if (c < cfg_.coarse_gamma && step_ > cfg_.step_min) {
    step_ /= 2;
    if (step_ < cfg_.step_min) step_ = cfg_.step_min;
    state_ = State::Fine;
  } else if (c > 0.4f && step_ < cfg_.step_max) {
    step_ *= 2;
    if (step_ > cfg_.step_max) step_ = cfg_.step_max;
    state_ = State::Coarse;
  } else if (c <= cfg_.target_gamma) {
    state_ = State::Track;
    step_ = cfg_.step0;
  }
}

void Search::tick() {
  if (!enable_ || state_ == State::Idle || state_ == State::Fault) {
    return;
  }

  if (state_ == State::Preset) {
    loadPreset();
    return;
  }

  float c0 = measure();
  if (c0 <= cfg_.target_gamma) {
    state_ = State::Track;
    no_improve_ = 0;
    return;
  }

  const float before = c0;
  for (int ax = 0; ax < static_cast<int>(Axis::Count); ++ax) {
    const Axis axis = static_cast<Axis>(ax);
    const int d = step_;

    const float cp = tryMove(axis, d);
    const float cm = tryMove(axis, -2 * d);

    if (c0 <= cp && c0 <= cm) {
      tryMove(axis, d);  // back to original
    } else if (cp <= cm) {
      tryMove(axis, 2 * d);  // to +d
      c0 = cp;
    } else {
      c0 = cm;  // stay at -d
    }
  }

  ++iters_;
  if (c0 < before - 1e-4f) {
    no_improve_ = 0;
  } else {
    ++no_improve_;
  }

  adaptStep(c0);

  if (iters_ > cfg_.timeout_iters && c0 > cfg_.target_gamma) {
    fault_ |= Fault::Timeout;
    state_ = State::Fault;
  }
}

}  // namespace rf1k::match

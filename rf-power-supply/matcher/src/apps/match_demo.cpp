/**
 * Host demo of the C++ matcher (same algorithm as FreeRTOS task body).
 * On MCU: construct StubActuator → real BoardActuator, then MatchTask::start().
 */
#include "match/freertos_task.hpp"
#include "match/search.hpp"
#include "stub_actuator.hpp"

#include <cstdio>

int main() {
  using namespace rf::match;

  StubActuator hw(100.f);
  hw.setPos(200, 800);

  Config cfg;
  cfg.target_gamma = 0.05f;
  cfg.settle_ms = 0;  // host: no settle wait
  cfg.step0 = 32;

  Search search(hw, cfg);
  search.setMechanicsPos(200, 800);
  search.setPreset(200, 800);  // bad preset → must search
  search.setEnable(true);

  std::printf("C++ matcher demo: start Cs=%d Cp=%d\n", hw.pos(Axis::Cs),
              hw.pos(Axis::Cp));

  for (int i = 0; i < 80; ++i) {
    search.tick();
    if (i % 10 == 0 || search.state() == State::Track ||
        search.state() == State::Fault) {
      std::printf(
          "i=%02d state=%d |G|=%.3f VSWR=%.2f Cs=%d Cp=%d step=%d\n", i,
          static_cast<int>(search.state()), search.lastGamma(), search.lastVswr(),
          hw.pos(Axis::Cs), hw.pos(Axis::Cp), search.step());
    }
    if (search.state() == State::Track || search.state() == State::Fault) {
      break;
    }
  }

  // Also show how the FreeRTOS helper is used on host:
  // MatchTask::start(search, cfg.period_ms);

  return search.lastGamma() <= cfg.target_gamma ? 0 : 1;
}

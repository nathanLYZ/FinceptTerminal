/**
 * ≤50 W dummy-load bring-up procedure (host plant).
 * Steps: apply bring-up limits → RF on @ 10 W → match → ramp to 50 W.
 */
#include "rf1k/bringup.hpp"
#include "rf1k/system.hpp"
#include "stub_plant.hpp"

#include <cstdio>

static void syncModbusFromSetpoints(rf1k::System& sys) {
  // tickComm() applies holding → setpoints; keep registers aligned.
  auto& mb = sys.modbus();
  const auto& sp = sys.setpoints();
  std::uint16_t cmd = 0;
  if (sp.rf_on) cmd |= rf1k::CmdRfOn;
  if (sp.match_auto) cmd |= rf1k::CmdMatchAuto;
  if (sp.clear_fault) cmd |= rf1k::CmdClearFault;
  mb.holding(rf1k::HCmdWord) = cmd;
  mb.holding(rf1k::HPowerSetW) =
      static_cast<std::uint16_t>(sp.power_set_w + 0.5f);
  mb.holding(rf1k::HRampWS) = static_cast<std::uint16_t>(sp.ramp_w_s + 0.5f);
  mb.holding(rf1k::HPreflTripW) =
      static_cast<std::uint16_t>(sp.prefl_trip_w + 0.5f);
}

static void spin(rf1k::System& sys, int ms) {
  for (int t = 0; t < ms; ++t) {
    if (t % 10 == 0) {
      syncModbusFromSetpoints(sys);
      sys.tickComm();
    }
    sys.tickPower(0.001f);
    if (t % 20 == 0) sys.tickMatch();
  }
}

int main() {
  rf1k::StubPlant plant;
  plant.setMatchPos(220, 780);  // slightly mistuned 50 Ω path

  rf1k::System sys(plant, plant);
  sys.init();

  rf1k::BringupLimits lim;
  lim.power_limit_w = 50.f;
  lim.ramp_w_s = 20.f;
  lim.prefl_trip_w = 15.f;

  std::printf("=== RF-1K ≤50 W dummy-load bring-up ===\n");

  // Phase 1: low power ignition + match
  rf1k::startBringupRf(sys, 10.f, lim);
  syncModbusFromSetpoints(sys);
  std::printf("phase1: RF on @ 10 W (limit=%g)\n", lim.power_limit_w);
  spin(sys, 3000);
  std::printf("  state=%d Pf=%.1f Pr=%.1f VSWR=%.2f Cs=%d Cp=%d\n",
              static_cast<int>(sys.state()), sys.telemetry().pf_w,
              sys.telemetry().pr_w, sys.telemetry().vswr, sys.telemetry().cs_pos,
              sys.telemetry().cp_pos);

  if (sys.state() == rf1k::State::Fault) {
    std::printf("FAIL fault during phase1: 0x%x\n", sys.fault());
    return 1;
  }

  // Phase 2: ramp to 50 W ceiling (request 200 — must clamp)
  sys.setpoints().power_set_w = 200.f;
  syncModbusFromSetpoints(sys);
  std::printf("phase2: request 200 W (must clamp to 50 W)\n");
  spin(sys, 5000);
  std::printf("  state=%d Pf=%.1f Pr=%.1f VSWR=%.2f limit=%g\n",
              static_cast<int>(sys.state()), sys.telemetry().pf_w,
              sys.telemetry().pr_w, sys.telemetry().vswr,
              sys.setpoints().power_limit_w);

  const bool ok = sys.state() == rf1k::State::Run &&
                  sys.telemetry().pf_w > 40.f && sys.telemetry().pf_w < 55.f &&
                  sys.telemetry().vswr < 1.3f && sys.fault() == 0;
  std::printf("%s bring-up50\n", ok ? "OK" : "FAIL");
  return ok ? 0 : 1;
}

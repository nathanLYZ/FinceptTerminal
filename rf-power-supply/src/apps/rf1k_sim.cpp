/**
 * Full RF-1K host simulation (C++): power ramp + auto-match + modbus map.
 */
#include "rf1k/modbus.hpp"
#include "rf1k/platform/freertos_app.hpp"
#include "stub_plant.hpp"
#include "rf1k/system.hpp"

#include <cstdio>

int main() {
  rf1k::StubPlant plant;
  plant.setMatchPos(200, 800);

  rf1k::System sys(plant, plant);
  sys.init();

  // Seed via Modbus holding registers
  sys.modbus().holding(rf1k::HPowerSetW) = 500;
  sys.modbus().holding(rf1k::HRampWS) = 200;
  sys.modbus().holding(rf1k::HPreflTripW) = 400;
  sys.modbus().holding(rf1k::HCmdWord) =
      static_cast<std::uint16_t>(rf1k::CmdRfOn | rf1k::CmdMatchAuto);
  sys.tickComm();

  std::printf("RF-1K C++ sim: 500 W with auto-match\n");

  for (int t = 0; t < 8000; ++t) {
    sys.tickPower(0.001f);
    if (t % 20 == 0) sys.tickMatch();
    if (t % 10 == 0) sys.tickComm();
    if (t % 800 == 0) {
      const auto& tm = sys.telemetry();
      std::printf(
          "t=%4.1fs state=%d Pf=%6.1f Pr=%5.1f VSWR=%4.2f Vbus=%5.1f Cs=%d Cp=%d\n",
          t * 0.001, static_cast<int>(sys.state()), tm.pf_w, tm.pr_w, tm.vswr,
          tm.vbus_v, tm.cs_pos, tm.cp_pos);
    }
  }

  sys.tickComm();
  std::printf("final: state=%d Pf=%.1f mb_pf=%u fault=0x%04x\n",
              static_cast<int>(sys.state()), sys.telemetry().pf_w,
              sys.modbus().input(rf1k::IPfW), sys.fault());

  return (sys.state() == rf1k::State::Run && sys.telemetry().pf_w > 450.f) ? 0
                                                                         : 1;
}

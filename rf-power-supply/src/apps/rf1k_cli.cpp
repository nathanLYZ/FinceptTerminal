/**
 * Interactive C++ console for RF-1K host plant.
 * Commands: on <W>, set <W>, off, status, clear, quit
 */
#include "stub_plant.hpp"
#include "rf1k/system.hpp"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

static void printStatus(const rf1k::System& sys) {
  const auto& tm = sys.telemetry();
  std::printf(
      "state=%d  Pf=%7.1f W  Pr=%6.1f W  VSWR=%5.2f  Tpa=%5.1f  "
      "fault=0x%04x  Cs=%d Cp=%d\n",
      static_cast<int>(sys.state()), tm.pf_w, tm.pr_w, tm.vswr, tm.temp_pa_c,
      sys.fault(), tm.cs_pos, tm.cp_pos);
}

static void advance(rf1k::System& sys, int ms) {
  for (int t = 0; t < ms; ++t) {
    sys.tickPower(0.001f);
    if (t % 20 == 0) sys.tickMatch();
    if (t % 10 == 0) sys.tickComm();
  }
}

int main() {
  rf1k::StubPlant plant;
  plant.setMatchPos(200, 800);
  rf1k::System sys(plant, plant);
  sys.init();
  sys.setpoints().match_auto = true;
  sys.setpoints().prefl_trip_w = 400.f;

  std::printf("RF-1K C++ console. Commands: on <W>, set <W>, off, status, clear, quit\n");

  std::string line;
  while (std::printf("> "), std::getline(std::cin, line)) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if (cmd.empty()) continue;
    if (cmd == "quit" || cmd == "exit" || cmd == "q") break;
    if (cmd == "on") {
      float w = 100.f;
      iss >> w;
      sys.setpoints().power_set_w = w;
      sys.setpoints().rf_on = true;
    } else if (cmd == "set") {
      float w = 0.f;
      iss >> w;
      sys.setpoints().power_set_w = w;
    } else if (cmd == "off") {
      sys.setpoints().rf_on = false;
      sys.setpoints().power_set_w = 0.f;
    } else if (cmd == "clear") {
      sys.setpoints().clear_fault = true;
    } else if (cmd == "status") {
      // fall through
    } else {
      std::printf("unknown command\n");
      continue;
    }
    advance(sys, 500);
    printStatus(sys);
  }
  return 0;
}

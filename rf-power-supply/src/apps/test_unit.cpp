/**
 * Unit tests for RF-1K C++ core (no external test framework).
 */
#include "rf1k/bringup.hpp"
#include "rf1k/match/metrics.hpp"
#include "rf1k/match/search.hpp"
#include "rf1k/metrics.hpp"
#include "rf1k/modbus.hpp"
#include "rf1k/pid.hpp"
#include "rf1k/system.hpp"
#include "stub_actuator.hpp"
#include "stub_plant.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace {

int g_fail = 0;

void expect(bool cond, const char* name) {
  if (!cond) {
    std::printf("FAIL  %s\n", name);
    ++g_fail;
  } else {
    std::printf("ok    %s\n", name);
  }
}

void expectNear(float got, float want, float eps, const char* name) {
  expect(std::fabs(got - want) <= eps, name);
}

void test_metrics() {
  expectNear(rf1k::gammaMag(100.f, 0.f), 0.f, 1e-4f, "gamma matched");
  expectNear(rf1k::gammaMag(50.f, 50.f), 0.7071f, 1e-3f, "gamma half power");
  expect(rf1k::gammaMag(0.f, 0.f) >= 0.99f, "gamma no RF");
  expectNear(rf1k::vswrFromGamma(0.f), 1.f, 1e-4f, "vswr matched");
  expect(rf1k::vswrFromGamma(0.05f) < 1.11f, "vswr 0.05");
  expect(rf1k::vswrFromGamma(0.999f) >= 90.f, "vswr open");
}

void test_pid() {
  rf1k::Pid pid;
  pid.kp = 0.5f;
  pid.ki = 0.0f;
  pid.kd = 0.0f;
  pid.out_min = 0.f;
  pid.out_max = 60.f;
  const float u = rf1k::pidStep(pid, 10.f, 0.001f);
  expectNear(u, 5.f, 1e-3f, "pid proportional");

  pid.integr = 0.f;
  pid.ki = 1000.f;  // integrate hard
  float last = 0.f;
  for (int i = 0; i < 100; ++i) last = rf1k::pidStep(pid, 1.f, 0.001f);
  expectNear(last, 60.f, 1e-2f, "pid antiwindup saturates max");
}

void test_matcher() {
  rf1k::match::StubActuator hw(100.f);
  hw.setPos(200, 800);
  rf1k::match::Config cfg;
  cfg.target_gamma = 0.05f;
  cfg.settle_ms = 0;
  cfg.step0 = 32;
  rf1k::match::Search search(hw, cfg);
  search.setMechanicsPos(200, 800);
  search.setPreset(200, 800);
  search.setEnable(true);

  bool ok = false;
  for (int i = 0; i < 80; ++i) {
    search.tick();
    if (search.state() == rf1k::match::State::Track &&
        search.lastGamma() <= 0.05f) {
      ok = true;
      break;
    }
  }
  expect(ok, "matcher converges Track");
  expect(search.lastVswr() < 1.15f, "matcher VSWR<1.15");
}

void test_system_sim() {
  rf1k::StubPlant plant;
  plant.setMatchPos(200, 800);
  rf1k::System sys(plant, plant);
  sys.init();
  sys.modbus().holding(rf1k::HPowerSetW) = 500;
  sys.modbus().holding(rf1k::HRampWS) = 200;
  sys.modbus().holding(rf1k::HPreflTripW) = 400;
  sys.modbus().holding(rf1k::HCmdWord) =
      static_cast<std::uint16_t>(rf1k::CmdRfOn | rf1k::CmdMatchAuto);

  for (int t = 0; t < 8000; ++t) {
    if (t % 10 == 0) sys.tickComm();
    sys.tickPower(0.001f);
    if (t % 20 == 0) sys.tickMatch();
  }
  sys.tickComm();

  expect(sys.state() == rf1k::State::Run, "system ends in Run");
  expect(sys.telemetry().pf_w > 450.f, "system Pf>450");
  expect(sys.telemetry().vswr < 1.2f, "system VSWR<1.2");
  expect(sys.fault() == 0, "system no fault");
  expect(sys.modbus().input(rf1k::IPfW) >= 450, "modbus Pf register");
}

void test_interlock_fault() {
  rf1k::StubPlant plant;
  plant.setMatchPos(512, 512);
  rf1k::System sys(plant, plant);
  sys.init();
  sys.setpoints().rf_on = true;
  sys.setpoints().power_set_w = 100.f;
  sys.setpoints().match_auto = false;
  sys.setpoints().prefl_trip_w = 500.f;

  // Run a bit matched
  for (int t = 0; t < 500; ++t) sys.tickPower(0.001f);
  expect(sys.state() == rf1k::State::Ramp || sys.state() == rf1k::State::Run,
         "running before interlock");

  // Force interlock open via reading — StubPlant has no setter; use reflection
  // trip instead by mistuning heavily under power.
  plant.setMatchPos(0, 1023);
  sys.setpoints().prefl_trip_w = 5.f;
  for (int t = 0; t < 50; ++t) sys.tickPower(0.001f);
  expect(sys.state() == rf1k::State::Fault, "high refl → Fault");
  expect((sys.fault() & rf1k::Fault::HighRefl) != 0, "fault bit HighRefl");
}

void test_bringup_50w_clamp() {
  rf1k::StubPlant plant;
  plant.setMatchPos(512, 512);
  rf1k::System sys(plant, plant);
  sys.init();
  rf1k::startBringupRf(sys, 10.f);
  expect(sys.setpoints().power_limit_w <= 50.f, "bringup limit≤50");
  sys.setpoints().power_set_w = 200.f;  // attempt over-limit
  for (int t = 0; t < 6000; ++t) {
    sys.tickPower(0.001f);
    if (t % 20 == 0) sys.tickMatch();
  }
  expect(sys.telemetry().pf_w < 55.f, "bringup Pf clamped <55");
  expect(sys.telemetry().pf_w > 35.f, "bringup Pf reached ~50");
  expect(sys.state() == rf1k::State::Run, "bringup ends Run");
}

void test_modbus_roundtrip() {
  rf1k::ModbusMap map;
  map.init();
  std::uint8_t req[8] = {1, 0x06, 0x00, 0x01, 0x00, 100, 0, 0};
  auto crc = rf1k::ModbusMap::crc16(req, 6);
  req[6] = static_cast<std::uint8_t>(crc & 0xff);
  req[7] = static_cast<std::uint8_t>(crc >> 8);
  std::uint8_t resp[64];
  const int n = map.processRtu(req, 8, resp, 64);
  expect(n == 8, "modbus rtu write len");
  expect(map.holding(rf1k::HPowerSetW) == 100, "modbus power=100");
}

}  // namespace

int main() {
  test_metrics();
  test_pid();
  test_matcher();
  test_modbus_roundtrip();
  test_system_sim();
  test_bringup_50w_clamp();
  test_interlock_fault();

  if (g_fail) {
    std::printf("%d test(s) failed\n", g_fail);
    return 1;
  }
  std::printf("all unit tests passed\n");
  return 0;
}

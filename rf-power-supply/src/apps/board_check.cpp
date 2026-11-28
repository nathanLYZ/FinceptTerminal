/**
 * Compile/link check for BoardHw against mock STM32 HAL.
 * Simulates detector ADC corresponding to ~10–50 W after cal defaults.
 */
#include "rf1k/bringup.hpp"
#include "rf1k/platform/board_hw.hpp"
#include "rf1k/system.hpp"

#include <cmath>
#include <cstdio>

/** Invert detToWatts roughly to pick an ADC code for a target wattage. */
static std::uint16_t wattsToAdc(float watts, float v0, float k) {
  if (watts < 1e-6f) watts = 1e-6f;
  const float db = 10.f * std::log10(watts * 1000.f);
  const float v = v0 + k * db;
  float raw = (v / 3.3f) * 65535.f;
  if (raw < 0.f) raw = 0.f;
  if (raw > 65535.f) raw = 65535.f;
  return static_cast<std::uint16_t>(raw);
}

int main() {
  rf1k::BoardCal cal = rf1k::defaultBoardCal();
  cal.bringup.max_vbus_v = 20.f;

  rf1k::BoardHw hw(cal);
  hw.init();

  // Inject ~10 W forward, low reflection
  const auto pf_adc =
      wattsToAdc(10.f, cal.det.pf_v0, cal.det.pf_k);
  const auto pr_adc =
      wattsToAdc(0.2f, cal.det.pr_v0, cal.det.pr_k);
  hw.setAdcRaw(pf_adc, pr_adc, 15000, 22000);

  std::printf("BoardHw check: Pf=%.2f W Pr=%.2f W Vbus_sense=%.1f T=%.1f\n",
              hw.readPfW(), hw.readPrW(), hw.readVbusV(), hw.readTempPaC());
  std::printf("interlock=%d fault_latch=%d\n", hw.interlockOk(),
              hw.faultLatched());

  hw.setBusVoltage(25.f);  // should clamp to bringup max 20 V
  std::printf("bus_cmd after 25V request: %.1f V (expect ≤20)\n",
              hw.lastBusCmd());

  hw.rfEnable(true);
  hw.stepperMove(0, 5);
  hw.stepperMove(1, -3);
  std::printf("stepper Cs=%d Cp=%d rf=%d\n", hw.stepperPos(0), hw.stepperPos(1),
              hw.rfEnabled());

  rf1k::System sys(hw, hw);
  sys.init();
  rf1k::startBringupRf(sys, 10.f, cal.bringup);

  // Feed rising power readings as bus comes up (crude open-loop inject)
  for (int t = 0; t < 2000; ++t) {
    const float target = sys.telemetry().pf_w;  // unused
    (void)target;
    // Keep ADC representing ~clamp(requested ramp, 50)
    float p = 10.f;
    if (t > 500) p = 25.f;
    if (t > 1200) p = 48.f;
    hw.setAdcRaw(wattsToAdc(p, cal.det.pf_v0, cal.det.pf_k),
                 wattsToAdc(0.3f, cal.det.pr_v0, cal.det.pr_k), 18000, 23000);
    sys.tickPower(0.001f);
    if (t % 20 == 0) sys.tickMatch();
  }

  std::printf("bringup loop: state=%d Pf=%.1f bus_cmd=%.1f\n",
              static_cast<int>(sys.state()), sys.telemetry().pf_w,
              hw.lastBusCmd());

  const bool ok = hw.lastBusCmd() <= 20.01f && hw.interlockOk() &&
                  hw.readPfW() > 1.f;
  std::printf("%s board_check\n", ok ? "OK" : "FAIL");
  return ok ? 0 : 1;
}

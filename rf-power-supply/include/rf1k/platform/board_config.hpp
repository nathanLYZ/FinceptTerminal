#pragma once

#include <cstdint>

namespace rf1k {

/** Bring-up / dummy-load safety defaults. */
struct BringupLimits {
  float power_limit_w{50.f};
  float ramp_w_s{20.f};       // slower than production
  float prefl_trip_w{15.f};   // trip early on 50 Ω bring-up
  float vswr_warn{1.5f};
  float max_vbus_v{20.f};     // keep PA well below rated during first light
  float max_temp_pa_c{70.f};
};

/** Detector calibration: P_w = 10^((Vdet - V0) / k) / 1000 */
struct DetCal {
  float pf_v0{0.55f};
  float pf_k{0.025f};  // V per dB
  float pr_v0{0.55f};
  float pr_k{0.025f};
};

struct SenseCal {
  float vbus_fs_v{66.f};     // volts at full-scale ADC
  float temp_offset_c{-50.f};
  float temp_gain_c{100.f};  // °C across full scale
};

struct BoardCal {
  DetCal det{};
  SenseCal sense{};
  BringupLimits bringup{};
};

inline BoardCal defaultBoardCal() { return BoardCal{}; }

/** STM32H743 pin labels (CubeMX GPIO names expected on target). */
namespace pins {
constexpr const char* kAdcPf = "PA0";
constexpr const char* kAdcPr = "PA1";
constexpr const char* kAdcVbus = "PA2";
constexpr const char* kAdcIdc = "PA3";
constexpr const char* kAdcTempPa = "PC0";
constexpr const char* kDacVbus = "PA4";
constexpr const char* kRfEnable = "PB0";
constexpr const char* kFaultClr = "PB1";
constexpr const char* kFaultLatch = "PB2";
constexpr const char* kInterlock = "PB10";
constexpr const char* kStepCs = "PD12";
constexpr const char* kDirCs = "PD13";
constexpr const char* kStepCp = "PD14";
constexpr const char* kDirCp = "PD15";
constexpr const char* kEnStepper = "PE0";
constexpr const char* kDdsCs = "PE11";
constexpr const char* kDdsIoUpdate = "PE10";
constexpr const char* kDdsReset = "PE9";
constexpr const char* kRs485De = "PD7";
}  // namespace pins

}  // namespace rf1k

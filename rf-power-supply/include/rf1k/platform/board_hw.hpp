#pragma once

#include "rf1k/match/types.hpp"
#include "rf1k/platform/board_config.hpp"
#include "rf1k/platform/hw_port.hpp"

namespace rf1k {

/**
 * Board-level driver implementing IHardware + match::IActuator.
 *
 * - Host / CI: compile with -DRF_STM32_MOCK (uses mock HAL)
 * - Target: -DRF_TARGET_STM32 and CubeMX-generated main.h / HAL
 *
 * ADC DMA order: [Pf, Pr, Vbus, Idc, TempPa]
 */
class BoardHw final : public IHardware, public match::IActuator {
 public:
  explicit BoardHw(BoardCal cal = defaultBoardCal());

  void init();  // start ADC DMA, reset DDS, enable steppers
  void setCal(const BoardCal& cal) { cal_ = cal; }
  const BoardCal& cal() const { return cal_; }

  /** Inject raw ADC counts (mock / unit-test). On target, DMA fills these. */
  void setAdcRaw(std::uint16_t pf, std::uint16_t pr, std::uint16_t vbus,
                 std::uint16_t temp_pa);

  // IHardware
  float readPfW() override;
  float readPrW() override;
  float readVbusV() override;
  float readTempPaC() override;
  bool interlockOk() override;
  bool faultLatched() override;
  void setBusVoltage(float volts) override;
  void setDdsAmplitude(float norm_0_1) override;
  void rfEnable(bool on) override;
  void faultClear() override;
  void stepperMove(int axis, int delta_steps) override;
  int stepperPos(int axis) override;

  // match::IActuator
  match::PowerSample readPower() override;
  void move(match::Axis axis, int delta_steps) override;
  void delayMs(std::uint32_t ms) override;
  bool hitLimit(match::Axis axis, int direction) override;

  float lastBusCmd() const { return bus_cmd_v_; }
  float lastAmpCmd() const { return amp_cmd_; }
  bool rfEnabled() const { return rf_on_; }

 private:
  float adcToVolt(std::uint16_t raw) const;
  float detToWatts(float vdet, float v0, float k) const;
  void pulseStep(int axis);
  void writeDdsAmplitude(float norm);
  void writeDacBus(float volts);

  BoardCal cal_{};
  std::uint16_t adc_pf_{0};
  std::uint16_t adc_pr_{0};
  std::uint16_t adc_vbus_{0};
  std::uint16_t adc_temp_{0};
  int pos_[2]{512, 512};
  float bus_cmd_v_{0.f};
  float amp_cmd_{0.f};
  bool rf_on_{false};
  bool mock_interlock_ok_{true};
  bool mock_fault_latched_{false};
};

}  // namespace rf1k

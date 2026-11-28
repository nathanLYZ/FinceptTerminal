#include "rf1k/platform/board_hw.hpp"

#include <cmath>

#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
#include "main.h"
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern SPI_HandleTypeDef hspi4;
#endif
#endif

namespace rf1k {

BoardHw::BoardHw(BoardCal cal) : cal_(cal) {}

void BoardHw::init() {
  pos_[0] = 512;
  pos_[1] = 512;
  bus_cmd_v_ = 0.f;
  amp_cmd_ = 0.f;
  rf_on_ = false;

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
  static uint16_t dma[5];
  HAL_ADC_Start_DMA(&hadc1, reinterpret_cast<uint32_t*>(dma), 5);
  // Note: wire HAL_ADC_ConvCpltCallback to copy dma[] into adc_* fields.
  HAL_GPIO_WritePin(DDS_RESET_GPIO_Port, DDS_RESET_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(DDS_RESET_GPIO_Port, DDS_RESET_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(EN_STEPPER_GPIO_Port, EN_STEPPER_Pin, GPIO_PIN_RESET);
  rfEnable(false);
  writeDacBus(0.f);
#else
  // Mock: mid-scale detectors ≈ low power until app injects ADC
  setAdcRaw(8000, 500, 10000, 20000);
#endif
}

void BoardHw::setAdcRaw(std::uint16_t pf, std::uint16_t pr, std::uint16_t vbus,
                        std::uint16_t temp_pa) {
  adc_pf_ = pf;
  adc_pr_ = pr;
  adc_vbus_ = vbus;
  adc_temp_ = temp_pa;
}

float BoardHw::adcToVolt(std::uint16_t raw) const {
  return 3.3f * static_cast<float>(raw) / 65535.f;
}

float BoardHw::detToWatts(float vdet, float v0, float k) const {
  if (k <= 1e-6f) return 0.f;
  const float db_like = (vdet - v0) / k;
  const float mw = std::pow(10.f, db_like / 10.f);
  return mw / 1000.f;
}

float BoardHw::readPfW() {
  return detToWatts(adcToVolt(adc_pf_), cal_.det.pf_v0, cal_.det.pf_k);
}

float BoardHw::readPrW() {
  return detToWatts(adcToVolt(adc_pr_), cal_.det.pr_v0, cal_.det.pr_k);
}

float BoardHw::readVbusV() {
  return (static_cast<float>(adc_vbus_) / 65535.f) * cal_.sense.vbus_fs_v;
}

float BoardHw::readTempPaC() {
  const float frac = static_cast<float>(adc_temp_) / 65535.f;
  return cal_.sense.temp_offset_c + frac * cal_.sense.temp_gain_c;
}

bool BoardHw::interlockOk() {
#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  return HAL_GPIO_ReadPin(INTERLOCK_GPIO_Port, INTERLOCK_Pin) == GPIO_PIN_SET;
#else
  return mock_interlock_ok_;
#endif
}

bool BoardHw::faultLatched() {
#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  return HAL_GPIO_ReadPin(FAULT_LATCH_GPIO_Port, FAULT_LATCH_Pin) ==
         GPIO_PIN_SET;
#else
  return mock_fault_latched_;
#endif
}

void BoardHw::writeDacBus(float volts) {
  float vmax = cal_.sense.vbus_fs_v;
  if (cal_.bringup.max_vbus_v > 0.f && cal_.bringup.max_vbus_v < vmax) {
    // Always respect bring-up bus ceiling when configured on this BoardCal.
    vmax = cal_.bringup.max_vbus_v;
  }
  if (volts < 0.f) volts = 0.f;
  if (volts > vmax) volts = vmax;
  bus_cmd_v_ = volts;
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
  const uint32_t code =
      static_cast<uint32_t>((volts / cal_.sense.vbus_fs_v) * 4095.f + 0.5f);
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, code);
  HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
#elif defined(RF_STM32_MOCK)
  (void)volts;
#endif
}

void BoardHw::setBusVoltage(float volts) { writeDacBus(volts); }

void BoardHw::writeDdsAmplitude(float norm) {
  if (norm < 0.f) norm = 0.f;
  if (norm > 1.f) norm = 1.f;
  amp_cmd_ = norm;
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
  // Prefer external VGA via DAC; optional AD9951 ASF write goes here.
  (void)norm;
#endif
}

void BoardHw::setDdsAmplitude(float norm_0_1) { writeDdsAmplitude(norm_0_1); }

void BoardHw::rfEnable(bool on) {
  rf_on_ = on;
#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  HAL_GPIO_WritePin(RF_ENABLE_GPIO_Port, RF_ENABLE_Pin,
                    on ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
}

void BoardHw::faultClear() {
#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  HAL_GPIO_WritePin(FAULT_CLR_GPIO_Port, FAULT_CLR_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(FAULT_CLR_GPIO_Port, FAULT_CLR_Pin, GPIO_PIN_RESET);
#else
  mock_fault_latched_ = false;
#endif
}

void BoardHw::pulseStep(int axis) {
#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  auto* port = (axis == 0) ? STEP_CS_GPIO_Port : STEP_CP_GPIO_Port;
  auto pin = (axis == 0) ? STEP_CS_Pin : STEP_CP_Pin;
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
  for (volatile int i = 0; i < 40; ++i) {
  }
  HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
  for (volatile int i = 0; i < 40; ++i) {
  }
#else
  (void)axis;
#endif
}

void BoardHw::stepperMove(int axis, int delta_steps) {
  if (axis < 0 || axis > 1) return;
  const int dir = delta_steps >= 0 ? 1 : 0;
  int n = delta_steps >= 0 ? delta_steps : -delta_steps;
#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  if (axis == 0) {
    HAL_GPIO_WritePin(DIR_CS_GPIO_Port, DIR_CS_Pin,
                      dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(DIR_CP_GPIO_Port, DIR_CP_Pin,
                      dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
#endif
  for (int i = 0; i < n; ++i) pulseStep(axis);
  pos_[axis] += delta_steps;
}

int BoardHw::stepperPos(int axis) {
  return (axis >= 0 && axis < 2) ? pos_[axis] : 0;
}

match::PowerSample BoardHw::readPower() {
  return {readPfW(), readPrW()};
}

void BoardHw::move(match::Axis axis, int delta_steps) {
  stepperMove(static_cast<int>(axis), delta_steps);
}

void BoardHw::delayMs(std::uint32_t ms) {
#if defined(RF_USE_FREERTOS)
  vTaskDelay(pdMS_TO_TICKS(ms));
#elif defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  HAL_Delay(ms);
#else
  (void)ms;
#endif
}

bool BoardHw::hitLimit(match::Axis axis, int) {
#if defined(RF_TARGET_STM32) || defined(RF_STM32_MOCK)
  if (axis == match::Axis::Cs) {
    return HAL_GPIO_ReadPin(LIMIT_CS_GPIO_Port, LIMIT_CS_Pin) == GPIO_PIN_SET;
  }
  return HAL_GPIO_ReadPin(LIMIT_CP_GPIO_Port, LIMIT_CP_Pin) == GPIO_PIN_SET;
#else
  const int p = pos_[static_cast<int>(axis)];
  return p < 0 || p > 1023;
#endif
}

}  // namespace rf1k

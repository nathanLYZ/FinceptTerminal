/**
 * Example BoardActuator for STM32 + FreeRTOS.
 * Compile only on target (RF_USE_FREERTOS + HAL). Kept as reference — not
 * linked into the host demo.
 */
#if defined(RF_USE_FREERTOS) && defined(RF_TARGET_STM32)

#include "match/types.hpp"

// #include "main.h"          // CubeMX pins: STEP_CS, DIR_CS, ...
// extern ADC_HandleTypeDef hadc1;

namespace rf::match {

class BoardActuator final : public IActuator {
 public:
  PowerSample readPower() override {
    // Convert detector ADC → watts with calibration table.
    // return PowerSample{pf, pr};
    return PowerSample{0.f, 0.f};
  }

  void move(Axis axis, int delta_steps) override {
    // STEP/DIR pulse train for TMC2209; update encoder/pot if available.
    (void)axis;
    (void)delta_steps;
  }

  void delayMs(std::uint32_t ms) override {
    vTaskDelay(pdMS_TO_TICKS(ms));  // allow other tasks while motor settles
  }

  bool hitLimit(Axis axis, int direction) override {
    (void)axis;
    (void)direction;
    // return HAL_GPIO_ReadPin(LIM_x) == ...
    return false;
  }
};

/*
 * void startMatcherTask() {
 *   static BoardActuator hw;
 *   static Search search(hw, Config{});
 *   search.setEnable(true);
 *   MatchTask::start(search, 20, "Match", 3, 512);
 * }
 */

}  // namespace rf::match

#endif

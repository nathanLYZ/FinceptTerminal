#pragma once

#include "rf1k/system.hpp"

#if defined(RF_USE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#else
#include <cstdint>
inline void vTaskDelay(std::uint32_t) {}
#ifndef pdMS_TO_TICKS
#define pdMS_TO_TICKS(ms) (ms)
#endif
using TaskHandle_t = void*;
using TaskFunction_t = void (*)(void*);
inline int xTaskCreate(TaskFunction_t, const char*, unsigned, void*, unsigned,
                       TaskHandle_t*) {
  return 1;
}
#endif

namespace rf1k {

/** Spawn Fault/Power/Match/Comm tasks around a System instance. */
class AppTasks {
 public:
  static void start(System& sys) {
    sys_ = &sys;
#if defined(RF_USE_FREERTOS)
    xTaskCreate(&AppTasks::powerEntry, "Power", 512, &sys, 4, nullptr);
    xTaskCreate(&AppTasks::matchEntry, "Match", 512, &sys, 3, nullptr);
    xTaskCreate(&AppTasks::commEntry, "Comm", 512, &sys, 3, nullptr);
#else
    (void)sys;
#endif
  }

  /** Host cooperative loop for N milliseconds of simulated time. */
  static void runHost(System& sys, int duration_ms) {
    for (int t = 0; t < duration_ms; ++t) {
      sys.tickPower(0.001f);
      if (t % 20 == 0) sys.tickMatch();
      if (t % 10 == 0) sys.tickComm();
    }
  }

 private:
#if defined(RF_USE_FREERTOS)
  static void powerEntry(void* p) {
    auto* s = static_cast<System*>(p);
    TickType_t last = xTaskGetTickCount();
    for (;;) {
      s->tickPower(0.001f);
      vTaskDelayUntil(&last, pdMS_TO_TICKS(1));
    }
  }
  static void matchEntry(void* p) {
    auto* s = static_cast<System*>(p);
    TickType_t last = xTaskGetTickCount();
    for (;;) {
      s->tickMatch();
      vTaskDelayUntil(&last, pdMS_TO_TICKS(20));
    }
  }
  static void commEntry(void* p) {
    auto* s = static_cast<System*>(p);
    TickType_t last = xTaskGetTickCount();
    for (;;) {
      s->tickComm();
      vTaskDelayUntil(&last, pdMS_TO_TICKS(10));
    }
  }
#endif
  static System* sys_;
};

}  // namespace rf1k

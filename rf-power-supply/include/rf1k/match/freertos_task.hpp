#pragma once

#include "rf1k/match/search.hpp"

#include <cstdint>

#if defined(RF_USE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#else
#include <cstdint>
using TaskHandle_t = void*;
using TaskFunction_t = void (*)(void*);
inline void vTaskDelay(std::uint32_t) {}
#ifndef pdMS_TO_TICKS
#define pdMS_TO_TICKS(ms) (ms)
#endif
inline int xTaskCreate(TaskFunction_t, const char*, unsigned, void*, unsigned,
                       TaskHandle_t*) {
  return 1;
}
#endif

namespace rf1k::match {

/**
 * FreeRTOS task that periodically calls Search::tick().
 *
 * Board code:
 *   rf::match::MatchTask::start(search, cfg.period_ms, "Match", 3, 512);
 */
class MatchTask {
 public:
  struct Args {
    Search* search{nullptr};
    std::uint32_t period_ms{20};
  };

  static bool start(Search& search, std::uint32_t period_ms = 20,
                    const char* name = "Match", unsigned priority = 3,
                    unsigned stack_words = 512) {
    args_.search = &search;
    args_.period_ms = period_ms;
    return xTaskCreate(&MatchTask::entry, name, stack_words, &args_, priority,
                       &handle_) == 1;
  }

  /** No RTOS: run the same tick loop for bring-up / unit tests. */
  static void runBlocking(Search& search, int max_iters) {
    for (int i = 0; i < max_iters; ++i) {
      search.tick();
      if (search.state() == State::Track || search.state() == State::Fault) {
        break;
      }
    }
  }

 private:
  static void entry(void* param) {
    auto* args = static_cast<Args*>(param);
#if defined(RF_USE_FREERTOS)
    TickType_t last = xTaskGetTickCount();
    const TickType_t period = pdMS_TO_TICKS(args->period_ms);
    for (;;) {
      args->search->tick();
      vTaskDelayUntil(&last, period);
    }
#else
    (void)args;
    for (;;) {
      vTaskDelay(pdMS_TO_TICKS(20));
    }
#endif
  }

  static Args args_;
  static TaskHandle_t handle_;
};

}  // namespace rf1k::match

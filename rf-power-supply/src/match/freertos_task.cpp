#include "rf1k/match/freertos_task.hpp"

namespace rf1k::match {

MatchTask::Args MatchTask::args_{};
TaskHandle_t MatchTask::handle_{nullptr};

}  // namespace rf1k::match

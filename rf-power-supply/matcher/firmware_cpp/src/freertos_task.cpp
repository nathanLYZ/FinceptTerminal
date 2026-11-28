#include "match/freertos_task.hpp"

namespace rf::match {

MatchTask::Args MatchTask::args_{};
TaskHandle_t MatchTask::handle_{nullptr};

}  // namespace rf::match

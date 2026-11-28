cmake_minimum_required(VERSION 3.16)
project(rf1k LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rf1k_core
  src/pid.cpp
  src/metrics.cpp
  src/modbus.cpp
  src/system.cpp
  src/match/search.cpp
  src/match/freertos_task.cpp
  src/platform/freertos_app.cpp
)
target_include_directories(rf1k_core PUBLIC include src src/platform src/match)

add_executable(rf1k_sim src/apps/rf1k_sim.cpp)
target_link_libraries(rf1k_sim PRIVATE rf1k_core)

add_executable(rf1k_cli src/apps/rf1k_cli.cpp)
target_link_libraries(rf1k_cli PRIVATE rf1k_core)

add_executable(test_modbus src/apps/test_modbus.cpp src/modbus.cpp)
target_include_directories(test_modbus PRIVATE include)

add_executable(test_unit src/apps/test_unit.cpp)
target_link_libraries(test_unit PRIVATE rf1k_core)

add_executable(rf1k_bringup50 src/apps/rf1k_bringup50.cpp)
target_link_libraries(rf1k_bringup50 PRIVATE rf1k_core)

add_executable(board_check
  src/apps/board_check.cpp
  src/platform/board_hw.cpp
  src/platform/mock_hal/stm32h7xx_hal.cpp
)
target_link_libraries(board_check PRIVATE rf1k_core)
target_compile_definitions(board_check PRIVATE RF_STM32_MOCK)
target_include_directories(board_check PRIVATE src/platform/mock_hal)
target_link_libraries(board_check PRIVATE m)

enable_testing()
add_test(NAME test_modbus COMMAND test_modbus)
add_test(NAME test_unit COMMAND test_unit)
add_test(NAME rf1k_bringup50 COMMAND rf1k_bringup50)
add_test(NAME board_check COMMAND board_check)
add_test(NAME rf1k_sim COMMAND rf1k_sim)

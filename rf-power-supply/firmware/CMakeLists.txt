cmake_minimum_required(VERSION 3.16)
project(rf1k_sim C)

set(RF1K_SOURCES
  app/main.c
  app/state_machine.c
  app/power_loop.c
  app/match_loop.c
  app/fault_manager.c
  app/pid.c
  app/tasks/rf_tasks.c
  services/modbus_regs.c
  platform/freertos_shim.c
  drivers/hw_stub.c
)

add_executable(rf_sim ${RF1K_SOURCES})
target_include_directories(rf_sim PRIVATE inc platform app/tasks)
target_link_libraries(rf_sim m)
target_compile_options(rf_sim PRIVATE -Wall -Wextra -std=c11)

add_executable(test_modbus tests/test_modbus.c services/modbus_regs.c)
target_include_directories(test_modbus PRIVATE inc)
target_compile_options(test_modbus PRIVATE -Wall -Wextra -std=c11)

enable_testing()
add_test(NAME modbus COMMAND test_modbus)
add_test(NAME sim_run COMMAND rf_sim)

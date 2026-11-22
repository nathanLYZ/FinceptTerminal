cmake_minimum_required(VERSION 3.16)
project(rf1k_sim C)

add_executable(rf_sim
  app/main.c
  app/state_machine.c
  app/power_loop.c
  app/match_loop.c
  app/fault_manager.c
  app/pid.c
  drivers/hw_stub.c
)
target_include_directories(rf_sim PRIVATE inc)
target_link_libraries(rf_sim m)
target_compile_options(rf_sim PRIVATE -Wall -Wextra -std=c11)

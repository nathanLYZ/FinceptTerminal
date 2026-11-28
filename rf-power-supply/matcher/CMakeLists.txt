cmake_minimum_required(VERSION 3.16)
project(rf_matcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(rf_match
  src/search.cpp
  src/freertos_task.cpp
)
target_include_directories(rf_match PUBLIC include src)

add_executable(match_demo src/apps/match_demo.cpp)
target_link_libraries(match_demo PRIVATE rf_match)
target_include_directories(match_demo PRIVATE src)

add_executable(match_cli src/apps/match_cli.cpp)
target_link_libraries(match_cli PRIVATE rf_match)
target_include_directories(match_cli PRIVATE src)

enable_testing()
add_test(NAME match_demo COMMAND match_demo)
add_test(NAME match_cli COMMAND match_cli --cs 200 --cp 800)

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -Isrc -Isrc/platform -Isrc/match
LIB_SRCS = \
  src/pid.cpp \
  src/metrics.cpp \
  src/modbus.cpp \
  src/system.cpp \
  src/match/search.cpp \
  src/match/freertos_task.cpp \
  src/platform/freertos_app.cpp

BOARD_FLAGS = -DRF_STM32_MOCK -Isrc/platform/mock_hal
BOARD_SRCS = src/platform/board_hw.cpp src/platform/mock_hal/stm32h7xx_hal.cpp

SIM = build/rf1k_sim
CLI = build/rf1k_cli
TEST_MB = build/test_modbus
TEST_UNIT = build/test_unit
BRINGUP50 = build/rf1k_bringup50
BOARD_CHECK = build/board_check

.PHONY: all test board-check bringup50 clean

all: $(SIM) $(CLI) $(TEST_MB) $(TEST_UNIT) $(BRINGUP50) $(BOARD_CHECK)

$(SIM): $(LIB_SRCS) src/apps/rf1k_sim.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/rf1k_sim.cpp -o $(SIM)

$(CLI): $(LIB_SRCS) src/apps/rf1k_cli.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/rf1k_cli.cpp -o $(CLI)

$(TEST_MB): src/modbus.cpp src/apps/test_modbus.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) src/modbus.cpp src/apps/test_modbus.cpp -o $(TEST_MB)

$(TEST_UNIT): $(LIB_SRCS) src/apps/test_unit.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/test_unit.cpp -o $(TEST_UNIT)

$(BRINGUP50): $(LIB_SRCS) src/apps/rf1k_bringup50.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/rf1k_bringup50.cpp -o $(BRINGUP50)

$(BOARD_CHECK): $(LIB_SRCS) $(BOARD_SRCS) src/apps/board_check.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(BOARD_FLAGS) $(LIB_SRCS) $(BOARD_SRCS) \
	  src/apps/board_check.cpp -o $(BOARD_CHECK) -lm

board-check: $(BOARD_CHECK)
	./$(BOARD_CHECK)

bringup50: $(BRINGUP50)
	./$(BRINGUP50)

test: all
	./$(TEST_MB)
	./$(TEST_UNIT)
	./$(BRINGUP50)
	./$(BOARD_CHECK)
	./$(SIM)

clean:
	rm -rf build

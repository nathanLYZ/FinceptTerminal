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

SIM = build/rf1k_sim
CLI = build/rf1k_cli
TEST_MB = build/test_modbus
TEST_UNIT = build/test_unit

.PHONY: all test clean

all: $(SIM) $(CLI) $(TEST_MB) $(TEST_UNIT)

$(SIM): $(LIB_SRCS) src/apps/rf1k_sim.cpp include/rf1k/*.hpp include/rf1k/match/*.hpp src/platform/stub_plant.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/rf1k_sim.cpp -o $(SIM)

$(CLI): $(LIB_SRCS) src/apps/rf1k_cli.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/rf1k_cli.cpp -o $(CLI)

$(TEST_MB): src/modbus.cpp src/apps/test_modbus.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) src/modbus.cpp src/apps/test_modbus.cpp -o $(TEST_MB)

$(TEST_UNIT): $(LIB_SRCS) src/apps/test_unit.cpp src/platform/stub_plant.hpp src/match/stub_actuator.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/test_unit.cpp -o $(TEST_UNIT)

test: all
	./$(TEST_MB)
	./$(TEST_UNIT)
	./$(SIM)

clean:
	rm -rf build

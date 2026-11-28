CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
SRCS = src/search.cpp src/freertos_task.cpp src/main.cpp
OUT = build/match_cpp_demo

.PHONY: all test clean

all: $(OUT)

$(OUT): $(SRCS) include/match/*.hpp src/stub_actuator.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(OUT)

test: $(OUT)
	./$(OUT)

clean:
	rm -rf build

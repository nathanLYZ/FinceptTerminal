CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -Isrc
LIB_SRCS = src/search.cpp src/freertos_task.cpp
DEMO = build/match_demo
CLI = build/match_cli

.PHONY: all test cli clean

all: $(DEMO) $(CLI)

$(DEMO): $(LIB_SRCS) src/apps/match_demo.cpp include/match/*.hpp src/stub_actuator.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/match_demo.cpp -o $(DEMO)

$(CLI): $(LIB_SRCS) src/apps/match_cli.cpp include/match/*.hpp src/stub_actuator.hpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(LIB_SRCS) src/apps/match_cli.cpp -o $(CLI)

test: $(DEMO) $(CLI)
	./$(DEMO)
	./$(CLI) --cs 180 --cp 820

cli: $(CLI)
	./$(CLI)

clean:
	rm -rf build

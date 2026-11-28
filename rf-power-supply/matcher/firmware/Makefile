CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2
SRCS = match_metrics.c match_search.c hw_match_stub.c match_task.c
OUT = build/match_demo

.PHONY: all test clean

all: $(OUT)

$(OUT): $(SRCS) match_types.h match_metrics.h match_search.h
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT) -lm

test: $(OUT)
	./$(OUT)

clean:
	rm -rf build

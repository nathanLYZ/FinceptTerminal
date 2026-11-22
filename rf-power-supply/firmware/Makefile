CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinc
SRCS = app/main.c app/state_machine.c app/power_loop.c app/match_loop.c \
       app/fault_manager.c app/pid.c drivers/hw_stub.c
OUT = build/rf_sim

.PHONY: all clean run

all: $(OUT)

$(OUT): $(SRCS) inc/rf_types.h inc/rf_api.h
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(OUT) -lm

run: $(OUT)
	./$(OUT)

clean:
	rm -rf build

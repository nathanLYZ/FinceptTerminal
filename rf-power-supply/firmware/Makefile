CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinc -Iplatform -Iapp/tasks
SRCS_CORE = app/state_machine.c app/power_loop.c app/match_loop.c \
            app/fault_manager.c app/pid.c app/tasks/rf_tasks.c \
            services/modbus_regs.c platform/freertos_shim.c \
            drivers/hw_stub.c
SRCS_APP = app/main.c $(SRCS_CORE)
OUT = build/rf_sim
TEST_OUT = build/test_modbus

.PHONY: all clean run test

all: $(OUT)

$(OUT): $(SRCS_APP) inc/rf_types.h inc/rf_api.h inc/modbus_regs.h \
        platform/freertos_shim.h app/tasks/rf_tasks.h
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS_APP) -o $(OUT) -lm

$(TEST_OUT): tests/test_modbus.c services/modbus_regs.c inc/modbus_regs.h inc/rf_types.h
	mkdir -p build
	$(CC) $(CFLAGS) tests/test_modbus.c services/modbus_regs.c -o $(TEST_OUT)

run: $(OUT)
	./$(OUT)

test: $(TEST_OUT) $(OUT)
	./$(TEST_OUT)
	./$(OUT)

clean:
	rm -rf build

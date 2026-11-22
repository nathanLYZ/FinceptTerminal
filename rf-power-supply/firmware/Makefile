CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -O2 -Iinc -Iplatform -Iapp/tasks
SRCS_CORE = app/state_machine.c app/power_loop.c app/match_loop.c \
            app/fault_manager.c app/pid.c app/tasks/rf_tasks.c \
            services/modbus_regs.c services/modbus_rtu.c services/board_cal.c \
            platform/freertos_shim.c
SRCS_HOST = app/main.c $(SRCS_CORE) drivers/hw_stub.c
OUT = build/rf_sim
TEST_OUT = build/test_modbus
STM32_OUT = build/stm32_check

STM32_INC = -Itarget/stm32/mock_hal -Itarget/stm32/Drivers/RF1K -Iinc -Iapp/tasks
STM32_FLAGS = -DRF_TARGET_STM32 -DRF_STM32_MOCK $(STM32_INC)
STM32_SRCS = \
  target/stm32/Core/Src/main_stm32.c \
  target/stm32/Drivers/RF1K/hw_stm32.c \
  target/stm32/Drivers/RF1K/dds_ad9951.c \
  target/stm32/Drivers/RF1K/stepper_tmc.c \
  target/stm32/Drivers/RF1K/modbus_uart.c \
  target/stm32/Drivers/RF1K/board_gpio_adc.c \
  target/stm32/mock_hal/stm32h7xx_hal.c \
  app/state_machine.c app/power_loop.c app/match_loop.c \
  app/fault_manager.c app/pid.c app/tasks/rf_tasks.c \
  services/modbus_regs.c services/modbus_rtu.c services/board_cal.c

.PHONY: all clean run test stm32-check

all: $(OUT)

$(OUT): $(SRCS_HOST) inc/rf_types.h inc/rf_api.h inc/modbus_regs.h \
        platform/freertos_shim.h app/tasks/rf_tasks.h
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS_HOST) -o $(OUT) -lm

TEST_RTU_OUT = build/test_modbus_rtu

$(TEST_OUT): tests/test_modbus.c services/modbus_regs.c \
             inc/modbus_regs.h inc/rf_types.h
	mkdir -p build
	$(CC) $(CFLAGS) tests/test_modbus.c services/modbus_regs.c -o $(TEST_OUT)

$(TEST_RTU_OUT): tests/test_modbus_rtu.c services/modbus_regs.c services/modbus_rtu.c \
                 services/board_cal.c
	mkdir -p build
	$(CC) $(CFLAGS) tests/test_modbus_rtu.c services/modbus_regs.c services/modbus_rtu.c \
	  services/board_cal.c -o $(TEST_RTU_OUT)

$(STM32_OUT): $(STM32_SRCS)
	mkdir -p build
	$(CC) -std=c11 -Wall -Wextra -O2 $(STM32_FLAGS) $(STM32_SRCS) -o $(STM32_OUT) -lm

run: $(OUT)
	./$(OUT)

stm32-check: $(STM32_OUT)
	./$(STM32_OUT)

test: $(TEST_OUT) $(TEST_RTU_OUT) $(OUT) stm32-check
	./$(TEST_OUT)
	./$(TEST_RTU_OUT)
	./$(OUT)

clean:
	rm -rf build

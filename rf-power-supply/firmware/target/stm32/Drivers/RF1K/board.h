#ifndef RF1K_BOARD_H
#define RF1K_BOARD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef RF_TARGET_STM32
#include "main.h" /* CubeMX: pin macros */
#endif

void board_init(void);
void board_adc_start(void);
void board_modbus_uart_init(void);

/* Filled by DMA / ISR */
typedef struct {
    volatile uint16_t raw_pf;
    volatile uint16_t raw_pr;
    volatile uint16_t raw_vbus;
    volatile uint16_t raw_idc;
    volatile uint16_t raw_temp_pa;
    volatile uint16_t raw_cs;
    volatile uint16_t raw_cp;
} board_adc_raw_t;

extern board_adc_raw_t g_adc_raw;

/* Modbus UART RX complete frame callback (ISR / idle-line) */
void board_modbus_on_frame(const uint8_t *data, uint16_t len);
int board_modbus_pop_request(uint8_t *buf, int cap);
void board_modbus_send(const uint8_t *data, uint16_t len);

void board_dds_init(void);
void board_dds_set_freq_hz(uint32_t hz);
void board_dds_set_amplitude(float norm_0_1);

void board_stepper_init(void);
void board_stepper_move(int axis, int delta_steps);
int32_t board_stepper_pos(int axis);

void board_set_vbus_dac(float volts);
void board_rf_enable(bool on);
bool board_interlock_ok(void);
bool board_fault_latched(void);
void board_fault_clear(void);

#endif /* RF1K_BOARD_H */

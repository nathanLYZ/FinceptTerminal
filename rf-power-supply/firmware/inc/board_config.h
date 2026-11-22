#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>

#define RF1K_F0_HZ            13560000u
#define RF1K_P_MAX_W          1000u
#define RF1K_VBUS_MAX_V       60.0f
#define RF1K_MODBUS_SLAVE     1u
#define RF1K_MODBUS_BAUD      115200u
#define RF1K_UART_RX_BUF      256u
#define RF1K_MB_FRAME_MAX     256u

/* AD8307 calibration: P_w = 10^((Vdet - V0)/k) / 1000
   Defaults — replace after factory cal (stored in Flash). */
typedef struct {
    float pf_v0;
    float pf_k;
    float pr_v0;
    float pr_k;
} rf_det_cal_t;

typedef struct {
    float vbus_gain;   /* volts per ADC count fraction */
    float temp_pa_offset_c;
    float temp_pa_gain;
} rf_sense_cal_t;

extern rf_det_cal_t g_det_cal;
extern rf_sense_cal_t g_sense_cal;

void board_cal_load_defaults(void);

#endif /* BOARD_CONFIG_H */

#ifndef MODBUS_REGS_H
#define MODBUS_REGS_H

#include <stdint.h>

#include "rf_types.h"

/* Holding registers (RW) */
enum {
    MB_H_CMD_WORD = 0x0000,
    MB_H_POWER_SET_W = 0x0001,
    MB_H_RAMP_W_S = 0x0002,
    MB_H_PREFL_TRIP_W = 0x0003,
    MB_H_CS_PRESET = 0x0004,
    MB_H_CP_PRESET = 0x0005,
    MB_H_COUNT = 6
};

/* Input registers (RO) — protocol docs use 0x0100+, stored densely here */
enum {
    MB_I_STATE = 0,
    MB_I_PF_W = 1,
    MB_I_PR_W = 2,
    MB_I_VSWR_X100 = 3,
    MB_I_VBUS_MV = 4,
    MB_I_TEMP_PA_X10 = 5,
    MB_I_FAULT_LATCH = 6,
    MB_I_CS_POS = 7,
    MB_I_CP_POS = 8,
    MB_I_COUNT = 9
};

enum {
    MB_CMD_RF_ON = 1u << 0,
    MB_CMD_CLEAR_FAULT = 1u << 1,
    MB_CMD_MATCH_AUTO = 1u << 2
};

typedef struct {
    uint16_t holding[MB_H_COUNT];
    uint16_t input[MB_I_COUNT];
} modbus_map_t;

void modbus_map_init(modbus_map_t *map);
void modbus_export_telemetry(modbus_map_t *map, const rf_system_t *sys);
void modbus_apply_holding(modbus_map_t *map, rf_system_t *sys);

/* Absolute protocol address → dense input index; returns -1 if unknown */
int modbus_input_index(uint16_t abs_addr);

/**
 * Handle one Modbus RTU request frame (without CRC).
 * Writes response body into resp, returns response length excluding CRC, or <0.
 */
int modbus_handle_pdu(modbus_map_t *map, const uint8_t *req, int req_len,
                      uint8_t *resp, int resp_max);

uint16_t modbus_crc16(const uint8_t *data, int len);

#endif /* MODBUS_REGS_H */

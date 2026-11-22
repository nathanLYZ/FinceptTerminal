#include "modbus_regs.h"

#include <string.h>

void modbus_map_init(modbus_map_t *map)
{
    memset(map, 0, sizeof(*map));
    map->holding[MB_H_RAMP_W_S] = 200;
    map->holding[MB_H_PREFL_TRIP_W] = 200;
    map->holding[MB_H_CMD_WORD] = MB_CMD_MATCH_AUTO;
}

int modbus_input_index(uint16_t abs_addr)
{
    if (abs_addr < 0x0100u || abs_addr > 0x0108u) {
        return -1;
    }
    return (int)(abs_addr - 0x0100u);
}

void modbus_export_telemetry(modbus_map_t *map, const rf_system_t *sys)
{
    map->input[MB_I_STATE] = (uint16_t)sys->state;
    map->input[MB_I_PF_W] = (uint16_t)(sys->tm.pf_w < 0 ? 0 : sys->tm.pf_w + 0.5f);
    map->input[MB_I_PR_W] = (uint16_t)(sys->tm.pr_w < 0 ? 0 : sys->tm.pr_w + 0.5f);
    map->input[MB_I_VSWR_X100] = (uint16_t)(sys->tm.vswr * 100.0f + 0.5f);
    map->input[MB_I_VBUS_MV] = (uint16_t)(sys->tm.vbus_v * 1000.0f + 0.5f);
    map->input[MB_I_TEMP_PA_X10] = (uint16_t)(sys->tm.temp_pa_c * 10.0f + 0.5f);
    map->input[MB_I_FAULT_LATCH] = (uint16_t)sys->fault_latch;
    map->input[MB_I_CS_POS] = (uint16_t)sys->tm.cs_pos;
    map->input[MB_I_CP_POS] = (uint16_t)sys->tm.cp_pos;
}

void modbus_apply_holding(modbus_map_t *map, rf_system_t *sys)
{
    uint16_t cmd = map->holding[MB_H_CMD_WORD];
    sys->sp.rf_on_cmd = (cmd & MB_CMD_RF_ON) != 0;
    sys->sp.clear_fault = (cmd & MB_CMD_CLEAR_FAULT) != 0;
    sys->sp.match_auto = (cmd & MB_CMD_MATCH_AUTO) != 0;
    sys->sp.power_set_w = (float)map->holding[MB_H_POWER_SET_W];
    sys->sp.ramp_w_s = (float)map->holding[MB_H_RAMP_W_S];
    sys->sp.prefl_trip_w = (float)map->holding[MB_H_PREFL_TRIP_W];
    sys->sp.cs_preset = (int32_t)map->holding[MB_H_CS_PRESET];
    sys->sp.cp_preset = (int32_t)map->holding[MB_H_CP_PRESET];

    /* CLEAR_FAULT is one-shot from master perspective */
    if (cmd & MB_CMD_CLEAR_FAULT) {
        map->holding[MB_H_CMD_WORD] = (uint16_t)(cmd & ~MB_CMD_CLEAR_FAULT);
    }
}

uint16_t modbus_crc16(const uint8_t *data, int len)
{
    uint16_t crc = 0xFFFFu;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            if (crc & 1u) {
                crc = (uint16_t)((crc >> 1) ^ 0xA001u);
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static void put_u16_be(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffu);
}

int modbus_handle_pdu(modbus_map_t *map, const uint8_t *req, int req_len,
                      uint8_t *resp, int resp_max)
{
    if (req_len < 2 || resp_max < 5) {
        return -1;
    }
    uint8_t addr = req[0];
    uint8_t fn = req[1];
    resp[0] = addr;
    resp[1] = fn;

    if (fn == 0x03) { /* Read Holding */
        if (req_len < 6) {
            return -1;
        }
        uint16_t start = (uint16_t)((req[2] << 8) | req[3]);
        uint16_t qty = (uint16_t)((req[4] << 8) | req[5]);
        if (start + qty > MB_H_COUNT || qty == 0) {
            resp[1] = (uint8_t)(fn | 0x80u);
            resp[2] = 0x02;
            return 3;
        }
        resp[2] = (uint8_t)(qty * 2u);
        for (uint16_t i = 0; i < qty; ++i) {
            put_u16_be(&resp[3 + i * 2], map->holding[start + i]);
        }
        return 3 + qty * 2;
    }

    if (fn == 0x04) { /* Read Input */
        if (req_len < 6) {
            return -1;
        }
        uint16_t start = (uint16_t)((req[2] << 8) | req[3]);
        uint16_t qty = (uint16_t)((req[4] << 8) | req[5]);
        int idx0 = modbus_input_index(start);
        if (idx0 < 0 || qty == 0 || (idx0 + qty) > MB_I_COUNT) {
            resp[1] = (uint8_t)(fn | 0x80u);
            resp[2] = 0x02;
            return 3;
        }
        resp[2] = (uint8_t)(qty * 2u);
        for (uint16_t i = 0; i < qty; ++i) {
            put_u16_be(&resp[3 + i * 2], map->input[idx0 + i]);
        }
        return 3 + qty * 2;
    }

    if (fn == 0x06) { /* Write Single Holding */
        if (req_len < 6) {
            return -1;
        }
        uint16_t reg = (uint16_t)((req[2] << 8) | req[3]);
        uint16_t val = (uint16_t)((req[4] << 8) | req[5]);
        if (reg >= MB_H_COUNT) {
            resp[1] = (uint8_t)(fn | 0x80u);
            resp[2] = 0x02;
            return 3;
        }
        map->holding[reg] = val;
        memcpy(resp, req, 6);
        return 6;
    }

    if (fn == 0x10) { /* Write Multiple Holding */
        if (req_len < 7) {
            return -1;
        }
        uint16_t start = (uint16_t)((req[2] << 8) | req[3]);
        uint16_t qty = (uint16_t)((req[4] << 8) | req[5]);
        uint8_t bc = req[6];
        if (bc != qty * 2 || req_len < 7 + bc || start + qty > MB_H_COUNT) {
            resp[1] = (uint8_t)(fn | 0x80u);
            resp[2] = 0x02;
            return 3;
        }
        for (uint16_t i = 0; i < qty; ++i) {
            map->holding[start + i] =
                (uint16_t)((req[7 + i * 2] << 8) | req[8 + i * 2]);
        }
        resp[2] = req[2];
        resp[3] = req[3];
        resp[4] = req[4];
        resp[5] = req[5];
        return 6;
    }

    resp[1] = (uint8_t)(fn | 0x80u);
    resp[2] = 0x01; /* illegal function */
    return 3;
}

#include <stdio.h>
#include <string.h>

#include "modbus_regs.h"
#include "rf_types.h"

static int g_fail;

static void expect_eq_u16(const char *name, uint16_t got, uint16_t want)
{
    if (got != want) {
        printf("FAIL %s: got %u want %u\n", name, got, want);
        g_fail++;
    } else {
        printf("ok   %s\n", name);
    }
}

static void expect_true(const char *name, int cond)
{
    if (!cond) {
        printf("FAIL %s\n", name);
        g_fail++;
    } else {
        printf("ok   %s\n", name);
    }
}

int main(void)
{
    modbus_map_t map;
    rf_system_t sys;
    memset(&sys, 0, sizeof(sys));
    modbus_map_init(&map);

    /* Write power + RF on via FC06 */
    uint8_t req[8];
    uint8_t resp[64];
    req[0] = 1;
    req[1] = 0x06;
    req[2] = 0x00;
    req[3] = 0x01; /* POWER_SET */
    req[4] = 0x01;
    req[5] = 0xF4; /* 500 */
    int n = modbus_handle_pdu(&map, req, 6, resp, (int)sizeof(resp));
    expect_true("fc06 len", n == 6);
    expect_eq_u16("power holding", map.holding[MB_H_POWER_SET_W], 500);

    map.holding[MB_H_CMD_WORD] = MB_CMD_RF_ON | MB_CMD_MATCH_AUTO;
    modbus_apply_holding(&map, &sys);
    expect_true("rf on applied", sys.sp.rf_on_cmd);
    expect_eq_u16("setpoint", (uint16_t)sys.sp.power_set_w, 500);

    sys.state = RF_STATE_RUN;
    sys.tm.pf_w = 498.2f;
    sys.tm.pr_w = 12.1f;
    sys.tm.vswr = 1.37f;
    sys.tm.vbus_v = 31.5f;
    sys.tm.temp_pa_c = 62.5f;
    sys.fault_latch = 0;
    sys.tm.cs_pos = 512;
    sys.tm.cp_pos = 510;
    modbus_export_telemetry(&map, &sys);

    /* Read input regs from absolute 0x0100 */
    req[1] = 0x04;
    req[2] = 0x01;
    req[3] = 0x00;
    req[4] = 0x00;
    req[5] = 0x03; /* state, pf, pr */
    n = modbus_handle_pdu(&map, req, 6, resp, (int)sizeof(resp));
    expect_true("fc04 len", n == 9);
    expect_eq_u16("state", (uint16_t)((resp[3] << 8) | resp[4]), RF_STATE_RUN);
    expect_eq_u16("pf", (uint16_t)((resp[5] << 8) | resp[6]), 498);
    expect_eq_u16("pr", (uint16_t)((resp[7] << 8) | resp[8]), 12);

    uint8_t frame[16] = {1, 3, 0, 0, 0, 1};
    uint16_t crc = modbus_crc16(frame, 6);
    expect_true("crc nonzero", crc != 0);

    if (g_fail) {
        printf("%d failures\n", g_fail);
        return 1;
    }
    printf("all modbus tests passed\n");
    return 0;
}

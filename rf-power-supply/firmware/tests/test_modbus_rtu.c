#include <stdio.h>
#include <string.h>

#include "board_config.h"
#include "modbus_regs.h"
#include "modbus_rtu.h"

int main(void)
{
    modbus_map_t map;
    modbus_map_init(&map);
    map.holding[MB_H_POWER_SET_W] = 250;

    uint8_t req[8] = {RF1K_MODBUS_SLAVE, 0x03, 0x00, 0x01, 0x00, 0x01, 0, 0};
    uint16_t crc = modbus_crc16(req, 6);
    req[6] = (uint8_t)(crc & 0xff);
    req[7] = (uint8_t)(crc >> 8);

    uint8_t resp[64];
    int n = modbus_rtu_process_frame(&map, req, 8, resp, (int)sizeof(resp));
    if (n < 7) {
        printf("FAIL rtu len %d\n", n);
        return 1;
    }
    uint16_t val = (uint16_t)((resp[3] << 8) | resp[4]);
    if (val != 250) {
        printf("FAIL value %u\n", val);
        return 1;
    }
    printf("ok   modbus rtu read holding=%u resp_len=%d\n", val, n);
    return 0;
}

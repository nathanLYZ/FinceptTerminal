#include "modbus_regs.h"
#include "board_config.h"

#include <string.h>

/**
 * Feed a raw UART byte stream; emits complete RTU frames (addr..crc).
 * Call from Comm task with inter-frame idle detection already done,
 * or pass a complete frame buffer.
 */
int modbus_rtu_process_frame(modbus_map_t *map, const uint8_t *frame, int len,
                             uint8_t *out, int out_max)
{
    if (len < 4) {
        return -1;
    }
    uint16_t crc_rx = (uint16_t)(frame[len - 2] | (frame[len - 1] << 8));
    uint16_t crc_calc = modbus_crc16(frame, len - 2);
    if (crc_rx != crc_calc) {
        return -2;
    }
    if (frame[0] != RF1K_MODBUS_SLAVE && frame[0] != 0) {
        return 0; /* not for us */
    }

    int body = modbus_handle_pdu(map, frame, len - 2, out, out_max - 2);
    if (body < 0) {
        return body;
    }
    uint16_t crc = modbus_crc16(out, body);
    out[body] = (uint8_t)(crc & 0xffu);
    out[body + 1] = (uint8_t)((crc >> 8) & 0xffu);
    return body + 2;
}

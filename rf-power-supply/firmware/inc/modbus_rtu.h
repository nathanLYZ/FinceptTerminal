#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "modbus_regs.h"

int modbus_rtu_process_frame(modbus_map_t *map, const uint8_t *frame, int len,
                             uint8_t *out, int out_max);

#endif /* MODBUS_RTU_H */

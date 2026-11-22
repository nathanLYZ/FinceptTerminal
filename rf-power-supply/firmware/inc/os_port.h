/**
 * OS portability: host uses FreeRTOS shim; STM32 uses real FreeRTOS.
 */
#ifndef OS_PORT_H
#define OS_PORT_H

#if defined(RF_TARGET_STM32)
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#else
#include "freertos_shim.h"
/* Host shim has no queues — provide minimal stand-ins for Modbus RX path */
typedef struct {
    uint8_t *buf;
    int cap;
    int len;
} ShimQueue_t;
typedef ShimQueue_t *QueueHandle_t;
static inline QueueHandle_t xQueueCreate(unsigned len, unsigned item)
{
    (void)len;
    (void)item;
    return 0;
}
static inline int xQueueSend(QueueHandle_t q, const void *item, unsigned t)
{
    (void)q;
    (void)item;
    (void)t;
    return 0;
}
static inline int xQueueReceive(QueueHandle_t q, void *item, unsigned t)
{
    (void)q;
    (void)item;
    (void)t;
    return 0;
}
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS 1
#endif

#endif /* OS_PORT_H */

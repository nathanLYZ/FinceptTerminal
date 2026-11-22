/**
 * RS-485 Modbus RTU on USART2 + DE pin.
 * Uses idle-line detection to delimit frames.
 */
#include "board.h"
#include "board_config.h"

#include <string.h>

#define FRAME_Q 4

static uint8_t s_frame_q[FRAME_Q][RF1K_MB_FRAME_MAX];
static uint16_t s_frame_len[FRAME_Q];
static volatile uint8_t s_q_w, s_q_r;

void board_modbus_on_frame(const uint8_t *data, uint16_t len)
{
    if (len == 0 || len > RF1K_MB_FRAME_MAX) {
        return;
    }
    uint8_t idx = (uint8_t)(s_q_w % FRAME_Q);
    memcpy(s_frame_q[idx], data, len);
    s_frame_len[idx] = len;
    s_q_w++;
}

int board_modbus_pop_request(uint8_t *buf, int cap)
{
    if (s_q_r == s_q_w) {
        return 0;
    }
    uint8_t idx = (uint8_t)(s_q_r % FRAME_Q);
    uint16_t n = s_frame_len[idx];
    if (n > (uint16_t)cap) {
        n = (uint16_t)cap;
    }
    memcpy(buf, s_frame_q[idx], n);
    s_q_r++;
    return (int)n;
}

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
#include "main.h"
extern UART_HandleTypeDef huart2;
#define RX_RING 512
static uint8_t s_rx_ring[RX_RING];
static volatile uint16_t s_rx_w;
static uint8_t s_rx_byte;

void board_modbus_uart_init(void)
{
    s_rx_w = 0;
    s_q_w = s_q_r = 0;
    HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
}

void board_modbus_send(const uint8_t *data, uint16_t len)
{
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET);
    HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 50);
    HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART2) {
        return;
    }
    s_rx_ring[s_rx_w % RX_RING] = s_rx_byte;
    s_rx_w++;
    HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1);
}

void RF1K_UART_IdleHandler(void)
{
    static uint16_t last;
    uint16_t now = s_rx_w;
    uint16_t n = (uint16_t)(now - last);
    if (n == 0 || n > RF1K_MB_FRAME_MAX) {
        last = now;
        return;
    }
    uint8_t tmp[RF1K_MB_FRAME_MAX];
    for (uint16_t i = 0; i < n; ++i) {
        tmp[i] = s_rx_ring[(last + i) % RX_RING];
    }
    last = now;
    board_modbus_on_frame(tmp, n);
}

#else

void board_modbus_uart_init(void)
{
    s_q_w = s_q_r = 0;
}

void board_modbus_send(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
}

#endif

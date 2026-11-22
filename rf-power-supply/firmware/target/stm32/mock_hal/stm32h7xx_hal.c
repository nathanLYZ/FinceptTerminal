#include "stm32h7xx_hal.h"

static int s_gpio_ports[8];
GPIO_TypeDef *GPIOB = (GPIO_TypeDef *)&s_gpio_ports[0];
GPIO_TypeDef *GPIOD = (GPIO_TypeDef *)&s_gpio_ports[1];
GPIO_TypeDef *GPIOE = (GPIO_TypeDef *)&s_gpio_ports[2];

void HAL_Init(void) {}
void HAL_Delay(uint32_t ms) { (void)ms; }
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState s)
{
    (void)port;
    (void)pin;
    (void)s;
}
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    (void)port;
    (void)pin;
    return GPIO_PIN_SET; /* interlock closed */
}
void HAL_SPI_Transmit(SPI_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t)
{
    (void)h;
    (void)d;
    (void)n;
    (void)t;
}
void HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *d, uint16_t n, uint32_t t)
{
    (void)h;
    (void)d;
    (void)n;
    (void)t;
}
void HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *d, uint16_t n)
{
    (void)h;
    (void)d;
    (void)n;
}
void HAL_ADC_Start_DMA(ADC_HandleTypeDef *h, uint32_t *buf, uint32_t n)
{
    (void)h;
    (void)buf;
    (void)n;
}
void HAL_DAC_SetValue(DAC_HandleTypeDef *h, uint32_t ch, uint32_t align, uint32_t v)
{
    (void)h;
    (void)ch;
    (void)align;
    (void)v;
}
void HAL_DAC_Start(DAC_HandleTypeDef *h, uint32_t ch)
{
    (void)h;
    (void)ch;
}
void __HAL_UART_ENABLE_IT(UART_HandleTypeDef *h, uint32_t it)
{
    (void)h;
    (void)it;
}

#pragma once
#include <cstdint>

struct GPIO_TypeDef {
  int id;
};
struct ADC_HandleTypeDef {
  int Instance;
};
struct DAC_HandleTypeDef {
  int Instance;
};
struct SPI_HandleTypeDef {
  int Instance;
};
struct UART_HandleTypeDef {
  int Instance;
};

enum GPIO_PinState { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 };

#define GPIO_PIN_0 (1u << 0)
#define GPIO_PIN_1 (1u << 1)
#define GPIO_PIN_2 (1u << 2)
#define GPIO_PIN_9 (1u << 9)
#define GPIO_PIN_10 (1u << 10)
#define GPIO_PIN_11 (1u << 11)
#define GPIO_PIN_12 (1u << 12)
#define GPIO_PIN_13 (1u << 13)
#define GPIO_PIN_14 (1u << 14)
#define GPIO_PIN_15 (1u << 15)
#define ADC1 1
#define DAC_CHANNEL_1 1
#define DAC_ALIGN_12B_R 0

extern GPIO_TypeDef *GPIOB, *GPIOD, *GPIOE;
extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern SPI_HandleTypeDef hspi4;

void HAL_Delay(uint32_t ms);
void HAL_GPIO_WritePin(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState s);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef* port, uint16_t pin);
void HAL_ADC_Start_DMA(ADC_HandleTypeDef* h, uint32_t* buf, uint32_t n);
void HAL_DAC_SetValue(DAC_HandleTypeDef* h, uint32_t ch, uint32_t align,
                      uint32_t v);
void HAL_DAC_Start(DAC_HandleTypeDef* h, uint32_t ch);
void HAL_SPI_Transmit(SPI_HandleTypeDef* h, uint8_t* d, uint16_t n, uint32_t t);

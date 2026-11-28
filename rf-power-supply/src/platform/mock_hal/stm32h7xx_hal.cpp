#include "stm32h7xx_hal.h"

static int ports[4];
GPIO_TypeDef* GPIOB = reinterpret_cast<GPIO_TypeDef*>(&ports[0]);
GPIO_TypeDef* GPIOD = reinterpret_cast<GPIO_TypeDef*>(&ports[1]);
GPIO_TypeDef* GPIOE = reinterpret_cast<GPIO_TypeDef*>(&ports[2]);
ADC_HandleTypeDef hadc1{ADC1};
DAC_HandleTypeDef hdac1{1};
SPI_HandleTypeDef hspi4{4};

static uint32_t gpio_state;

void HAL_Delay(uint32_t) {}
void HAL_GPIO_WritePin(GPIO_TypeDef*, uint16_t pin, GPIO_PinState s) {
  if (s == GPIO_PIN_SET)
    gpio_state |= pin;
  else
    gpio_state &= ~static_cast<uint32_t>(pin);
}
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef*, uint16_t pin) {
  // Default: interlock closed (SET), limits not hit (RESET), fault not latched
  if (pin == GPIO_PIN_10) return GPIO_PIN_SET;  // INTERLOCK
  if (pin == GPIO_PIN_2) return GPIO_PIN_RESET;  // FAULT_LATCH / LIMIT
  if (pin == GPIO_PIN_1) return GPIO_PIN_RESET;
  return (gpio_state & pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}
void HAL_ADC_Start_DMA(ADC_HandleTypeDef*, uint32_t*, uint32_t) {}
void HAL_DAC_SetValue(DAC_HandleTypeDef*, uint32_t, uint32_t, uint32_t) {}
void HAL_DAC_Start(DAC_HandleTypeDef*, uint32_t) {}
void HAL_SPI_Transmit(SPI_HandleTypeDef*, uint8_t*, uint16_t, uint32_t) {}

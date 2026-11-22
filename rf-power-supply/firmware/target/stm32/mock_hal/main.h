/**
 * Minimal CubeMX-like main.h for host-side STM32 compile checks.
 * Real firmware uses the CubeMX-generated main.h instead.
 */
#ifndef MAIN_H_MOCK
#define MAIN_H_MOCK

#include "stm32h7xx_hal.h"

#define RF_ENABLE_Pin GPIO_PIN_0
#define RF_ENABLE_GPIO_Port GPIOB
#define FAULT_CLR_Pin GPIO_PIN_1
#define FAULT_CLR_GPIO_Port GPIOB
#define FAULT_LATCH_Pin GPIO_PIN_2
#define FAULT_LATCH_GPIO_Port GPIOB
#define INTERLOCK_Pin GPIO_PIN_10
#define INTERLOCK_GPIO_Port GPIOB

#define DDS_CS_Pin GPIO_PIN_11
#define DDS_CS_GPIO_Port GPIOE
#define DDS_IOUPDATE_Pin GPIO_PIN_10
#define DDS_IOUPDATE_GPIO_Port GPIOE
#define DDS_RESET_Pin GPIO_PIN_9
#define DDS_RESET_GPIO_Port GPIOE

#define STEP_CS_Pin GPIO_PIN_12
#define STEP_CS_GPIO_Port GPIOD
#define DIR_CS_Pin GPIO_PIN_13
#define DIR_CS_GPIO_Port GPIOD
#define STEP_CP_Pin GPIO_PIN_14
#define STEP_CP_GPIO_Port GPIOD
#define DIR_CP_Pin GPIO_PIN_15
#define DIR_CP_GPIO_Port GPIOD
#define EN_STEPPER_Pin GPIO_PIN_0
#define EN_STEPPER_GPIO_Port GPIOE

#define RS485_DE_Pin GPIO_PIN_7
#define RS485_DE_GPIO_Port GPIOD

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_ADC1_Init(void);
void MX_DAC1_Init(void);
void MX_SPI4_Init(void);
void MX_USART2_UART_Init(void);
void MX_FREERTOS_Init(void);

#endif

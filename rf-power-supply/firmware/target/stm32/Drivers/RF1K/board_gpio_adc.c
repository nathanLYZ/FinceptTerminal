/**
 * Board bring-up: GPIO actuators, DAC bus trim, ADC raw mirror.
 */
#include "board.h"
#include "board_config.h"

board_adc_raw_t g_adc_raw;

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac1;
extern TIM_HandleTypeDef htim6;

/* DMA buffer: Pf Pr Vbus Idc TempPa Cs Cp */
static uint16_t s_adc_dma[7];

void board_init(void)
{
    board_cal_load_defaults();
    board_dds_init();
    board_stepper_init();
    board_modbus_uart_init();
    board_rf_enable(false);
    board_set_vbus_dac(0.0f);
    board_adc_start();
}

void board_adc_start(void)
{
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)s_adc_dma, 7);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance != ADC1) {
        return;
    }
    g_adc_raw.raw_pf = s_adc_dma[0];
    g_adc_raw.raw_pr = s_adc_dma[1];
    g_adc_raw.raw_vbus = s_adc_dma[2];
    g_adc_raw.raw_idc = s_adc_dma[3];
    g_adc_raw.raw_temp_pa = s_adc_dma[4];
    g_adc_raw.raw_cs = s_adc_dma[5];
    g_adc_raw.raw_cp = s_adc_dma[6];
}

void board_set_vbus_dac(float volts)
{
    float norm = volts / RF1K_VBUS_MAX_V;
    if (norm < 0.0f) {
        norm = 0.0f;
    }
    if (norm > 1.0f) {
        norm = 1.0f;
    }
    uint32_t code = (uint32_t)(norm * 4095.0f + 0.5f);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, code);
    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);
}

void board_rf_enable(bool on)
{
    HAL_GPIO_WritePin(RF_ENABLE_GPIO_Port, RF_ENABLE_Pin,
                      on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool board_interlock_ok(void)
{
    return HAL_GPIO_ReadPin(INTERLOCK_GPIO_Port, INTERLOCK_Pin) == GPIO_PIN_SET;
}

bool board_fault_latched(void)
{
    return HAL_GPIO_ReadPin(FAULT_LATCH_GPIO_Port, FAULT_LATCH_Pin) == GPIO_PIN_SET;
}

void board_fault_clear(void)
{
    HAL_GPIO_WritePin(FAULT_CLR_GPIO_Port, FAULT_CLR_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(FAULT_CLR_GPIO_Port, FAULT_CLR_Pin, GPIO_PIN_RESET);
}

#else

void board_init(void)
{
    board_cal_load_defaults();
    board_dds_init();
    board_stepper_init();
    board_modbus_uart_init();
    board_adc_start();
}

void board_adc_start(void)
{
    /* Mock: mid-scale detectors until host plant drives values via stub path.
       When building stm32-check with hw_stm32, tests inject raw samples. */
    g_adc_raw.raw_pf = 20000;
    g_adc_raw.raw_pr = 1000;
    g_adc_raw.raw_vbus = 30000;
    g_adc_raw.raw_idc = 10000;
    g_adc_raw.raw_temp_pa = 20000;
    g_adc_raw.raw_cs = 32768;
    g_adc_raw.raw_cp = 32768;
}

void board_set_vbus_dac(float volts) { (void)volts; }
void board_rf_enable(bool on) { (void)on; }
bool board_interlock_ok(void) { return true; }
bool board_fault_latched(void) { return false; }
void board_fault_clear(void) {}

#endif

/**
 * AD9951 SPI driver (13.56 MHz CW).
 * CubeMX: SPI4, soft CS on PE11.
 */
#include "board.h"
#include "board_config.h"

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
#include "main.h"
extern SPI_HandleTypeDef hspi4;
#endif

static float s_amp;

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)

static void cs(int level)
{
    HAL_GPIO_WritePin(DDS_CS_GPIO_Port, DDS_CS_Pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void spi_write(const uint8_t *data, uint16_t n)
{
    cs(0);
    HAL_SPI_Transmit(&hspi4, (uint8_t *)data, n, 20);
    cs(1);
}

static void ad9951_write_reg(uint8_t reg, const uint8_t *payload, uint8_t n)
{
    uint8_t buf[8];
    buf[0] = reg;
    for (uint8_t i = 0; i < n && i < 7; ++i) {
        buf[1 + i] = payload[i];
    }
    spi_write(buf, (uint16_t)(1 + n));
    /* IO_UPDATE pulse */
    HAL_GPIO_WritePin(DDS_IOUPDATE_GPIO_Port, DDS_IOUPDATE_Pin, GPIO_PIN_SET);
    for (volatile int i = 0; i < 50; ++i) {
    }
    HAL_GPIO_WritePin(DDS_IOUPDATE_GPIO_Port, DDS_IOUPDATE_Pin, GPIO_PIN_RESET);
}

#endif /* real HAL */

void board_dds_init(void)
{
    s_amp = 0.0f;
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
    HAL_GPIO_WritePin(DDS_RESET_GPIO_Port, DDS_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(DDS_RESET_GPIO_Port, DDS_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    board_dds_set_freq_hz(RF1K_F0_HZ);
#endif
}

void board_dds_set_freq_hz(uint32_t hz)
{
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
    /* FTW = f * 2^32 / SYSCLK ; assume 400 MHz sync clock */
    const double sysclk = 400e6;
    uint32_t ftw = (uint32_t)((double)hz / sysclk * 4294967296.0);
    uint8_t p[4] = {
        (uint8_t)(ftw >> 24), (uint8_t)(ftw >> 16),
        (uint8_t)(ftw >> 8), (uint8_t)ftw
    };
    ad9951_write_reg(0x04, p, 4); /* FTW0 */
#else
    (void)hz;
#endif
}

void board_dds_set_amplitude(float norm_0_1)
{
    if (norm_0_1 < 0.0f) {
        norm_0_1 = 0.0f;
    }
    if (norm_0_1 > 1.0f) {
        norm_0_1 = 1.0f;
    }
    s_amp = norm_0_1;
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
    /* ASF in CFR — simplified: write amplitude scale register if used;
       many RF-1K builds use external VGA driven by DAC instead. */
    (void)s_amp;
#endif
}

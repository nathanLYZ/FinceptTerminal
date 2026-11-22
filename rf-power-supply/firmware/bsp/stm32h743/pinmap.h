/**
 * STM32H743VIT6 pin assignment draft for RF-1K CTRL board (LQFP-100).
 * String tokens are for CubeMX / documentation — not compiled as GPIO defs.
 */
#ifndef RF1K_PINMAP_H
#define RF1K_PINMAP_H

/* ADC — Pf / Pr / Vbus / Idc / NTC */
#define PIN_ADC_PF            "PA0"   /* ADC1_INP16 */
#define PIN_ADC_PR            "PA1"
#define PIN_ADC_VBUS          "PA2"
#define PIN_ADC_IDC           "PA3"
#define PIN_ADC_TEMP_PA       "PC0"
#define PIN_ADC_TEMP_PSU      "PC1"
#define PIN_ADC_CS_POS        "PC2"
#define PIN_ADC_CP_POS        "PC3"

/* DAC / SPI for DDS (move DDS to SPI4 if ETH RMII needs PA1/PA7) */
#define PIN_DAC_VBUS_REF      "PA4"
#define PIN_SPI_DDS           "SPI4"
#define PIN_SPI4_SCK          "PE12"
#define PIN_SPI4_MISO         "PE13"
#define PIN_SPI4_MOSI         "PE14"
#define PIN_DDS_CS            "PE11"
#define PIN_DDS_IOUPDATE      "PE10"
#define PIN_DDS_RESET         "PE9"

/* RF enable / protection latch */
#define PIN_RF_ENABLE         "PB0"
#define PIN_FAULT_LATCH_CLR   "PB1"
#define PIN_FAULT_LATCH_IN    "PB2"   /* EXTI */
#define PIN_INTERLOCK_IN      "PB10"  /* EXTI falling = open */
#define PIN_CONTACTOR         "PB11"

/* Stepper STEP/DIR */
#define PIN_STEP_CS           "PD12"
#define PIN_DIR_CS            "PD13"
#define PIN_STEP_CP           "PD14"
#define PIN_DIR_CP            "PD15"
#define PIN_EN_STEPPER        "PE0"

/* RS-485 */
#define PIN_UART2_TX          "PD5"
#define PIN_UART2_RX          "PD6"
#define PIN_RS485_DE          "PD7"

/* Ethernet RMII (LAN8742A) — keep default Nucleo-style pins free of DDS */
#define PIN_ETH_RMII          "LAN8742A_RMII_DEFAULT"

/* HMI */
#define PIN_HMI_SPI_SCK       "PB3"
#define PIN_HMI_SPI_MOSI      "PB5"
#define PIN_HMI_TFT_CS        "PB6"
#define PIN_HMI_TFT_DC        "PB7"
#define PIN_HMI_ENC_A         "PE2"
#define PIN_HMI_ENC_B         "PE3"
#define PIN_HMI_ENC_BTN       "PE4"
#define PIN_ESTOP_IN          "PE5"

#endif /* RF1K_PINMAP_H */

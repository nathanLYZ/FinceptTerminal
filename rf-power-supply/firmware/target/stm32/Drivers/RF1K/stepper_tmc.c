/**
 * STEP/DIR stepper backend for Cs/Cp (TMC2209 in STEP/DIR mode).
 */
#include "board.h"

static int32_t s_pos[2];

#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
#include "main.h"

static void pulse_step(GPIO_TypeDef *port, uint16_t pin)
{
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    for (volatile int i = 0; i < 80; ++i) {
    }
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    for (volatile int i = 0; i < 80; ++i) {
    }
}
#endif

void board_stepper_init(void)
{
    s_pos[0] = 200;
    s_pos[1] = 800;
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
    HAL_GPIO_WritePin(EN_STEPPER_GPIO_Port, EN_STEPPER_Pin, GPIO_PIN_RESET); /* enable */
#endif
}

void board_stepper_move(int axis, int delta_steps)
{
    if (axis < 0 || axis > 1) {
        return;
    }
    int dir = delta_steps >= 0 ? 1 : 0;
    int n = delta_steps >= 0 ? delta_steps : -delta_steps;
#if defined(RF_TARGET_STM32) && !defined(RF_STM32_MOCK)
    if (axis == 0) {
        HAL_GPIO_WritePin(DIR_CS_GPIO_Port, DIR_CS_Pin, dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
        for (int i = 0; i < n; ++i) {
            pulse_step(STEP_CS_GPIO_Port, STEP_CS_Pin);
        }
    } else {
        HAL_GPIO_WritePin(DIR_CP_GPIO_Port, DIR_CP_Pin, dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
        for (int i = 0; i < n; ++i) {
            pulse_step(STEP_CP_GPIO_Port, STEP_CP_Pin);
        }
    }
#else
    (void)dir;
    (void)n;
#endif
    s_pos[axis] += delta_steps;
}

int32_t board_stepper_pos(int axis)
{
    if (axis < 0 || axis > 1) {
        return 0;
    }
    return s_pos[axis];
}

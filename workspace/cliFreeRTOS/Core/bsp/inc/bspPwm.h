#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "stdint.h"
#include "stm32f4xx_hal.h"

typedef enum
{
    PWM_CH_1,
    PWM_CH_2,
    PWM_CH_3,
    PWM_CH_4,
    MAX_PWM_CH,
} pwmChannels_e;

HAL_StatusTypeDef bspPwmSetFreq(uint32_t uNewFreq);
TIM_HandleTypeDef* bspPwmGetHandler(void);
void bspPwmStart(pwmChannels_e eChannelIndex);
HAL_StatusTypeDef bspPwmSetDuty(uint8_t uNewDuty, pwmChannels_e xChannel);
HAL_StatusTypeDef bspPwmInit(void);


#endif

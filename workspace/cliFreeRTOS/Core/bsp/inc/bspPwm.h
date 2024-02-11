/**
  ******************************************************************************
  * @file    bspPwm.h
  * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
  * @brief   Header file that exposes PWM data types and PWM APIs
  ******************************************************************************
*/

#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "bspTypeDef.h"

typedef enum
{
    PWM_CH_1,
    PWM_CH_2,
    PWM_CH_3,
    PWM_CH_4,
    MAX_PWM_CH,
} pwmChannels_e;

BspError_e bspPwmInit(void);
TIM_HandleTypeDef* bspPwmGetHandler(void);
BspError_e bspPwmSetFreq(uint32_t uNewFreq);
void bspPwmStart(pwmChannels_e eChannelIndex);
BspError_e bspPwmSetDuty(uint8_t uNewDuty, pwmChannels_e xChannel);

#endif

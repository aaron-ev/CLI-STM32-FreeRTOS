/**
  ******************************************************************************
  * @file    bspRtc.h
  * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
  * @brief   Header file that exposes RTC APIs
  ******************************************************************************
*/

#ifndef __BSP_RTC_H
#define __BSP_RTC_H

#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "bspTypeDef.h"

typedef struct
{
    uint8_t uHours;
    uint8_t uMinutes;
    uint8_t uSeconds;
}BspRtcTime;

BspError_e bspRtcInit(void);
BspError_e bspRtcGetTime(BspRtcTime* bspRtcTime);
BspError_e bspRtcSetTime(BspRtcTime* bspRtcTime);

#endif

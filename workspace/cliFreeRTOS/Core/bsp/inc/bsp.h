/**
  ******************************************************************************
  * @file    bsp.h
  * @author  Aaron Escoboza
  * @brief   Header file to expose initialization functions
  ******************************************************************************
*/


#ifndef __BSP__H
#define __BSP__H

#include "appConfig.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "bspPwm.h"
#include "bspGpio.h"
#include "bspClk.h"
#include "errno.h"

typedef enum
{
    BSP_NO_ERROR,
    BSP_ERROR_EIO = EIO,
} BspError_e;

BspError_e bspInit(void);

#endif

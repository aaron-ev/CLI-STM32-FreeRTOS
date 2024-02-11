/**
  ******************************************************************************
  * @file    bspClk.h
  * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
  * @brief   Header file for clock information
  ******************************************************************************
*/

#ifndef __BSP_CLK_H
#define __BSP_CLK_H


#include "stdio.h"
#include "stdint.h"
#include "stm32f4xx_hal.h"

void bspGetClockIinfo(char *pcWriteBuffer, size_t xWriteBufferLen);

#endif

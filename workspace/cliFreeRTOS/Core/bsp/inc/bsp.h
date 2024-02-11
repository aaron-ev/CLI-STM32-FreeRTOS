/**
******************************************************************************
* @file    bsp.h
* @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
* @brief   Header file to expose a BSP generic functions
******************************************************************************
*/

#ifndef __BSP__H
#define __BSP__H

#include "bspPwm.h"
#include "bspGpio.h"
#include "bspClk.h"
#include "bspRtc.h"

BspError_e bspInit(void);

#endif

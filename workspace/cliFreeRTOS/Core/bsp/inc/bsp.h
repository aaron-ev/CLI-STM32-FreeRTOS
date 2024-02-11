/**
******************************************************************************
* @file    bsp.h
* @author  Aaron Escoboza
* @brief   Header file to expose a BSP generic functions
******************************************************************************
*/

#ifndef __BSP__H
#define __BSP__H

#include "bspPwm.h"
#include "bspGpio.h"
#include "bspClk.h"

BspError_e bspInit(void);

#endif

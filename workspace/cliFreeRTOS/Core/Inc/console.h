/**
 ******************************************************************************
 * @file    console.h
 * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
 * @brief   Console header file: APIs to handle the console.
 ******************************************************************************
 */

#ifndef __CONSOLE__H
#define __CONSOLE__H

#include "FreeRTOS.h"

BaseType_t xbspConsoleInit(uint16_t usStackSize, UBaseType_t uxPriority, UART_HandleTypeDef *pxUartHandle);

#endif

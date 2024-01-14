
#ifndef __CONSOLE__H
#define __CONSOLE__H

#include "FreeRTOS.h"

BaseType_t xConsoleInit(uint16_t usStackSize, UBaseType_t uxPriority, UART_HandleTypeDef *pxUartHandle);

#endif

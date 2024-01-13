
#ifndef __CONSOLE__H
#define __CONSOLE__H

#include "stdint.h"
#include "FreeRTOS.h"

BaseType_t xConsoleInit(uint16_t usStackSize, UBaseType_t uxPriority);
void vConsoleReadFromISR(char charToQueue);

#endif

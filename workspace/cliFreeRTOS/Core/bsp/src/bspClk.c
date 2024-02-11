/**
******************************************************************************
* @file    bspClk.c
* @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
* @brief   source file to implement functions related to the clock tree.
******************************************************************************
*/

#include "bspClk.h"

/**
* @brief Gets system clock, PCLKx and CLK dividers.
* @param *pcWriteBuffer pointer to buffer where clock information
          will be stored.
* @param xWriteBufferLen buffer length.
* @retval void
*/
void bspGetClockIinfo(char *pcWriteBuffer, size_t xWriteBufferLen)
{
    uint32_t uPCLK1;
    uint32_t uPCLK2;
    uint32_t uSysClock;
    uint32_t APB1CLKDivider;
    uint32_t APB2CLKDivider;
    uint32_t APB1TimerClocks;
    uint32_t APB2TimerClocks;

    /* Update clock system according to register values */
    SystemCoreClockUpdate();
    /* Read SysClock, PCLK1 and PCLK2 */
    uSysClock = HAL_RCC_GetHCLKFreq();
    uPCLK1 = HAL_RCC_GetPCLK1Freq();
    uPCLK2 = HAL_RCC_GetPCLK2Freq();
    /* Calculate APB1 and APB2*/
    APB1CLKDivider = (uint32_t)(RCC->CFGR & RCC_CFGR_PPRE1);
    APB2CLKDivider = (uint32_t)(RCC->CFGR & RCC_CFGR_PPRE2);
    APB1TimerClocks = (APB1CLKDivider > RCC_CFGR_PPRE1_DIV1) ? (uPCLK1 * 2) : uPCLK1;
    APB2TimerClocks = (APB2CLKDivider > RCC_CFGR_PPRE1_DIV1) ? (uPCLK2 * 2) : uPCLK2;

    snprintf(pcWriteBuffer, xWriteBufferLen,
             "Clock name           Hz       kHz       MHz\n"\
             "===========       ========  ========  ========\n"\
             "System clock      %lu     %lu        %lu\n"\
             "APB1 peripheral   %lu     %lu        %lu\n"\
             "APB2 peripheral   %lu     %lu        %lu\n"\
             "APB1 timers       %lu     %lu        %lu\n"\
             "APB2 timers       %lu     %lu        %lu\n",
             uSysClock, uSysClock / 1000, uSysClock / 1000000,
             uPCLK1, uPCLK1 / 1000, uPCLK1 / 1000000,
             uPCLK2, uPCLK2 / 1000, uPCLK2 / 1000000,
             APB1TimerClocks, APB1TimerClocks / 1000, APB1TimerClocks / 1000000,
             APB2TimerClocks, APB2TimerClocks / 1000, APB2TimerClocks / 1000000);
}

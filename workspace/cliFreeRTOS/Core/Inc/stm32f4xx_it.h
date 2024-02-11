/**
 ******************************************************************************
 * @file    stm32f4xx_it.h
 * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
 * @brief   Interrupt header file: Holds interrupt handlers.
 ******************************************************************************
 */

#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);
void TIM1_BRK_TIM9_IRQHandler(void);

#endif

/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
 * @brief   Interrupt source file: Holds interrupt handler implementations.
 ******************************************************************************
 */

#include "stm32f4xx_it.h"
#include "bspPwm.h"

extern TIM_HandleTypeDef htim9;
extern UART_HandleTypeDef consoleHandle;

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
    while (1){}
}

/**
* @brief This function handles Hard fault interrupt.
*/
void HardFault_Handler(void)
{
    while (1){}
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
    while (1){}
}

/**
* @brief This function handles Pre-fetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
    while (1){}
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
    while (1){}
}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void){}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles TIM1 break interrupt and TIM9 global interrupt.
*/
void TIM1_BRK_TIM9_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim9);
}

/**
* @brief This function handles UART1 interrupts
*/
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&consoleHandle);
}

/**
* @brief This function handles TIM2 interrupts.
*/
void TIM2_IRQHandler(void)
{
    TIM_HandleTypeDef* pwmTimHandler = bspPwmGetHandler();
    if (pwmTimHandler)
    {
        HAL_TIM_IRQHandler(pwmTimHandler);
    }
}

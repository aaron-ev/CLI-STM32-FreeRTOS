/**
 ******************************************************************************
 * @file         main.c
 * @author       Aaron Escoboza, Github account: https://github.com/aaron-ev
 * @brief        Command Line Interpreter based on FreeRTOS and STM32 HAL layer
 ******************************************************************************
 */

#include "main.h"
#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "console.h"
#include "appConfig.h"

TaskHandle_t xTaskHeartBeatHandler;
extern UART_HandleTypeDef consoleHandle;

/**
* @brief Heart beat task indicates project alive by toggling an LED.
* @param *pvParams data passed at task creation
* @retval void
*/
void vTaskHeartBeat(void *pvParams)
{
    while (1)
    {
       HAL_GPIO_TogglePin(HEART_BEAT_LED_PORT, HEART_BEAT_LED_PIN);
       vTaskDelay(pdMS_TO_TICKS(HEART_BEAT_BLINK_DELAY));
    }
}

/**
* @brief main function: Initialize BSP, console, and FreeRTOS.
* @param void
* @retval void
*/
int main(void)
{
    BaseType_t retVal;
    HAL_StatusTypeDef halStatus;

    halStatus = bspInit();
    if (halStatus != HAL_OK)
        goto main_out;

    retVal = xbspConsoleInit(CONSOLE_STACK_SIZE, CONSOLE_TASK_PRIORITY, &consoleHandle);
    if (retVal != pdTRUE)
        goto main_out;

    retVal = xTaskCreate(vTaskHeartBeat,
                         "task-heart-beat",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         HEART_BEAT_PRIORITY_TASK,
                         &xTaskHeartBeatHandler);
    if (retVal != pdTRUE)
        goto main_out;

    /* By default, all PWM channels are started */
    bspPwmStart(PWM_CH_1);
    bspPwmStart(PWM_CH_2);
    bspPwmStart(PWM_CH_3);
    bspPwmStart(PWM_CH_4);

    vTaskStartScheduler();

main_out:
    if (!xTaskHeartBeatHandler)
    {
        vTaskDelete(xTaskHeartBeatHandler);
    }
    Error_Handler();
}

/**
* @brief  Period elapsed callback in non blocking mode
* @note   This function is called  when TIM9 interrupt took place, inside
* HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
* a global variable "uwTick" used as application time base.
* @param  htim : TIM handle
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM9)
  {
    HAL_IncTick();
  }
}

/**
* @brief  This function is executed in case of error occurrence.
* @retval None
*/
void Error_Handler(void)
{
  __disable_irq();
  HAL_GPIO_WritePin(HEART_BEAT_LED_PORT, HEART_BEAT_LED_PIN, GPIO_PIN_SET);
  while (1){}
}

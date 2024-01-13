/**
 ******************************************************************************
 * @file         main.c
 * @author       Aaron Escoboza
 * @brief        Command Line Interpreter based on FreeRTOS and STM32 HAL layer
 *               Github account: https://github.com/aaron-ev
 ******************************************************************************
 */

#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "appConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "console.h"

TaskHandle_t xTaskHeartBeatHandler; 

/*
* Task to indicate the freeRTOS app is alive.
*/
void vTaskHeartBeat(void *params)
{
    while (1)
    {
       HAL_GPIO_TogglePin(HEART_BEAT_LED_PORT, HEART_BEAT_LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(HEART_BEAT_BLINK_DELAY));
    }
}

int main(void)
{
    BaseType_t retVal;
    HAL_StatusTypeDef halStatus;

    halStatus = bspInit();
    if (halStatus != HAL_OK)
    {
        goto main_out;
    }

    retVal = xConsoleInit(3000, CONSOLE_TASK_PRIORITY);
    if (retVal != pdTRUE)
    {
        goto main_out;
    }

    /* Create tasks and start the scheduler */
    retVal = xTaskCreate(vTaskHeartBeat,
                         "task-heart-beat",
                         configMINIMAL_STACK_SIZE,
                         NULL,
                         HEART_BEAT_PRIORITY_TASK,
                         &xTaskHeartBeatHandler);
    if (retVal != pdTRUE)
    {
        goto main_out;
    }

    vTaskStartScheduler();

main_out:
    if (!xTaskHeartBeatHandler)
    {
        vTaskDelete(xTaskHeartBeatHandler);
    }
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
  while (1)
  {
  }
}

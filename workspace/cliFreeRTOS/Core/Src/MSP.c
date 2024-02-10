#include "main.h"
#include "appConfig.h"

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
    /* Enable clocks for some system settings */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    /* Enable clock for GPIOs being used */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM5_CLK_ENABLE();

    /* Enable clock for the console */
    __HAL_RCC_USART1_CLK_ENABLE();

    HAL_NVIC_SetPriority(TIM2_IRQn, 14, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandler)
{
    GPIO_InitTypeDef uartGpioInit;

    if (uartHandler->Instance == CONSOLE_INSTANCE)
    {
        /* Configure pins to work as a UART device */
        uartGpioInit.Pin = CONSOLE_TX_PIN | CONSOLE_RX_PIN;
        uartGpioInit.Mode = GPIO_MODE_AF_PP;
        uartGpioInit.Pull = GPIO_NOPULL;
        uartGpioInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        uartGpioInit.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(CONSOLE_GPIO_PORT, &uartGpioInit);

        /* Configure interrupt*/
        HAL_NVIC_SetPriority(USART1_IRQn, 15 , 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *timerHandler)
{
    GPIO_InitTypeDef pwmGpioInit = {0};

    pwmGpioInit.Pin = PWM_GPIO_PINX;
    pwmGpioInit.Mode = GPIO_MODE_AF_PP;
    pwmGpioInit.Pull = GPIO_NOPULL;
    pwmGpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    pwmGpioInit.Alternate = PWM_GPIO_ALTERNATE;
    HAL_GPIO_Init(PWM_GPIO_INSTANCE, &pwmGpioInit);
}

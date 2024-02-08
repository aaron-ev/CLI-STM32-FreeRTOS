/**
 ******************************************************************************
 * @file    bsp.c
 * @author  Aaron Escoboza
 * @brief   source file to implement low level initializations.
 ******************************************************************************
 */

#include "bsp.h"

UART_HandleTypeDef consoleHandle;

/**
* @brief Initialize system clocks, PLL and Clock dividers.
* @param void
* @retval BspError_e
*/
static BspError_e clkInit(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 80;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        return BSP_ERROR_EIO;
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        return BSP_ERROR_EIO;
    }

    return BSP_NO_ERROR;
}

/**
* @brief Initialize GPIO pin for heart beat functionality.
* @param void
* @retval void
*/
static void heartBeatInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Heart beat: GPIO settings  */
    GPIO_InitStruct.Pin = HEART_BEAT_LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HEART_BEAT_LED_PORT, &GPIO_InitStruct);
}

/**
* @brief Initialize UART frame.
* @param void
* @retval BSP error
*/
BspError_e consoleInit(void)
{
    /* GPIO initializations */
    consoleHandle.Instance = CONSOLE_INSTANCE;
    consoleHandle.Init.BaudRate = CONSOLE_BAUDRATE;
    consoleHandle.Init.WordLength = UART_WORDLENGTH_8B;
    consoleHandle.Init.StopBits = UART_STOPBITS_1;
    consoleHandle.Init.Parity = UART_PARITY_NONE;
    consoleHandle.Init.Mode = UART_MODE_TX_RX;
    consoleHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    consoleHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&consoleHandle) != HAL_OK)
    {
        return BSP_ERROR_EIO;
    }

    return BSP_NO_ERROR;
}

/**
* @brief Calls all BSP init functions.
* @param void
* @retval BspError_e
*/
BspError_e bspInit(void)
{
    HAL_StatusTypeDef halStatus;

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    halStatus = HAL_Init();
    if (halStatus != HAL_OK)
    {
        return BSP_ERROR_EIO;
    }

    halStatus = clkInit();
    if (halStatus != HAL_OK)
    {
        return BSP_ERROR_EIO;
    }

    halStatus = consoleInit();
    if (halStatus != HAL_OK)
    {
        return BSP_ERROR_EIO;
    }

    heartBeatInit();

    halStatus = bspPwmInit();
    if (halStatus != HAL_OK)
    {
        return BSP_ERROR_EIO;
    }

    return BSP_NO_ERROR;
}

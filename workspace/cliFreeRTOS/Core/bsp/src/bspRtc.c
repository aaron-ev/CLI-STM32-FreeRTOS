/**
 ******************************************************************************
 * @file    bspRtc.
 * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
 * @brief   source file to implement functions RTC peripheral.
 ******************************************************************************
 */

#include "bspRtc.h"
#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "appConfig.h"

static RTC_HandleTypeDef xRtcHandler;

/**
* @brief Get the current time stored in RTC registers
* @param bspRtcTime pointer to a RTC timer structure
* @retval BSP error
*/
BspError_e bspRtcGetTime(BspRtcTime* bspRtcTime)
{
    RTC_TimeTypeDef xTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&xRtcHandler, &xTime, RTC_FORMAT_BIN);

    /* There is an issue with STM32, date should be
    *  read after time to make RTC work properly.
    *  Ref: https://community.st.com/t5/stm32-mcus-products/while-reading-the-rtc-not-providing-proper-date-after-the-rtc/td-p/138546
    */
    HAL_RTC_GetDate(&xRtcHandler, &sDate, RTC_FORMAT_BIN);

    bspRtcTime->uHours = xTime.Hours;
    bspRtcTime->uMinutes = xTime.Minutes;
    bspRtcTime->uSeconds = xTime.Seconds;
    return BSP_NO_ERROR;
}

/**
* @brief Set a new time to RTC registers
* @param bspRtcTime pointer to a RTC timer structure
* @retval BSP error
*/
BspError_e bspRtcSetTime(BspRtcTime* bspRtcTime)
{
    RTC_TimeTypeDef xTime = {0};
    HAL_StatusTypeDef halStatus;

    if (bspRtcTime->uHours < 0 || bspRtcTime->uHours > 24)
        return BSP_ERROR_EINVAL;
    if (bspRtcTime->uMinutes < 0 || bspRtcTime->uHours > 59)
        return BSP_ERROR_EINVAL;
    if (bspRtcTime->uSeconds < 0 || bspRtcTime->uSeconds > 59)
        return BSP_ERROR_EINVAL;

    xTime.Hours = bspRtcTime->uHours;
    xTime.Minutes = bspRtcTime->uMinutes;
    xTime.Seconds = bspRtcTime->uSeconds;
    halStatus = HAL_RTC_SetTime(&xRtcHandler, &xTime, RTC_FORMAT_BIN);
    if (halStatus != HAL_OK)
        return BSP_ERROR_EIO;

    return BSP_NO_ERROR;
}

/**
* @brief Initialize RTC peripheral.
* @param void
* @retval BSP error
*/
BspError_e bspRtcInit(void)
 {
    RTC_TimeTypeDef timeDef;

    xRtcHandler.Instance = RTC;
    xRtcHandler.Init.HourFormat = RTC_HOURFORMAT_24;
    xRtcHandler.Init.AsynchPrediv = 127;
    xRtcHandler.Init.SynchPrediv = 255;
    xRtcHandler.Init.OutPut = RTC_OUTPUT_DISABLE;
    xRtcHandler.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_LOW;
    xRtcHandler.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&xRtcHandler) != HAL_OK)
        return BSP_ERROR_EIO;

    timeDef.Hours = 12;
    timeDef.Minutes = 0;
    if (HAL_RTC_SetTime(&xRtcHandler, &timeDef, RTC_FORMAT_BIN) != HAL_OK)
        return BSP_ERROR_EIO;

    return BSP_NO_ERROR;
 }

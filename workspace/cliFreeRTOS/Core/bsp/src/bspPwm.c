/**
 ******************************************************************************
 * @file    bspPwm.
 * @author  Aaron Escoboza
 * @brief   source file to implement functions related to PWM signal
 *          manipulation.
 ******************************************************************************
 */

#include "bspPwm.h"
#include "stm32f4xx_hal.h"
#include "stdint.h"
#include "appConfig.h"

#define PWM_MAX_CHANNELS                4
#define PWM_DEFAULT_FREQ                1000 /* 1kHz when clk is 1Mhz */

typedef struct
{
  TIM_OC_InitTypeDef xOcInit;
  uint16_t channel;
  uint8_t uDuty;
}Pwm_TIM_OC_InitTypeDef;

typedef struct
{
    TIM_HandleTypeDef xTimHandle;
    Pwm_TIM_OC_InitTypeDef uChannelXConfig[PWM_MAX_CHANNELS];
}PwmConfigStruct;

PwmConfigStruct pwmConfigStruct =
{
    {
        PWM_TIM_INSTANCE,                               /* Timer instance */
        {   /* TIM_Base_InitTypeDef */
            40,                                         /* Prescaler */
            TIM_COUNTERMODE_UP,                         /* Counter mode */
            PWM_DEFAULT_FREQ - 1,                                   /* Period */
            TIM_CLOCKDIVISION_DIV1                      /* Clock division */
        }
    },
    {
        {   /* Channel 1 */
            {
                TIM_OCMODE_PWM1,                         /* Specifies the TIM mode */
                ( (PWM_DEFAULT_FREQ - 1) * 50 ) / 100,   /* Pulse value */
                TIM_OCNPOLARITY_HIGH,                    /* Output polarity */
            },
            TIM_CHANNEL_1,                               /* Channel number */
            50                                           /* Duty cycle */
        },
        {   /* Channel 2 */
            {
                TIM_OCMODE_PWM1,                        /* Specifies the TIM mode */
                ( (PWM_DEFAULT_FREQ - 1) * 50 ) / 100,  /* Pulse value */
                TIM_OCNPOLARITY_HIGH,                   /* Output polarity */
            },
            TIM_CHANNEL_2,                              /* Channel number */
            50                                          /* Duty cycle */
        },
        {   /* Channel 3 */
            {
                TIM_OCMODE_PWM1,                        /* Specifies the TIM mode */
                ( (PWM_DEFAULT_FREQ - 1) * 50 ) / 100,  /* Pulse value */
                TIM_OCNPOLARITY_HIGH,                   /* Output polarity */
            },
            TIM_CHANNEL_3,                              /* Channel number */
            50                                          /* Duty cycle */
        },
        {   /* Channel 4 */
            {
                TIM_OCMODE_PWM1,                        /* Specifies the TIM mode */
                ( (PWM_DEFAULT_FREQ - 1) * 50 ) / 100,  /* Pulse value */
                TIM_OCNPOLARITY_HIGH,                   /* Output polarity */
            },
            TIM_CHANNEL_4,                              /* Channel number */
            50                                          /* Duty cycle */
        }
    }
};

/**
* @brief Gets the timer handler associated to a PWM channel.
* @param void
* @retval Pointer to the timer handler.
*/
TIM_HandleTypeDef* bspPwmGetHandler(void)

{
    return &pwmConfigStruct.xTimHandle;
}

/**
* @brief Starts a PWM cannel.
* @param eChannelIdex BSP channel number
* @retval void
*/
void bspPwmStart(pwmChannels_e eChannelIndex)
{
    switch (eChannelIndex)
    {
        case  PWM_CH_1: HAL_TIM_OC_Start_IT(&pwmConfigStruct.xTimHandle, TIM_CHANNEL_1); break;
        case  PWM_CH_2: HAL_TIM_OC_Start_IT(&pwmConfigStruct.xTimHandle, TIM_CHANNEL_2); break;
        case  PWM_CH_3: HAL_TIM_OC_Start_IT(&pwmConfigStruct.xTimHandle, TIM_CHANNEL_3); break;
        case  PWM_CH_4: HAL_TIM_OC_Start_IT(&pwmConfigStruct.xTimHandle, TIM_CHANNEL_4); break;
        default: break;
    }
}

/**
* @brief Sets a new frequency
* @param uNewFreq Frequency to be set
* @retval HAL status
* @note 1 decimal value = 1Hz
*/
HAL_StatusTypeDef bspPwmSetFreq(uint32_t uNewFreq)
{
    int i;
    float period;
    HAL_StatusTypeDef status;

    if (uNewFreq < 1)
    {
        return HAL_ERROR;
    }

    period = (1 / (float)uNewFreq) * 1000000;
    pwmConfigStruct.xTimHandle.Init.Period = period;

    for( i = 0; i < PWM_MAX_CHANNELS; i++)
    {
       HAL_TIM_OC_Stop_IT(&pwmConfigStruct.xTimHandle, pwmConfigStruct.uChannelXConfig[i].channel);
       pwmConfigStruct.uChannelXConfig[i].xOcInit.Pulse = (period * pwmConfigStruct.uChannelXConfig[i].uDuty) / 100;
       HAL_TIM_PWM_ConfigChannel(&pwmConfigStruct.xTimHandle, &pwmConfigStruct.uChannelXConfig[i].xOcInit, pwmConfigStruct.uChannelXConfig[i].channel);
    }

    status = HAL_TIM_OC_Init(&pwmConfigStruct.xTimHandle);
    if (status != HAL_OK)
    {
        return HAL_ERROR;
    }

    for( i = 0; i < PWM_MAX_CHANNELS; i++)
    {
       HAL_TIM_OC_Start_IT(&pwmConfigStruct.xTimHandle, pwmConfigStruct.uChannelXConfig[i].channel);
    }

   /* Clear interrupt to not jump to the interrupt handler */
    __HAL_TIM_CLEAR_IT(&pwmConfigStruct.xTimHandle, TIM_IT_UPDATE);
    return HAL_OK;
}

/**
* @brief Sets a new duty cycle to a giving channel.
* @param uNewDuty Duty cycle to be set
* @param xChannel PWM channel
* @retval HAL status
*/
HAL_StatusTypeDef bspPwmSetDuty(uint8_t uNewDuty, pwmChannels_e xChannel)
{
    uint8_t uChannel;
    uint32_t newAutoReloadReg;

    if (xChannel >= PWM_MAX_CHANNELS)
        return HAL_ERROR;

    switch (xChannel)
    {
        case PWM_CH_1:
            uChannel = TIM_CHANNEL_1;
            pwmConfigStruct.uChannelXConfig[xChannel].uDuty = uNewDuty;
            break;
        case PWM_CH_2:
            uChannel = TIM_CHANNEL_2;
            pwmConfigStruct.uChannelXConfig[xChannel].uDuty = uNewDuty;
            break;
        case PWM_CH_3:
            uChannel = TIM_CHANNEL_3;
            pwmConfigStruct.uChannelXConfig[xChannel].uDuty = uNewDuty;
            break;
        case PWM_CH_4:
            uChannel = TIM_CHANNEL_4;
            pwmConfigStruct.uChannelXConfig[xChannel].uDuty = uNewDuty;
            break;
        default:  break;
    }

    newAutoReloadReg =  (__HAL_TIM_GET_AUTORELOAD(&pwmConfigStruct.xTimHandle) * uNewDuty) / 100;
    __HAL_TIM_SET_COMPARE(&pwmConfigStruct.xTimHandle, uChannel, newAutoReloadReg);

    return HAL_OK;
}

/**
* @brief Initialize the timer init and the PWM channel.
* @param void
* @retval HAL status
*/
HAL_StatusTypeDef bspPwmInit(void)
{
    int i;
    HAL_StatusTypeDef halStatus;

    halStatus = HAL_TIM_OC_Init(&pwmConfigStruct.xTimHandle);
    if (halStatus != HAL_OK)
    {
        return HAL_ERROR;
    }

     for( i = 0; i < PWM_MAX_CHANNELS; i++)
     {
        halStatus = HAL_TIM_PWM_ConfigChannel(&pwmConfigStruct.xTimHandle, &pwmConfigStruct.uChannelXConfig[i].xOcInit,
                                              pwmConfigStruct.uChannelXConfig[i].channel);
        if (halStatus != HAL_OK)
        {
            return HAL_ERROR;
        }
     }

    __HAL_TIM_CLEAR_IT(&pwmConfigStruct.xTimHandle, TIM_IT_UPDATE);
    return HAL_OK;
}

/**
  ******************************************************************************
  * @file    bspGpio.h
  * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
  * @brief   Header file that exposes GPIO data types and GPIo APIs
  ******************************************************************************
*/

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "stdint.h"
#include "stm32f4xx_hal.h"

typedef enum
{
    BSP_GPIOA,
    BSP_GPIOB,
    BSP_GPIOC,
    BSP_GPIOD,
    BSP_GPIOE,
    BSP_GPIOH,
    BSP_MAX_GPIO_INSTANCE,
}BspGpioInstance_e;

typedef enum
{
    BSP_GPIO_PIN_0,
    BSP_GPIO_PIN_1,
    BSP_GPIO_PIN_2,
    BSP_GPIO_PIN_3,
    BSP_GPIO_PIN_4,
    BSP_GPIO_PIN_5,
    BSP_GPIO_PIN_6,
    BSP_GPIO_PIN_7,
    BSP_GPIO_PIN_8,
    BSP_GPIO_PIN_9,
    BSP_GPIO_PIN_10,
    BSP_GPIO_PIN_11,
    BSP_GPIO_PIN_12,
    BSP_GPIO_PIN_13,
    BSP_GPIO_PIN_14,
    BSP_GPIO_PIN_15,
}BspPinNum_e;

typedef enum
{
    BSP_GPIO_PIN_LOW,
    BSP_GPIO_PIN_HIGH,
}BspGpioPinState_e;

void bspGpioToggle(BspGpioInstance_e eGpio, BspPinNum_e pinNum);
BspGpioInstance_e bspGpioMapInstance(const char pcGpioInstance);
BspGpioPinState_e bspGpioRead(BspGpioInstance_e eGpio, BspPinNum_e pinNum);
void bspGpioWrite(BspGpioInstance_e eGpio, BspPinNum_e pinNum, BspGpioPinState_e pinState);

#endif

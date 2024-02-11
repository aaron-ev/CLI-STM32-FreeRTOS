/**
 ******************************************************************************
 * @file    bspGpio.c
 * @author  Aaron Escoboza, Github account: https://github.com/aaron-ev
 * @brief   source file to implement functions related to GPIO operations.
 ******************************************************************************
 */

#include "bspGpio.h"

/**
* @brief Maps a number from the range 0 - 15 to STM32 pin macro value.
* @param uGpioNumber Pin number.
* @retval STM32 HAL pin number.
*/
static uint16_t bspMapPinNumFromBspToHal(uint16_t uGpioNumber)
{
    switch (uGpioNumber)
    {
        case  0: return GPIO_PIN_0;
        case  1: return GPIO_PIN_1;
        case  2: return GPIO_PIN_2;
        case  3: return GPIO_PIN_3;
        case  4: return GPIO_PIN_4;
        case  5: return GPIO_PIN_5;
        case  6: return GPIO_PIN_6;
        case  7: return GPIO_PIN_7;
        case  8: return GPIO_PIN_8;
        case  9: return GPIO_PIN_9;
        case 10: return GPIO_PIN_10;
        case 11: return GPIO_PIN_11;
        case 12: return GPIO_PIN_12;
        case 13: return GPIO_PIN_13;
        case 14: return GPIO_PIN_14;
        case 15: return GPIO_PIN_15;
        default: return 0; /* Invalid GPIO pin number */
    }
}

/**
* @brief Maps a letter (a, b, c, d, e , h) to a BSP GPIO instance.
* @param pcGpioInstance GPIO instance character.
* @retval BSP GPIO instance.
*/
BspGpioInstance_e bspGpioMapInstance(const char pcGpioInstance)
{
    if (pcGpioInstance == 'a' || pcGpioInstance == 'A')
    {
        return BSP_GPIOA;
    }
    else if (pcGpioInstance == 'b' || pcGpioInstance == 'B')
    {
        return BSP_GPIOB;
    }
    else if (pcGpioInstance == 'c' || pcGpioInstance == 'C')
    {
        return BSP_GPIOC;
    }
    else if (pcGpioInstance == 'd' || pcGpioInstance == 'D')
    {
        return BSP_GPIOD;
    }
    else if (pcGpioInstance == 'e' || pcGpioInstance == 'E')
    {
        return BSP_GPIOE;
    }
    else if (pcGpioInstance == 'h' || pcGpioInstance == 'H')
    {
        return BSP_GPIOH;
    }
    else
    {
        return BSP_MAX_GPIO_INSTANCE;
    }
}

/**
* @brief Maps from a BSP instance to HAL GPIO instance pointer.
* @param eGpio BSP GPIO instance.
* @retval HAL GPIO instance pointer.
*/
static GPIO_TypeDef* bspMapInstanceToHal(BspGpioInstance_e eGpio)
{
    switch (eGpio)
    {
        case BSP_GPIOA: return GPIOA;
        case BSP_GPIOB: return GPIOB;
        case BSP_GPIOC: return GPIOC;
        case BSP_GPIOD: return GPIOD;
        case BSP_GPIOE: return GPIOE;
        case BSP_GPIOH: return GPIOH;
        default: return NULL;
    }
}

/**
* @brief Toggles a GPIO pin.
* @param eGpio BSP GPIO instance.
* @param pinNum BSP GPIO pin number.
* @retval void
*/
void bspGpioToggle(BspGpioInstance_e eGpio, BspPinNum_e pinNum)
{
    uint32_t halPinNum = bspMapPinNumFromBspToHal(pinNum);
    GPIO_TypeDef* halGpioInstance = bspMapInstanceToHal(eGpio);

    if (eGpio < BSP_MAX_GPIO_INSTANCE)
    {
        HAL_GPIO_TogglePin(halGpioInstance, halPinNum);
    }
}

/**
* @brief Writes to a GPIO pin.
* @param eGpio BSP GPIO instance.
* @param pinNum BSP GPIO pin number.
* @param pinState new BSP pin state.
* @retval void
*/
void bspGpioWrite(BspGpioInstance_e eGpio, BspPinNum_e pinNum, BspGpioPinState_e pinState)
{
    uint32_t halPinNum = bspMapPinNumFromBspToHal(pinNum);
    GPIO_TypeDef* halGpioInstance = bspMapInstanceToHal(eGpio);

    if (eGpio < BSP_MAX_GPIO_INSTANCE)
    {
        HAL_GPIO_WritePin(halGpioInstance, halPinNum, pinState);
    }
}

/**
* @brief Reads from a GPIO pin.
* @param eGpio BSP GPIO instance.
* @param pinNum BSP GPIO pin number.
* @retval BSP pin state
*/
BspGpioPinState_e bspGpioRead(BspGpioInstance_e eGpio, BspPinNum_e pinNum)
{
    uint32_t halPinNum = bspMapPinNumFromBspToHal(pinNum);
    GPIO_TypeDef* halGpioInstance = bspMapInstanceToHal(eGpio);

    return (HAL_GPIO_ReadPin(halGpioInstance, halPinNum) == GPIO_PIN_SET) ?
            BSP_GPIO_PIN_HIGH : BSP_GPIO_PIN_LOW;
}

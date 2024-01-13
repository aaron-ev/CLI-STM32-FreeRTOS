/**
  ******************************************************************************
  * @file    appConfig.h
  * @author  Aaron Escoboza
  * @brief   Hold general application configuration
  ******************************************************************************
*/

#ifndef APP_CONFIG__H
#define APP_CONFIG__H

/* Enable debug messages */
#define DEBUG_PRINT_EN                      1 /* 1 = Enable , 0 =  Disable */

/* Task priorities */
#define HEART_BEAT_PRIORITY_TASK            1

/* Heart beat settings */
#define HEART_BEAT_LED_PORT                 GPIOC
#define HEART_BEAT_LED_PIN                  GPIO_PIN_13
#define HEART_BEAT_BLINK_DELAY              500 /* In ms */

/* CLI console settings */
#define CONSOLE_INSTANCE                    USART1
#define CONSOLE_TX_PIN                      GPIO_PIN_6
#define CONSOLE_RX_PIN                      GPIO_PIN_7
#define CONSOLE_GPIO_PORT                   GPIOB
#define CONSOLE_BAUDRATE                    9600
#define CONSOLE_TASK_PRIORITY               1 

#endif

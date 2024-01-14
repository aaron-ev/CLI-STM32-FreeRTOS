
/*
TODO:
      * Guard UART access with a mutex.
      * Handle data from ISR with a callback or similar -- completed
      * Investigate ASICII codes for backspace and handle them -- completed
      * Define better sizes for buffer.  
*/

#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32f4xx_hal.h"

#define MAX_IN_STR_LEN                          300
#define MAX_OUT_STR_LEN                         300
#define MAX_RX_QUEUE_LEN                        300

extern void vConsoleRegisterCommands(void);
extern UART_HandleTypeDef consoleHandle;

char cRxData;
QueueHandle_t xQueueRxHandle;
static const uint8_t *const pcWelcomeMsg = "Welcome to the console. Enter 'help' to view a list of available commands.\n";

/*
    Description: Read from a queue until there is a character in RX hardware
    buffer. 
*/
static BaseType_t xConsoleRead(uint8_t *cReadChar, size_t xLen)
{
    BaseType_t xRetVal = pdFALSE;

    if (xQueueRxHandle == NULL || cReadChar == NULL)
    {
        return xRetVal;
    }

    /* Block until the there is input from the user */
    xRetVal = xQueueReceive(xQueueRxHandle, cReadChar, portMAX_DELAY);
    return xRetVal;
}

/*
    Description: Write to TX device. 
*/
static HAL_StatusTypeDef vConsoleWrite(uint8_t *const buff, size_t len)
{
	HAL_StatusTypeDef status;

    /* No preprocessing needed, write directly to the hardware */
    status = HAL_UART_Transmit(&consoleHandle, buff, len, portMAX_DELAY);
    if ( status != HAL_OK)
    {
    	return HAL_ERROR;
    }
    return status;
}

void vTaskConsole(void *pvParams)
{
    char cReadCh;
    char cInputIndex = 0;
    BaseType_t xMoreDataToProcess;
    char pcInputString[MAX_IN_STR_LEN];
    char pcOutputString[MAX_OUT_STR_LEN];

    memset(pcInputString, 0x00, MAX_IN_STR_LEN);
    memset(pcOutputString, 0x00, MAX_OUT_STR_LEN);

    /* Create a queue to store characters from RX ISR */
    xQueueRxHandle = xQueueCreate(MAX_RX_QUEUE_LEN, sizeof(char));
    if (xQueueRxHandle == NULL)
    {
        goto fail_task_console;
    }


    /* Send a welcome message to the user */
    vConsoleWrite(pcWelcomeMsg, strlen(pcWelcomeMsg));

    /* Read UART RX (no blocking call) */
    HAL_UART_Receive_IT(&consoleHandle, &cRxData, 1);
    vConsoleWrite("\nCommand: ", strlen("\nCommand: "));

    while(1)
    {

        /* Block until there is a new character in RX buffer */
        xConsoleRead(&cReadCh, sizeof(cReadCh));

        /* Echo any user character */
        vConsoleWrite(&cRxData, 1);

        /* New line character received, command received correctly */
        if (cReadCh == '\n')
        {
              if (cInputIndex == 0)
              {
                  /* User only typed enter*/
                  vConsoleWrite("Command: ", strlen("Command: "));
                  continue;
              }
            do
            {
                xMoreDataToProcess = FreeRTOS_CLIProcessCommand
                                     (
                                        pcInputString,    /* Command string*/
                                        pcOutputString,   /* Output buffer */
                                        MAX_OUT_STR_LEN   /* Output buffer size */
                                     );
                vConsoleWrite(pcOutputString, sizeof(pcOutputString));
            } while(xMoreDataToProcess != pdFALSE);

            /* Reset input buffer for next command */
            cInputIndex = 0;
            memset(pcInputString, 0x00, MAX_IN_STR_LEN);
            memset(pcOutputString, 0x00, MAX_OUT_STR_LEN);
            vConsoleWrite("\nCommand: ", strlen("\nCommand: "));
        }
        else
        {
            if (cReadCh == '\r')
            {
                /* Ignore carriage returns, do nothing */
            }
            else if (cReadCh == '\b') /* Backspace case */
            {
                /* Erase last character in the input buffer */
                if (cInputIndex > 0)
                {
                    cInputIndex--;
                    pcInputString[cInputIndex] = '\0';
                }
                else
                {
                    vConsoleWrite(" ", strlen(" "));
                }
            }
            else
            {
                if (cInputIndex < MAX_IN_STR_LEN)
                {
                    pcInputString[cInputIndex] = cReadCh;
                    cInputIndex++;
                }
            }
        }
    }

fail_task_console:
    if (xQueueRxHandle)
    {
        vQueueDelete(xQueueRxHandle);
    }
    vTaskDelete(NULL);
}

BaseType_t xConsoleInit(uint16_t usStackSize, UBaseType_t uxPriority)
{
    /* Register all commands that can be accessed by the user */
    vConsoleRegisterCommands();
    return xTaskCreate(vTaskConsole,"CLI", usStackSize, NULL, uxPriority, NULL);
}

/*
    Description: Callback for UART RX event. 
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t pxHigherPriorityTaskWoken = pdFALSE;

    if (xQueueRxHandle != NULL)
    {
        xQueueSendToBackFromISR(xQueueRxHandle, &cRxData, &pxHigherPriorityTaskWoken);
    }
    HAL_UART_Receive_IT(&consoleHandle, &cRxData, 1);
}

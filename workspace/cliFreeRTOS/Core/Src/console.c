
/**
 ******************************************************************************
 * @file         console.c
 * @author       Aaron Escoboza
 * @brief        Command Line Interpreter based on FreeRTOS and STM32 HAL layer
 *               Github account: https://github.com/aaron-ev
 ******************************************************************************
 */

#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "queue.h"
#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MAX_IN_STR_LEN                          300
#define MAX_OUT_STR_LEN                         300
#define MAX_RX_QUEUE_LEN                        300

UART_HandleTypeDef *pxUartDevHandle;

char cRxData;
QueueHandle_t xQueueRxHandle;
static const char *pcWelcomeMsg = "Welcome to the console. Enter 'help' to view a list of available commands.\n";

static const char *prvpcTaskListHeader = "Task states: BL = Blocked RE = Ready DE = Deleted  SU = Suspended\n\n"\
                                         "Task name                         State  Priority  Stack remaining  %%CPU usage  Runtime\n"\
                                         "================================  =====  ========  ===============  ===========  =======\n";
/* Private available commands */
static BaseType_t prvCommandPwmSetFreq(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandPwmSetDuty(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandGpioWrite(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandGpioRead( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandEcho( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandTaskStats( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandHeap(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandClk(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

static const char *prvpcMapTaskState(eTaskState eState)
{
    switch (eState)
    {
        case eReady:
            return "RU";
        case eRunning:
            return "RE";
        case eDeleted:
            return "DE";
        case eSuspended:
            return "SU";
        case eBlocked:
            return "Bl";
        default:
            return "??";
    }
}

static GPIO_TypeDef *xMapGpioInstances(const char pcGpioInstance)
{
    if (pcGpioInstance == 'a' || pcGpioInstance == 'A')
    {
        return GPIOA;
    }
    else if (pcGpioInstance == 'b' || pcGpioInstance == 'B')
    {
        return GPIOB;
    }
    else if (pcGpioInstance == 'c' || pcGpioInstance == 'C')
    {
        return GPIOC;
    }
    else if (pcGpioInstance == 'd' || pcGpioInstance == 'D')
    {
        return GPIOD;
    }
    else if (pcGpioInstance == 'e' || pcGpioInstance == 'E')
    {
        return GPIOE;
    }
    else if (pcGpioInstance == 'h' || pcGpioInstance == 'H')
    {
        return GPIOH;
    }
    else 
    {
        return NULL;
    }
}

static const uint16_t uMapGpioNumber(uint16_t uGpioNumber)
{
    switch (uGpioNumber)
    {
        case 0: return GPIO_PIN_0;
        case 1: return GPIO_PIN_1;
        case 2: return GPIO_PIN_2;
        case 3: return GPIO_PIN_3;
        case 4: return GPIO_PIN_4;
        case 5: return GPIO_PIN_5;
        case 6: return GPIO_PIN_6;
        case 7: return GPIO_PIN_7;
        case 8: return GPIO_PIN_8;
        case 9: return GPIO_PIN_9;
        case 10: return GPIO_PIN_10;
        case 11: return GPIO_PIN_11;
        case 12: return GPIO_PIN_12;
        case 13: return GPIO_PIN_13;
        case 14: return GPIO_PIN_14;
        case 15: return GPIO_PIN_15;
        default: return 0; /* Invalid GPIO pin number */
    }
}

static const CLI_Command_Definition_t xCommands[] = 
{
    {
        "task-stats",
        "\r\ntask-stats:\r\n Displays a table with the state of each FreeRTOS task\r\n",
        prvCommandTaskStats,
        0 /* 0 parameters expected */
    },
    {
        "gpio-w",
        "\r\ngpio-w [gpio port] [pin number] [logical value]: Write a digital value to a port pin, example: gpio-w a 2 0 --> write logical zero to pin number 2 of GPIO port a\r\n",
        prvCommandGpioWrite,
        3 /* Parameters: [GPIO PORT] [GPIO pin number] [Logical value (1, 0)] */
    },
    {
        "gpio-r",
        "\r\ngpio-r [gpio port] [pin number] : Read logical level of a GPIO pin, example: gpio-r a 2 --> read GPIOA pin number 2\r\n",
        prvCommandGpioRead,
        2
    },
    {
       "echo",
       "\r\necho [string to echo]\r\n",
       prvCommandEcho,
       1
    },
    {
        "pwm-f",
        "\r\npwm-f [pwmChannel] [new frequency]: Update PWM frequency of a giving channel \r\n",
        prvCommandPwmSetFreq,
        2
    },
    {
        "pwm-d",
        "\r\npwmSetDuty [pwmChannel] [new duty cycle]: Update PWM duty cycle of a giving channel \r\n",
        prvCommandPwmSetDuty,
        2
    },
    {
        "heap",
        "\r\nheap: Display free heap memory\r\n",
        prvCommandHeap,
        0
    },
    {
        "clk",
        "\r\nclk: Display clock information\r\n",
        prvCommandClk,
        0
    },
    {
        NULL, 
        NULL,
        NULL,
        0
    }
};

static BaseType_t prvCommandTaskStats( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    static uint32_t uTaskIndex = 0;
    static uint32_t uTotalOfTasks = 0;
    static uint32_t uTotalRunTime = 1;
    static TaskStatus_t *pxTaskStatus = NULL; 
    TaskStatus_t *pxTmpTaskStatus = NULL;

    if (pxTaskStatus == NULL)
    {
        uTotalOfTasks = uxTaskGetNumberOfTasks();
        pxTaskStatus = pvPortMalloc(uTotalOfTasks * sizeof(TaskStatus_t));
        if (pxTaskStatus == NULL)
        {
           snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Not enough memory for task allocation"); 
           goto out_cmd_task_stats;
        }
        uTotalOfTasks = uxTaskGetSystemState(pxTaskStatus, uTotalOfTasks, &uTotalRunTime);
        uTaskIndex = 0;
        uTotalRunTime /= 100;
        snprintf(pcWriteBuffer, xWriteBufferLen, prvpcTaskListHeader);
    }
    else
    {
    	memset(pcWriteBuffer, 0x00, 300);
        /* Prevent from zero division */
        if (uTotalRunTime == 0)
            uTotalRunTime = 1;

        pxTmpTaskStatus = &pxTaskStatus[uTaskIndex];
        snprintf(pcWriteBuffer, xWriteBufferLen, 
                "%-32s  %5s  %8lu  %15d  %11lu  %7lu\n",
                pxTmpTaskStatus->pcTaskName, 
                prvpcMapTaskState(pxTmpTaskStatus->eCurrentState),
                pxTmpTaskStatus->uxCurrentPriority,
                pxTmpTaskStatus->usStackHighWaterMark,
                pxTmpTaskStatus->ulRunTimeCounter / uTotalRunTime,
                pxTmpTaskStatus->ulRunTimeCounter);
        uTaskIndex++;
    }

    /* Check if there is more tasks to be process */
    if (uTaskIndex < uTotalOfTasks)
       return pdTRUE;
    else 
    {
out_cmd_task_stats :
        if (pxTaskStatus != NULL)
        {
            vPortFree(pxTaskStatus);
            pxTaskStatus = NULL;
        }
        return pdFALSE;
    }
}

static BaseType_t prvCommandGpioWrite(char *pcWriteBuffer, size_t xWriteBufferLen,\
                                      const char *pcCommandString)
{
    BaseType_t xParamLen;
    const char * pcParam;
    char cGpioPort; 
    GPIO_TypeDef *xGpioInstance;
    GPIO_PinState xNewPinState;
    uint16_t uPinNumber;

    /* Get GPIO instance, pin number and new pin state specified by the user */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    cGpioPort = *pcParam;
    xGpioInstance = xMapGpioInstances(cGpioPort);
    if (xGpioInstance == NULL)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid GPIO port\n"); 
        goto out_cmd_gpio_write;
    }

    /* Get pin number */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParamLen);
    uPinNumber = uMapGpioNumber(atoi(pcParam));
    if (uPinNumber < GPIO_PIN_0 || uPinNumber > GPIO_PIN_15)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid pin number\n"); 
        goto out_cmd_gpio_write;
    }

    /* Get new pin state */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParamLen);
    xNewPinState = atoi(pcParam);
    if (xNewPinState != GPIO_PIN_SET && xNewPinState != GPIO_PIN_RESET)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid pin state\n"); 
        goto out_cmd_gpio_write;
    }

    HAL_GPIO_WritePin(xGpioInstance, uPinNumber, xNewPinState);
    snprintf(pcWriteBuffer, xWriteBufferLen, "Pin set to %d\n", xNewPinState);

out_cmd_gpio_write:
    return pdFALSE;
}

static BaseType_t prvCommandGpioRead(char *pcWriteBuffer, size_t xWriteBufferLen\
                                     , const char *pcCommandString)
{
    BaseType_t xParamLen;
    const char * pcParam;
    GPIO_TypeDef *xGpioInstance;
    GPIO_PinState xPinState;
    uint16_t uPinNumber;

    /* Get GPIO instance, pin number and new pin state specified by the user */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    xGpioInstance = xMapGpioInstances(*pcParam);
    if (xGpioInstance == NULL)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid GPIO port\n"); 
        goto out_cmd_gpio_read;
    }

    /* Get pin number */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParamLen);
    uPinNumber = uMapGpioNumber(atoi(pcParam));
    if (uPinNumber < GPIO_PIN_0 || uPinNumber > GPIO_PIN_15)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid pin number\n"); 
        goto out_cmd_gpio_read;
    }

    xPinState = HAL_GPIO_ReadPin(xGpioInstance, uPinNumber);
    snprintf(pcWriteBuffer, xWriteBufferLen, "Pin state: %d\n", xPinState); 

out_cmd_gpio_read:
    return pdFALSE;
}

static BaseType_t prvCommandEcho( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    const char *pcStrToOutput;
    BaseType_t xParamLen;
    pcStrToOutput = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    snprintf(pcWriteBuffer, xWriteBufferLen, pcStrToOutput); 
    return pdFALSE;
}

static BaseType_t prvCommandPwmSetFreq(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	return pdFALSE;
}
static BaseType_t prvCommandPwmSetDuty(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	return pdFALSE;
}

static BaseType_t prvCommandHeap(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{

    size_t xHeapFree;
    size_t xHeapMinMemExisted; 

    xHeapFree = xPortGetFreeHeapSize();
    xHeapMinMemExisted = xPortGetMinimumEverFreeHeapSize();

    snprintf(pcWriteBuffer, xWriteBufferLen, 
             "Heap size            : %3u bytes (%3d KiB)\nRemaining            : %3u bytes (%3d KiB)\nMinimum ever existed : %3u bytes (%3d KiB)\n",
             configTOTAL_HEAP_SIZE, configTOTAL_HEAP_SIZE / 1024, xHeapFree, xHeapFree / 1024, xHeapMinMemExisted, xHeapMinMemExisted / 1024);

    return pdFALSE;
}

static BaseType_t prvCommandClk(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    uint32_t uSysClock;
    SystemCoreClockUpdate();
    uSysClock = HAL_RCC_GetHCLKFreq();
    snprintf(pcWriteBuffer, xWriteBufferLen, "System clock: %lu Hz (%3lu kHz) (%3lu MHz)\n", uSysClock, uSysClock / 1000, uSysClock / 1000000);
    return pdFALSE;
}


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
static HAL_StatusTypeDef vConsoleWrite(const char *buff, size_t len)
{
	HAL_StatusTypeDef status;

    /* No preprocessing needed, write directly to the hardware */
    if (pxUartDevHandle == NULL)
    {
        return HAL_ERROR;
    }
    status = HAL_UART_Transmit(pxUartDevHandle, (uint8_t *)buff, len, portMAX_DELAY);
    if ( status != HAL_OK)
    {
    	return HAL_ERROR;
    }
    return status;
}


void vConsoleEnableRxInterrupt(void)
{
    if (pxUartDevHandle == NULL)
    {
        return;
    }
    HAL_UART_Receive_IT(pxUartDevHandle,(uint8_t*)&cRxData, 1);
}

void vTaskConsole(void *pvParams)
{
    uint8_t cReadCh;
    uint8_t uInputIndex = 0;
    BaseType_t xMoreDataToProcess;
    char pcInputString[MAX_IN_STR_LEN];
    char pcOutputString[MAX_OUT_STR_LEN];

    memset(pcInputString, 0x00, MAX_IN_STR_LEN);
    memset(pcOutputString, 0x00, MAX_OUT_STR_LEN);

    /* Create a queue to store characters from RX ISR */
    xQueueRxHandle = xQueueCreate(MAX_RX_QUEUE_LEN, sizeof(char));
    if (xQueueRxHandle == NULL)
    {
        goto out_task_console;
    }


    /* Send a welcome message to the user */
    vConsoleWrite(pcWelcomeMsg, strlen(pcWelcomeMsg));

    vConsoleEnableRxInterrupt();
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
              if (uInputIndex == 0)
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
            uInputIndex = 0;
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
                if (uInputIndex > 0)
                {
                    uInputIndex--;
                    pcInputString[uInputIndex] = '\0';
                }
                else
                {
                    vConsoleWrite(" ", strlen(" "));
                }
            }
            else
            {
                if (uInputIndex < MAX_IN_STR_LEN)
                {
                    pcInputString[uInputIndex] = cReadCh;
                    uInputIndex++;
                }
            }
        }
    }

out_task_console:
    if (xQueueRxHandle)
    {
        vQueueDelete(xQueueRxHandle);
    }
    vTaskDelete(NULL);
}

BaseType_t xConsoleInit(uint16_t usStackSize, UBaseType_t uxPriority, UART_HandleTypeDef *pxUartHandle)
{
    const CLI_Command_Definition_t *pCommand; 
    
    if (pxUartHandle == NULL)
    {
        return pdFALSE;
    }
    pxUartDevHandle =  pxUartHandle;

    /* Register all commands that can be accessed by the user */
    for (pCommand = xCommands; pCommand->pcCommand != NULL; pCommand++)
    {
        FreeRTOS_CLIRegisterCommand(pCommand);
    }
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
    vConsoleEnableRxInterrupt();
}

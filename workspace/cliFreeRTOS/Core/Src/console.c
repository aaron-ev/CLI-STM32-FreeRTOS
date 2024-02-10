
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
#include "bsp.h"

#define MAX_IN_STR_LEN                          300
#define MAX_OUT_STR_LEN                         600
#define MAX_RX_QUEUE_LEN                        300

                                                      /* ASCII codes          */
#define ASCII_TAB                               '\t'  /* Tabulate             */
#define ASCII_CR                                '\r'  /* Carriage return      */
#define ASCII_LF                                '\n'  /* Line feed            */
#define ASCII_BACKSPACE                         '\b'  /* Back space           */
#define ASCII_FORM_FEED                         '\f'  /* Form feed            */
#define ASCII_DEL                               127   /* Delete               */
#define ASCII_CTRL_PLUS_C                         3   /* CTRL + C             */
#define ASCII_NACK                               21   /* Negative acknowledge */

char cRxData;
QueueHandle_t xQueueRxHandle;
UART_HandleTypeDef *pxUartDevHandle;
static const char *pcWelcomeMsg = "Welcome to the console. Enter 'help' to view a list of available commands.\n";

static const char *prvpcTaskListHeader = "Task states: Bl = Blocked, Re = Ready, Ru = Running, De = Deleted,  Su = Suspended\n\n"\
                                         "Task name         State  Priority  Stack remaining  CPU usage  Runtime(us)\n"\
                                         "================= =====  ========  ===============  =========  ===========\n";
static const char *prvpcPrompt = "#cmd: ";

static BaseType_t prvCommandPwmSetFreq(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandPwmSetDuty(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandGpioWrite(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandGpioRead( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandEcho( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandTaskStats( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandHeap(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandClk(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandTicks(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/**
*   @brief  This function is executed in case of error occurrence.
*   @retval None
*/
static const char *prvpcMapTaskState(eTaskState eState)
{
    switch (eState)
    {
        case     eReady: return "Re";
        case   eRunning: return "Ru";
        case   eDeleted: return "De";
        case   eBlocked: return "Bl";
        case eSuspended: return "S";
        default: return "??";
    }
}

static const CLI_Command_Definition_t xCommands[] =
{
    {
        "task-stats",
        "\r\ntask-stats:\r\n Displays a table with the state of each FreeRTOS task.\r\n",
        prvCommandTaskStats,
        0
    },
    {
        "gpio-w",
        "\r\ngpio-w [gpio port] [pin number] [logical value]: Write a digital value to GPIOx pin.\r\n",
        prvCommandGpioWrite,
        3
    },
    {
        "gpio-r",
        "\r\ngpio-r [gpio port] [pin number] : Read a GPIO pin.\r\n",
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
        "\r\npwm-f [Frequency]: Set a new frequency.\r\n",
        prvCommandPwmSetFreq,
        1
    },
    {
        "pwm-d",
        "\r\npwm-d [Duty cycle] [Channel]: Set a new PWM duty cycle of a giving channel.\r\n",
        prvCommandPwmSetDuty,
        2
    },
    {
        "heap",
        "\r\nheap: Display free heap memory.\r\n",
        prvCommandHeap,
        0
    },
    {
        "clk",
        "\r\nclk: Display clock information.\r\n",
        prvCommandClk,
        0
    },
    {
        "ticks",
        "\r\nticks: Display OS tick count and run time in seconds.\r\n",
        prvCommandTicks,
        0
    },
    {
        NULL,
        NULL,
        NULL,
        0
    }
};

/**
* @brief Command that gets task statistics.
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandTaskStats( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    static uint32_t uTaskIndex = 0;
    static uint32_t uTotalOfTasks = 0;
    static uint32_t uTotalRunTime = 1;
    TaskStatus_t *pxTmpTaskStatus = NULL;
    static TaskStatus_t *pxTaskStatus = NULL;

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
        memset(pcWriteBuffer, 0x00, MAX_OUT_STR_LEN);
        /* Prevent from zero division */
        if (uTotalRunTime == 0)
            uTotalRunTime = 1;

        pxTmpTaskStatus = &pxTaskStatus[uTaskIndex];
        if (pxTmpTaskStatus->ulRunTimeCounter / uTotalRunTime < 1)
        {
         snprintf(pcWriteBuffer, xWriteBufferLen,
                 "%-16s  %5s  %8lu  %14dB       < 1%%  %11lu\n",
                 pxTmpTaskStatus->pcTaskName,
                 prvpcMapTaskState(pxTmpTaskStatus->eCurrentState),
                 pxTmpTaskStatus->uxCurrentPriority,
                 pxTmpTaskStatus->usStackHighWaterMark,
                 pxTmpTaskStatus->ulRunTimeCounter);
        }
        else
        {
            snprintf(pcWriteBuffer, xWriteBufferLen,
                    "%-16s  %5s  %8lu  %14dB  %8lu%%  %11lu\n",
                    pxTmpTaskStatus->pcTaskName,
                    prvpcMapTaskState(pxTmpTaskStatus->eCurrentState),
                    pxTmpTaskStatus->uxCurrentPriority,
                    pxTmpTaskStatus->usStackHighWaterMark,
                    pxTmpTaskStatus->ulRunTimeCounter / uTotalRunTime,
                    pxTmpTaskStatus->ulRunTimeCounter);
        }
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

/**
* @brief Command that writes to a GPIOx pin.
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandGpioWrite(char *pcWriteBuffer, size_t xWriteBufferLen,\
                                      const char *pcCommandString)
{
    BaseType_t xParamLen;
    BspPinNum_e bspPinNum;
    const char * pcGpioPinNum;
    const char * pcGpioInstance;
    const char * pcGpioPinSate;
    BspGpioPinState_e bspPinState;
    BspGpioInstance_e bspGpioInstance;

    /* Get GPIO instance, pin number and new pin state specified by the user */
    pcGpioInstance = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    bspGpioInstance = bspGpioMapInstance(*pcGpioInstance);
    if (bspGpioInstance >= BSP_MAX_GPIO_INSTANCE)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid GPIO port\n");
        goto out_cmd_gpio_write;
    }

    /* Get pin number */
    pcGpioPinNum = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParamLen);
    bspPinNum = atoi(pcGpioPinNum);
    if (bspPinNum < BSP_GPIO_PIN_0 || bspPinNum > BSP_GPIO_PIN_15)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid pin number\n");
        goto out_cmd_gpio_write;
    }

    /* Get new pin state */
    pcGpioPinSate = FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParamLen);
    bspPinState = atoi(pcGpioPinSate);
    if (bspPinState > BSP_GPIO_PIN_HIGH)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid pin state\n");
        goto out_cmd_gpio_write;
    }

    bspGpioWrite(bspGpioInstance, bspPinNum, bspPinState);
    snprintf(pcWriteBuffer, xWriteBufferLen, "GPIO: %c, Pin: %c set to %d\n",
             *pcGpioInstance, *pcGpioPinNum, bspPinState);

out_cmd_gpio_write:
    return pdFALSE;
}

/**
* @brief Command that reads from GPIOx Pin
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandGpioRead(char *pcWriteBuffer, size_t xWriteBufferLen\
                                     , const char *pcCommandString)
{
    BaseType_t xParamLen;
    const char * pcGpioPinNum;
    const char * pcGpioInstance;
    BspPinNum_e bspPinNum;
    BspGpioPinState_e xPinState;
    BspGpioInstance_e bspGpioInstance;

    /* Get GPIO instance, pin number and new pin state specified by the user */
    pcGpioInstance = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    bspGpioInstance = bspGpioMapInstance(*pcGpioInstance);
    if (bspGpioInstance >= BSP_MAX_GPIO_INSTANCE)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid GPIO port\n");
        goto out_cmd_gpio_read;
    }

    /* Get pin number */
    pcGpioPinNum = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParamLen);
    bspPinNum = atoi(pcGpioPinNum);
    if (bspPinNum < BSP_GPIO_PIN_0 || bspPinNum > BSP_GPIO_PIN_15)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: invalid pin number\n");
        goto out_cmd_gpio_read;
    }

    xPinState = bspGpioRead(bspGpioInstance, bspPinNum);
    snprintf(pcWriteBuffer, xWriteBufferLen, "GPIO: %c Pin: %c state: %d\n",
            *pcGpioInstance, *pcGpioPinNum, xPinState);

out_cmd_gpio_read:
    return pdFALSE;
}

/**
* @brief Echo command line in UNIX systems.
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandEcho( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    const char *pcStrToOutput;
    BaseType_t xParamLen;

    pcStrToOutput = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    snprintf(pcWriteBuffer, xWriteBufferLen, pcStrToOutput);

    return pdFALSE;
}

/**
* @brief Command that sets a new pwm frequency.
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandPwmSetFreq(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    const char * pcFreq;
    BaseType_t xParamLen;

    pcFreq = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    bspPwmSetFreq(atoi(pcFreq));
    snprintf(pcWriteBuffer, xWriteBufferLen, "Frequency set to %dHz\n", atoi(pcFreq));

    return pdFALSE;
}

/**
* @brief Command that sets a new pwm duty cycle.
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandPwmSetDuty(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    const char * pcDutyCycle;
    const char * pcChannel;
    BaseType_t xParamLen;

    pcDutyCycle = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    pcChannel = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParamLen);

    /* Index starts at index 0, so 1 is subtracted from channel */
    bspPwmSetDuty(atoi(pcDutyCycle), atoi(pcChannel) - 1);
    snprintf(pcWriteBuffer, xWriteBufferLen, "Channel %d set to %d%% duty cycle \n", atoi(pcChannel), atoi(pcDutyCycle));

    return pdFALSE;
}

/**
* @brief Command that gets heap information
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
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

/**
* @brief Command that gets clock information from current clock settings.
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandClk(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    bspGetClockIinfo(pcWriteBuffer, xWriteBufferLen);
    return pdFALSE;
}

/**
* @brief Command that calculate OS ticks information.
* @param *pcWriteBuffer FreeRTOS CLI write buffer.
* @param xWriteBufferLen Length of write buffer.
* @param *pcCommandString pointer to the command name.
* @retval HAL status
*/
static BaseType_t prvCommandTicks(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    uint32_t uMs;
    uint32_t uSec;
    TickType_t xTickCount = xTaskGetTickCount();

    uSec = xTickCount / configTICK_RATE_HZ;
    uMs = xTickCount % configTICK_RATE_HZ;
    snprintf(pcWriteBuffer, xWriteBufferLen,
             "\nTick rate: %u Hz\nTicks: %u\nRun time: %u.%.3u seconds\n",
              configTICK_RATE_HZ, xTickCount, uSec, uMs);

    return pdFALSE;
}

/**
* @brief Reads from UART RX buffer. Reads one bye at the time.
* @param *cReadChar pointer to where data will be stored.
* @retval HAL status
*/
static BaseType_t xConsoleRead(uint8_t *cReadChar, size_t xLen)
{
    BaseType_t xRetVal = pdFALSE;

    if (xQueueRxHandle == NULL || cReadChar == NULL)
    {
        return xRetVal;
    }

    /* Block until the there is input from the user */
    return xQueueReceive(xQueueRxHandle, cReadChar, portMAX_DELAY);
}

/**
* @brief Write to UART TX
* @param *buff buffer to be written.
* @retval HAL status
*/
static HAL_StatusTypeDef vConsoleWrite(const char *buff)
{
    HAL_StatusTypeDef status;
    size_t len = strlen(buff);

    if (pxUartDevHandle == NULL || *buff == '\0' || len < 1)
    {
        return HAL_ERROR;
    }

    status = HAL_UART_Transmit(pxUartDevHandle, (uint8_t *)buff, strlen(buff), portMAX_DELAY);
    if (status != HAL_OK)
    {
    	return HAL_ERROR;
    }
    return status;
}

/**
* @brief Enables UART RX reception.
* @param void
* @retval void
*/
void vConsoleEnableRxInterrupt(void)
{
    if (pxUartDevHandle == NULL)
    {
        return;
    }
    /* UART Rx IT is enabled by reading a character */
    HAL_UART_Receive_IT(pxUartDevHandle,(uint8_t*)&cRxData, 1);
}

/**
* @brief Task to handle user commands via serial communication.
* @param void* Data passed at task creation.
* @retval void
*/
void vTaskConsole(void *pvParams)
{
    char cReadCh = '\0';
    uint8_t uInputIndex = 0;
    BaseType_t xMoreDataToProcess;
    char pcInputString[MAX_IN_STR_LEN];
    char pcPrevInputString[MAX_IN_STR_LEN];
    char pcOutputString[MAX_OUT_STR_LEN];

    memset(pcInputString, 0x00, MAX_IN_STR_LEN);
    memset(pcPrevInputString, 0x00, MAX_IN_STR_LEN);
    memset(pcOutputString, 0x00, MAX_OUT_STR_LEN);

    /* Create a queue to store characters from RX ISR */
    xQueueRxHandle = xQueueCreate(MAX_RX_QUEUE_LEN, sizeof(char));
    if (xQueueRxHandle == NULL)
    {
        goto out_task_console;
    }

    vConsoleWrite(pcWelcomeMsg);
    vConsoleEnableRxInterrupt();
    vConsoleWrite(prvpcPrompt);

    while(1)
    {
        /* Block until there is a new character in RX buffer */
        xConsoleRead(&cReadCh, sizeof(cReadCh));

        switch (cReadCh)
        {
            case ASCII_CR:
            case ASCII_LF:
                if (uInputIndex != 0)
                {
                    vConsoleWrite("\n");
                    strncpy(pcPrevInputString, pcInputString, MAX_IN_STR_LEN);
                    do
                    {
                        xMoreDataToProcess = FreeRTOS_CLIProcessCommand
                                            (
                                                pcInputString,    /* Command string*/
                                                pcOutputString,   /* Output buffer */
                                                MAX_OUT_STR_LEN   /* Output buffer size */
                                            );
                        vConsoleWrite(pcOutputString);
                    } while(xMoreDataToProcess != pdFALSE);
                                    }
                else
                {
                    vConsoleWrite("\n");
                }
                uInputIndex = 0;
                memset(pcInputString, 0x00, MAX_IN_STR_LEN);
                memset(pcOutputString, 0x00, MAX_OUT_STR_LEN);
                vConsoleWrite(prvpcPrompt);
                break;
            case ASCII_FORM_FEED:
                vConsoleWrite("\x1b[2J\x1b[0;0H");
                vConsoleWrite("\n");
                vConsoleWrite(prvpcPrompt);
                break;
            case ASCII_CTRL_PLUS_C:
                uInputIndex = 0;
                memset(pcInputString, 0x00, MAX_IN_STR_LEN);
                vConsoleWrite("\n");
                vConsoleWrite(prvpcPrompt);
                break;
                        case ASCII_NACK:
            case ASCII_BACKSPACE:
                if (uInputIndex > 0)
                {
                    uInputIndex--;
                    pcInputString[uInputIndex] = '\0';
                    vConsoleWrite("\b \b");
                }
                break;
            case ASCII_TAB:
                while (uInputIndex)
                {
                    uInputIndex--;
                    vConsoleWrite("\b \b");
                }
                strncpy(pcInputString, pcPrevInputString, MAX_IN_STR_LEN);
                uInputIndex = (unsigned char)strlen(pcInputString);
                vConsoleWrite(pcInputString);
                break;
case ASCII_DEL: /* Delete or CTRL + backspace */
                while (uInputIndex)
                {
                    uInputIndex--;
                    vConsoleWrite("\b \b");
                }
                 memset(pcInputString, 0x00, MAX_IN_STR_LEN);
                break;
            default:
                /* Check if read character is between [Space] and [~] in ASCII table */
                if (uInputIndex < (MAX_IN_STR_LEN - 1 ) && (cReadCh >= 32 && cReadCh <= 126))
                {
                    pcInputString[uInputIndex] = cReadCh;
                    vConsoleWrite(pcInputString + uInputIndex);
                    uInputIndex++;
                }
                break;
        }
    }

out_task_console:
    if (xQueueRxHandle)
    {
        vQueueDelete(xQueueRxHandle);
    }
    vTaskDelete(NULL);
}

/**
* @brief Initialize the console by registering all commands and creating a task.
* @param usStackSize Task console stack size
* @param uxPriority Task console priority
* @param *pxUartHandle Pointer for uart handle.
* @retval FreeRTOS status
*/
BaseType_t xConsoleInit(uint16_t usStackSize, UBaseType_t uxPriority, UART_HandleTypeDef *pxUartHandle)
{
    const CLI_Command_Definition_t *pCommand;

    if (pxUartHandle == NULL)
    {
        return pdFALSE;
    }
    pxUartDevHandle = pxUartHandle;

    /* Register all commands that can be accessed by the user */
    for (pCommand = xCommands; pCommand->pcCommand != NULL; pCommand++)
    {
        FreeRTOS_CLIRegisterCommand(pCommand);
    }
    return xTaskCreate(vTaskConsole,"CLI", usStackSize, NULL, uxPriority, NULL);
}

/**
* @brief Callback for UART RX, triggered any time there is a new character.
* @param *huart Pointer to the uart handle.
* @retval void
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

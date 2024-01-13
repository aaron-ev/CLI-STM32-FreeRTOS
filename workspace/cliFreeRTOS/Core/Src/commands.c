#include "stdio.h"
#include <string.h>
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"
#include "task.h"
#include "stm32f401xc.h"
#include "stm32f4xx_hal.h"

/*
* TODO_LIST:   * GPIO: Check GPIO instance numbers in GPIO functions.
               * UART: Figure out how to listen a UART channel with just one queue or multiple.
               * UART: prvCommandUartListen: Check how to write correctly to the FreeRTOS output buffer.
               * PWM: NONE
*/

//extern xSemaphoreHandle xUartRxMutex;
//extern QueueHandle_t xQueueInputHandle;

static BaseType_t prvCommandPwmSetFreq(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandPwmSetDuty(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandUartListen(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandGpioWrite(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandGpioRead( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandEcho( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvCommandTaskStats( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

static const char *prvpcTaskListHeader = "Task states: BL = Blocked Re = Ready DE = Deleted  SU = Suspended\n\n"\
                                         "Task name                         State  Priority  Stack remaining  %%CPU usage  Runtime\n"\
                                         "================================  =====  ========  ===============  ===========  =======\n";

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

static const GPIO_TypeDef *xMapGpioInstances(const char pcGpioInstance)
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
        /* Ddo nothing*/
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
        default: break; /* Do nothing */
    }
}

static const CLI_Command_Definition_t xCommandTaskStats =
{
    "task-stats",
    "\r\ntask-stats:\r\n Displays a table with the state of each FreeRTOS task\r\n",
    prvCommandTaskStats,
    0 /* 0 parameters expected */
};

static const CLI_Command_Definition_t xCommandGpioWrite =
{
    "gpio-write",
    "\r\ngpio-write [gpio port] [pin number] [logical value]: Write a digital value to a port pin, example: gpio-write a 2 0 --> write logical zero to pin number 2 of GPIO port a\r\n",
    prvCommandGpioWrite,
    3, /* Parameters: [GPIO PORT] [GPIO pin number] [Logical value (1, 0)] */
};

static const CLI_Command_Definition_t xCommandGpioRead =
{
    "gpio-read",
    "\r\ngpio-read [gpio port] [pin number] : Read logical level of a GPIO pin, example: gpio-read a 2 --> read GPIOA pin number 2\r\n",
    prvCommandGpioRead,
    2,
};

static const CLI_Command_Definition_t xCommandEcho =
{
    "echo",
    "\r\n echo [string to echo]\r\n",
    prvCommandEcho,
    1,
};

static const CLI_Command_Definition_t xCommandPwmSetFreq =
{
    "pwm-f",
    "\r\npwm-f [pwmChannel] [new frequency]: Update PWM frequency of a giving channel \r\n",
    prvCommandPwmSetFreq,
    2,
};

static const CLI_Command_Definition_t xCommandPwmSetDuty =
{
    "pwmSetDuty",
    "\r\npwmSetDuty [pwmChannel] [new duty cycle]: Update PWM duty cycle of a giving channel \r\n",
    prvCommandPwmSetDuty,
    2,
};

static const CLI_Command_Definition_t xCommandUartListen =
{
    "uart-listen",
    "\r\nuart-listen [uart instance]: Listen teh RX hardware buffer \r\n",
    prvCommandUartListen,
    2,
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
                "%-32s  %5s  %8lu  %15u  %11u  %7u\n",
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

static BaseType_t prvCommandGpioWrite(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    BaseType_t xParamLen;
    char * pcParam;
    char cGpioPort; 
    GPIO_TypeDef *xGpioInstance;
    GPIO_PinState xNewPinState;
    uint16_t uPinNumber;

    /* Get GPIO instance, pin number and new pin state specified by the user */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    cGpioPort = *pcParam;
    xGpioInstance = xMapGpioInstances(*pcParam);
    /* Get pin number */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParamLen);
    uPinNumber = uMapGpioNumber(atoi(pcParam));
    /* Get new pin state */
    pcParam = FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParamLen);
    xNewPinState = atoi(pcParam);
    HAL_GPIO_WritePin(xGpioInstance, uPinNumber, xNewPinState);

    return pdFALSE;
}

static BaseType_t prvCommandGpioRead( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    // uint8_t ucPinState;
    // BaseType_t xParamLen;
    // uint8_t ucParamGpioPinNum;
    // uint8_t ucParamGpioInstanceNum;
    // GPIO_TypeDef *xGpioInstance = NULL;

    // /* Get GPIO instance and pin number specified by the user */
    // ucParamGpioInstanceNum = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    // ucParamGpioPinNum = FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParamLen);

    // /* Validate user parameters */
    // if (xParamLen != 1)
    // {
    //     strcpy(pcWriteBuffer, "Parameter 1 should have length of 1");
    // }

    // xGpioInstance = xGPIOMapInstances[ucParamGpioInstanceNum];
    // ucPinState = HAL_GPIO_ReadPin(xGpioInstance, ucParamGpioPinNum);

    // /* Send pin state to the user */
    // (ucPinState == GPIO_PIN_SET) ? strcpy(xWriteBufferLen, "HIGH"):strcpy(xWriteBufferLen, "LOW");

    return pdFALSE;
}

static BaseType_t prvCommandEcho( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    char *pcStrToOutput;
    BaseType_t xParamLen;
    pcStrToOutput = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    strcpy(pcWriteBuffer, pcStrToOutput);
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

static BaseType_t prvCommandUartListen(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    // char cReadChar;
    // BaseType_t xParamLen;
    // TickType_t xTimeToListen;
    // TickType_t xStartListening;

    // xTimeToListen = FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParamLen);
    // xTimeToListen = pdMS_TO_TICKS(xTimeToListen);

    // if (xSemaphoreTake(xUartRxMutex, pdMS_TO_TICKS(100)) != pdTRUE)
    // {
    //     strcpy(pcWriteBuffer, "Error: Could not listen UART device");
    // }
    // else
    // {
    //     /* Read RX characters until time specified by ther user */
    //     xStartListening = xTaskGetTickCount();
    //     while (xTaskGetTickCount() - xStartListening < xTimeToListen)
    //     {
    //         QueueReceive(xQueueInputHandle, cReadChar, 1);
    //         pcWriteBuffer[0] = cReadChar;
    //         return pdTRUE;
    //     }
    //      strcpy(pcWriteBuffer, "CONSOLE: Time for listening completed\n");
    // }
     return pdFALSE;
}


void vConsoleRegisterCommands(void)
{
    FreeRTOS_CLIRegisterCommand(&xCommandEcho);
    FreeRTOS_CLIRegisterCommand(&xCommandGpioRead);
    FreeRTOS_CLIRegisterCommand(&xCommandGpioWrite);
    FreeRTOS_CLIRegisterCommand(&xCommandPwmSetDuty);
    FreeRTOS_CLIRegisterCommand(&xCommandPwmSetFreq);
    FreeRTOS_CLIRegisterCommand(&xCommandTaskStats);
    FreeRTOS_CLIRegisterCommand(&xCommandUartListen);
}

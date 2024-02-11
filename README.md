IMPORTANT: This repo is under developmcommandent and first release will be posted by end of February 2024.

- [Introduction: FreeRTOS CLI + STM32](#introduction-freertos-cli--stm32)
- [Commands](#commands)
  - [Help](#help)
  - [GPIO read](#gpio-read)
  - [GPIO write](#gpio-write)
  - [Task statistics](#task-statistics)
  - [Heap](#heap)
  - [Clock](#clock)
  - [Ticks](#ticks)
  - [Pwm set frequency and set duty](#pwm-set-frequency-and-set-duty)
  - [RTC set and get time](#rtc-set-and-get-time)
  - [Version](#version)
- [Console software architecture](#console-software-architecture)
- [API documentation with Doxygen](#api-documentation-with-doxygen)

# Introduction: FreeRTOS CLI + STM32
FreeRTOS-Plus-CLI (Command Line Interface) provides a simple, small, extensible and RAM efficient method of enabling your FreeRTOS application to process command line input. Reference: <FreeRTOS CLI>(https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/FreeRTOS_Plus_Command_Line_Interface.html).

The following output is what would you see if you open a serial port with a serial cable connected to the microcontroller. The serial device configuration should be  9600 baud, no parity, 1 stop bit and 7 bit data frame (very common serial device configuration).
```
#cmd: Welcome to the console. Enter 'help' to view a list of available commands.
#cmd:
```
Once you are here, type any of the following commands.

# Commands
## Help

*help* lists all available commands.

```
#cmd: help

help:
 Lists all the registered commands

stats:
 Displays a table with the state of each FreeRTOS task.

gpio-w <gpio port> <pin number> <logical value>: Write a digital value to a GPIO pin.

gpio-r <gpio port> <pin number>: Read a GPIO pin.

echo <string to echo>

pwm-f <Frequency>: Set a new frequency.

pwm-d <Duty cycle> <Channel>: Set a new PWM duty cycle of a giving channel.

heap: Display free heap memory.

clk: Display clock information.

ticks: Display OS tick count and run time in seconds.
```

## GPIO read

*gpio-r* reads a GPIO pin state

Example: Read GPIO port A pin number 2

```
#cmd: gpio-r a 2

Pin state: 0
```

## GPIO write

*gpio-w* writes to a GPIO pin

Example: Write 1 to GPIO port C pin number 12

```
#cmd: gpi-w c 12 1

Pin set to 1
```

## Task statistics

*stats* shows a list with relevant information of each task such as task name,
state, priority, stack remaining, CPU usage and runtime.
For more information about FreeRTOS statistics, take a look at [FreeRTOS statistics](https://www.freertos.org/rtos-run-time-stats.html).

```
#cmd: stats

Task states: BL = Blocked RE = Ready DE = Deleted  SU = Suspended

Task name                         State  Priority  Stack remaining  %CPU usage  Runtime
================================  =====  ========  ===============  ===========  =======
CLI                                  RE         1             2638            0        0
IDLE                                 RU         0              108            0        0
task-hear                            Bl         1              104            0        0
Tmr Svc
```

## Heap

*heap* Shows heap size, remaining memory in the heap and
the minimum heap size ever existed since power on. Note: See FreeRTOS documentation for more information. To understand each output of this command, take a look at [FreeRTOS memory management](https://www.freertos.org/a00111.html).

```
#cmd: heap
Heap size            : 39300 bytes ( 38 KiB)
Remaining            : 24016 bytes ( 23 KiB)
Minimum ever existed : 23864 bytes ( 23 KiB)
```

## Clock

*clk* Shows STM32 clock information.

```
#cmd: clk
Clock name           Hz       kHz       MHz
===========       ========  ========  ========
System clock      80000000     80000        80
APB1 peripheral   20000000     20000        20
APB2 peripheral   80000000     80000        80
APB1 timers       40000000     40000        40
APB2 timers       80000000     80000        80
```

## Ticks

*ticks* Shows FreeRTOS tick count in ticks and run time in
seconds.

```
#cmd: ticks

Tick rate: 1000 Hz
Ticks: 2852
Run time: 2.852 seconds
```

## Pwm set frequency and set duty

*pwm-f* sets a new frequency in Hz.

*pwm-d* sets a new duty of a giving timer and channel. Duty cycle must be between 1% and 100%.

![pwm-f command](/docs/img/pwmCommand.png)

## RTC set and get time

*rtc-s* Sets a new time in 24hr format. Example: Set time to 12:0:0.

```
#cmd: rtc-s 12 0 0

Time (24hr format) set to: 12:0:0
```

*rtc-g* Gets the current time stored in RTC time registers. Example:

```
#cmd: rtc-g

Time (24hr format): 12:1:51
```

## Version

*version* Shows the current console version. Example:
```
#cmd: version

1.0
```

# Console software architecture

![Software architecture](/docs/img/sw_architecture.png)

# API documentation with Doxygen

There is documentation for every function in this project, Doxygen (documentation generator) was used to generate a PDF report [sw_docs_doxygen.pdf](/docs/doxygen/sw_docs_doxygen.pdf) that can be found in docs folder.

Doxygen also generates a HTML format that can be open from a web browser [HTML](/docs/doxygen/html/index.html).

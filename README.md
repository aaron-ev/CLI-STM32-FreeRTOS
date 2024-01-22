IMPORTANT: This repo is under development and first release will be posted by end of February 2024.

Table of contents:
- [CLI based on FreeRTOS and STM32](#cli-based-on-freertos-and-stm32)
- [Help command](#help-command)
- [GPIO read command](#gpio-read-command)
- [GPIO write command](#gpio-write-command)
- [Task statistics command](#task-statistics-command)
- [Heap command](#heap-command)
- [Clock command](#clock-command)
- [Ticks command](#ticks-command)

# CLI based on FreeRTOS and STM32

```
Welcome to the console. Enter 'help' to view a list of available commands.

Command:

```

# Help command

*help* command will list all available commands.

```
Command: help

help:
 Lists all the registered commands


task-stats:
 Displays a table with the state of each FreeRTOS task

gpio-w [gpio port] [pin number] [logical value]: Write a digital value to a port pin, example: gpio-w a 2 0 --> write logical zero to pin number 2 of GPIO port a

gpio-r [gpio port] [pin number] : Read logical level of a GPIO pin, example: gpio-r a 2 --> read GPIOA pin number 2

echo [string to echo]

pwm-f [pwmChannel] [new frequency]: Update PWM frequency of a giving channel 

pwmSetDuty [pwmChannel] [new duty cycle]: Update PWM duty cycle of a giving channel 

heap: Display free heap memory

clk: Display clock information

ticks: Display OS tick count and run time in seconds
```


# GPIO read command

*gpio-r* command will read a GPIOx pin

Example: Read GPIO port A pin number 2

```
Command: gpio-r  12

Pin state: 0
```

# GPIO write command

*gpio-w* command will write to GPIOx pin

Example: Write 1 to GPIO port C pin number 12

```
Command: gpi-w c 12 1

Pin set to 1
```

# Task statistics command

*task-stats* command shows a list with relevant information of each task such as task name, 
state, priority, stack remaining, CPU usage and runtime.

```
Command: task-stats

Task states: BL = Blocked RE = Ready DE = Deleted  SU = Suspended

Task name                         State  Priority  Stack remaining  %CPU usage  Runtime
================================  =====  ========  ===============  ===========  =======
CLI                                  RE         1             2638            0        0
IDLE                                 RU         0              108            0        0
task-hear                            Bl         1              104            0        0
Tmr Svc
```

# Heap command

*heap* command will display the heap size, remaining memory in the heap and 
the minimum heap size ever existed since power on. 

```
Command: heap
Heap size            : 39300 bytes ( 38 KiB)
Remaining            : 24016 bytes ( 23 KiB)
Minimum ever existed : 23864 bytes ( 23 KiB)
```

# Clock command

*clk* command will display STM32 clock information.

```
Command: clk
Clock name           Hz       kHz       MHz
===========       ========  ========  ========
System clock      80000000     80000        80
APB1 peripheral   20000000     20000        20
APB2 peripheral   80000000     80000        80
APB1 timers       40000000     40000        40
APB2 timers       80000000     80000        80
```

# Ticks command

*ticks* command will display FreeRTOS tick count in ticks and run time in
seconds.

```
Command: ticks

Tick rate: 1000 Hz
Ticks: 2852
Run time: 2.852 seconds
```

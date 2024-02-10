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
- [Pwm set frequency and set duty command](#pwm-set-frequency-and-set-duty-command)

# CLI based on FreeRTOS and STM32

```
#cmd: Welcome to the console. Enter 'help' to view a list of available commands.
#cmd:

```

# Help command

*help* lists all available commands.

```
#cmd: help

help:
 Lists all the registered commands

stats:
 Displays a table with the state of each FreeRTOS task.

gpio-w [gpio port] [pin number] [logical value]: Write a digital value to a GPIO pin.

gpio-r [gpio port] [pin number] : Read a GPIO pin.

echo [string to echo]

pwm-f [Frequency]: Set a new frequency.

pwm-d [Duty cycle] [Channel]: Set a new PWM duty cycle of a giving channel.

heap: Display free heap memory.

clk: Display clock information.

ticks: Display OS tick count and run time in seconds.
```

# GPIO read command

*gpio-r* reads a GPIO pin state

Example: Read GPIO port A pin number 2

```
#cmd: gpio-r a 2

Pin state: 0
```

# GPIO write command

*gpio-w* writes to a GPIO pin

Example: Write 1 to GPIO port C pin number 12

```
#cmd: gpi-w c 12 1

Pin set to 1
```

# Task statistics command

*stats* shows a list with relevant information of each task such as task name,
state, priority, stack remaining, CPU usage and runtime.

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

# Heap command

*heap* Shows heap size, remaining memory in the heap and
the minimum heap size ever existed since power on. Note: See FreeRTOS documentation for more information.

```
#cmd: heap
Heap size            : 39300 bytes ( 38 KiB)
Remaining            : 24016 bytes ( 23 KiB)
Minimum ever existed : 23864 bytes ( 23 KiB)
```

# Clock command

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

# Ticks command

*ticks* Shows FreeRTOS tick count in ticks and run time in
seconds.

```
#cmd: ticks

Tick rate: 1000 Hz
Ticks: 2852
Run time: 2.852 seconds
```

# Pwm set frequency and set duty command

*pwm-f* sets a new frequency in Hz.

*pwm-d* sets a new duty of a giving timer and channel. Duty cycle must be between 1% and 100%.

![pwm-f command](/img/pwmCommand.png)


Table of contents:
- [CLI based on FreeRTOS and STM32](#cli-based-on-freertos-and-stm32)
- [Help command](#help-command)
- [GPIO read command](#gpio-read-command)
- [GPIO write command](#gpio-write-command)
- [Task statistics command](#task-statistics-command)
- [Heap command](#heap-command)
- [Clock command](#clock-command)
- [ticks command](#ticks-command)

# CLI based on FreeRTOS and STM32
![welcomeMsg](/img/welcomeMsg.png)

# Help command

*help* command will list all available commands.

![helpCommand](/img/helpCommand.png)

# GPIO read command

Example: Read GPIO port A pin number 2

![gpioReadCommand](/img/gpioReadCommand.png)
# GPIO write command

Example: Write 1 to GPIO port C pin number 12

![gpioWriteCommand](/img/gpioWriteCommand.png)

# Task statistics command
Shows a list with relevant information of each task such as task name, 
state, priority, stack remaining, CPU usage and runtime.

![task-stats](/img/taskStatsCommand.png)

# Heap command

*heap* command will display the heap size, remaining memory in the heap and 
the minimum heap size ever existed since power on. 

![heap command](/img/heapCommand.png)

# Clock command

*clk* command will display STM32 clock information.

![clk command](/img/clkCommand.png)

# ticks command

*ticks* command will display FreeRTOS tick count in ticks and run time in
seconds.

```
Command: ticks

Tick rate: 1000 Hz
Ticks: 2852
Run time: 2.852 seconds
```

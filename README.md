# CLI-console-FreeRTOS
![helpCommand](/img/helpCommand.png)

# Available commands 

* GPIO read operation
```
gpio-r [GPIO PORT] [GPIO pin number]

Example: Read GPIO port A pin number 2

gpio-r A 2 
```

* GPIO write operation
```
gpio-w [GPIO PORT] [GPIO pin number] [New pin state]

Example: Write 1 to GPIO port C pin number 12

gpio-w C 12 1 
```

* Task information 

Shows a list with relevant information of each task such as task name, 
state, priority, stack remaining, CPU usage and runtime.

```
task-stats 
```
![task-stats](/img/taskStatsCommand.png)



# MS51_TimerEvent_CheckMCUIdle
MS51_TimerEvent_CheckMCUIdle

update @ 2025/11/27

1. init simple ring buffer timer event , and measure MCU loading (idle timing)

2. disable all interrupt , disable all task , only TIMER interrupt , and enable CpuLoadMonitor_Callback , to measure idle_max

- enable #define CPU_LOAD_CALIBRATION , under cpu_load_monitor.h

```c

#define CPU_LOAD_CALIBRATION

```

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/get_idle_max_value.jpg)


__record the idle_max value into idle_max in g_MCUIdleManager__

- turn off CPU_LOAD_CALIBRATION 

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/turn_off_CPU_LOAD_CALIBRATION.jpg)

3. when enable task 1 

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/turn_on_task_1.jpg)


when enable task 1 , 2

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/turn_on_task_1_2.jpg)


when enable task 1 , 2 , 3

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/turn_on_task_1_2_3.jpg)


when enable task 1 , 2 , 3 , 4

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/turn_on_task_1_2_3_4.jpg)


when enable task 1 , 2 , 3 , 4 , 5

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/turn_on_task_1_2_3_4_5.jpg)


4. when enable heavy loading task (1ms) , with enable printf

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/enable_flag_1ms_enable_printf.jpg)


when enable heavy loading task (1ms) , with disable printf

![image](https://github.com/released/MS51_TimerEvent_CheckMCUIdle/blob/main/enable_flag_1ms_remove_printf.jpg)

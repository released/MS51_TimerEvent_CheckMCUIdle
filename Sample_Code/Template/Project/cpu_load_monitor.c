/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "numicro_8051.h"

#include "timer_service.h"
#include "cpu_load_monitor.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

volatile MCU_IDLE_MANAGER_T g_MCUIdleManager = 
{
	0UL,            /*idle_counter*/
	27575UL,        /*idle_max*/
    0UL,            /*cpu_load_permil*/
	-1,             /*timer_id_cpumon*/
};

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void CpuLoadMonitor_Init(unsigned long idle_max_value)
{
    g_MCUIdleManager.idle_counter = 0UL;
    g_MCUIdleManager.idle_max = idle_max_value;
    g_MCUIdleManager.cpu_load_permil = 0UL;
}

void CpuLoadMonitor_Callback(void *user_data)
{
    uint32_t idle_now;
    uint32_t load_permil;

    user_data = user_data;   /* mark as used, avoid C280 */
    idle_now = g_MCUIdleManager.idle_counter;
    g_MCUIdleManager.idle_counter = 0UL;  /* reset for next 1s window */

#if defined (CPU_LOAD_CALIBRATION)
    /* calibraiton stage , reserve max idle value  */
    if (idle_now > g_MCUIdleManager.idle_max)
    {
        g_MCUIdleManager.idle_max = idle_now;
    }
#endif

    if (g_MCUIdleManager.idle_max == 0UL)
    {
        /* not finish , exit */
        return;
    }

    if (idle_now >= g_MCUIdleManager.idle_max)
    {
        load_permil = 0UL;
    }
    else
    {
        load_permil =
            ( (g_MCUIdleManager.idle_max - idle_now) * 1000UL ) / g_MCUIdleManager.idle_max;
    }

    g_MCUIdleManager.cpu_load_permil = load_permil;

    /* enable log only for debug , prevent heavy loading */
#if defined (CPU_LOAD_CALIBRATION)
    printf("idle_now=%lu, idle_max=%lu\r\n",
            (unsigned long)idle_now,
            (unsigned long)g_MCUIdleManager.idle_max);
#else
    printf("CPU load = %3lu.%01lu%% (idle_now=%lu, idle_max=%lu),Queue depth max=%u / %u, overflow=%lu\r\n",
            (unsigned long)(load_permil / 10UL),
            (unsigned long)(load_permil % 10UL),
            (unsigned long)idle_now,
            (unsigned long)g_MCUIdleManager.idle_max,
            (unsigned int)TimerService_GetQueueMaxUsed(),
            (unsigned int)TIMER_EVENT_QUEUE_SIZE,
            (unsigned long)TimerService_GetQueueOverflowCnt());

    TimerService_ClearQueueStats();            
#endif


}


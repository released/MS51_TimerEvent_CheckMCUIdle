#ifndef __CPU_LOAD_MONITOR_H__
#define __CPU_LOAD_MONITOR_H__

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

/*  
	template
	typedef struct _peripheral_manager_t
	{
		uint16_t* pu16Far;
		uint8_t u8Cmd;
		uint8_t au8Buf[33];
		uint8_t u8RecCnt;
		uint8_t bByPass;
	}PERIPHERAL_MANAGER_T;

	volatile PERIPHERAL_MANAGER_T g_PeripheralManager = 
	{
		.pu16Far = NULL,	//.pu16Far = 0	
		.u8Cmd = 0,
		.au8Buf = {0},		//.au8Buf = {100U, 200U},
		.u8RecCnt = 0,
		.bByPass = FALSE,
	};
	extern volatile PERIPHERAL_MANAGER_T g_PeripheralManager;
*/

typedef struct _mcu_idle_manager_t
{
	uint32_t idle_counter;
	uint32_t idle_max;
	uint32_t cpu_load_permil;
	uint16_t timer_id_cpumon;
}MCU_IDLE_MANAGER_T;

extern volatile MCU_IDLE_MANAGER_T g_MCUIdleManager;


/*-----------------------------------------------------------
 * CPU loading calibration mode
 * Only when this is defined, idle_max will be updated at runtime.
 *----------------------------------------------------------*/
// #define CPU_LOAD_CALIBRATION


/*_____ M A C R O S ________________________________________________________*/


/*_____ F U N C T I O N S __________________________________________________*/

void CpuLoadMonitor_Init(unsigned long idle_max_value);

void CpuLoadMonitor_Callback(void *user_data);

#endif //__CPU_LOAD_MONITOR_H__

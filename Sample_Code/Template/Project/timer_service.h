#ifndef __TIMER_SERVICE_H__
#define __TIMER_SERVICE_H__

/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>

/*_____ D E C L A R A T I O N S ____________________________________________*/

/* Max number of software timers */
#define TIMER_SERVICE_MAX_TIMERS        (8u)

/* Event queue size */
#define TIMER_EVENT_QUEUE_SIZE          (16u)

/* Timer type */
#define TIMER_KIND_FLAG                 (0u)    /* flag-based, no queue   */
#define TIMER_KIND_QUEUE                (1u)    /* queue-based, ring buf  */


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
	
	/////////////////////////////////////////////////////////////
	
*/

typedef void (*TIMER_CALLBACK_T)(void *user_data);

typedef struct _timer_instance_t
{
    uint16_t        period_ms;      /* 1~65535 ms */
    uint16_t        counter_ms;     /* internal counter */
    uint8_t         active;         /* 1 = running */
    uint8_t         kind;           /* TIMER_KIND_xxx */
    uint8_t         pending;        /* for FLAG timers */
    uint8_t         reserved;
    TIMER_CALLBACK_T callback;
    void           *user_data;
} TIMER_INSTANCE_T;

typedef struct _timer_event_queue_t
{
    uint8_t  head;
    uint8_t  tail;
    uint8_t  count;
    uint8_t  reserved;

    uint8_t  max_used;       /* max queue usage ever seen */
    uint16_t overflow_cnt;   /* how many events were dropped */

    int8_t   ids[TIMER_EVENT_QUEUE_SIZE];

} TIMER_EVENT_QUEUE_T;

/* exported globals for ISR use */
extern volatile uint8_t g_ts_tick_pending;  /* how many 1ms ticks pending */

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

/* API */
void  TimerService_Init(void);

/* create timers (only main context should call) */
int8_t TimerService_CreateTimerFlag(uint16_t period_ms,
                                    TIMER_CALLBACK_T cb,
                                    void *user_data);

int8_t TimerService_CreateTimerQueue(uint16_t period_ms,
                                     TIMER_CALLBACK_T cb,
                                     void *user_data);

/* backward compatible: default use QUEUE type */
int8_t TimerService_CreateTimer(uint16_t period_ms,
                                TIMER_CALLBACK_T cb,
                                void *user_data);

/* control */
void  TimerService_StartTimer(uint8_t timer_id);
void  TimerService_StopTimer(uint8_t timer_id);
void  TimerService_ChangePeriod(uint8_t timer_id, uint16_t new_period_ms);

/* main-loop service
   - consume all pending 1ms ticks
   - update timers and push queue events
   - call callbacks
*/
void  TimerService_Process(void);

/* optional: get system tick in ms (only main context should call) */
uint32_t TimerService_GetTickMs(void);

/* queue statistics (main context only) */
uint8_t  TimerService_GetQueueMaxUsed(void);
uint8_t  TimerService_GetQueueCurrentUsed(void);
uint16_t TimerService_GetQueueOverflowCnt(void);
void     TimerService_ClearQueueStats(void);

#endif //__TIMER_SERVICE_H__

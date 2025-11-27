/*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>
#include "numicro_8051.h"

#include "timer_service.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

volatile TIMER_INSTANCE_T xdata g_TimerService_List[TIMER_SERVICE_MAX_TIMERS];
volatile TIMER_EVENT_QUEUE_T xdata g_TimerEventQueue;

/* Number of 1ms ticks pending, increased in ISR, consumed in main */
volatile uint8_t g_ts_tick_pending = 0u;

/* System time in ms, updated only in main context */
static uint32_t g_ts_tick_ms = 0ul;

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

static void TimerService_OnTick1ms(void)
{
    uint8_t i;

    for (i = 0u; i < TIMER_SERVICE_MAX_TIMERS; i++)
    {
        if ((g_TimerService_List[i].active != 0u) &&
            (g_TimerService_List[i].callback != (TIMER_CALLBACK_T)0))
        {
            if (g_TimerService_List[i].counter_ms < 0xFFFFu)
            {
                g_TimerService_List[i].counter_ms++;
            }

            if (g_TimerService_List[i].counter_ms >= g_TimerService_List[i].period_ms)
            {
                g_TimerService_List[i].counter_ms = 0u;

                if (g_TimerService_List[i].kind == TIMER_KIND_FLAG)
                {
                    if (g_TimerService_List[i].pending == 0u)
                    {
                        g_TimerService_List[i].pending = 1u;
                    }
                }
                else
                {
                    /* queue-based: push timer id to event queue */
                    if (g_TimerEventQueue.count < TIMER_EVENT_QUEUE_SIZE)
                    {
                        g_TimerEventQueue.ids[g_TimerEventQueue.tail] = (int8_t)i;

                        g_TimerEventQueue.tail++;
                        if (g_TimerEventQueue.tail >= TIMER_EVENT_QUEUE_SIZE)
                        {
                            g_TimerEventQueue.tail = 0u;
                        }

                        g_TimerEventQueue.count++;

                        /* update max_used */
                        if (g_TimerEventQueue.count > g_TimerEventQueue.max_used)
                        {
                            g_TimerEventQueue.max_used = g_TimerEventQueue.count;
                        }
                    }
                    else
                    {
                        /* overflow, drop event */
                        if (g_TimerEventQueue.overflow_cnt < 0xFFFFu)
                        {
                            g_TimerEventQueue.overflow_cnt++;
                        }
                    }

                }
            }
        }
    }
}

static void TimerService_DispatchQueue(void)
{
    uint8_t head;
    uint8_t count;
    int8_t  id;
    TIMER_CALLBACK_T cb;
    void   *user;

    head  = g_TimerEventQueue.head;
    count = g_TimerEventQueue.count;

    while (count > 0u)
    {
        id = g_TimerEventQueue.ids[head];

        head++;
        if (head >= TIMER_EVENT_QUEUE_SIZE)
        {
            head = 0u;
        }

        count--;

        if ((id >= 0) && ((uint8_t)id < TIMER_SERVICE_MAX_TIMERS))
        {
            cb   = g_TimerService_List[(uint8_t)id].callback;
            user = g_TimerService_List[(uint8_t)id].user_data;

            if (cb != (TIMER_CALLBACK_T)0)
            {
                cb(user);
            }
        }
    }

    g_TimerEventQueue.head  = head;
    g_TimerEventQueue.count = 0u;
}

static void TimerService_DispatchFlags(void)
{
    uint8_t i;
    TIMER_CALLBACK_T cb;
    void   *user;

    for (i = 0u; i < TIMER_SERVICE_MAX_TIMERS; i++)
    {
        if ((g_TimerService_List[i].kind    == TIMER_KIND_FLAG) &&
            (g_TimerService_List[i].active  != 0u) &&
            (g_TimerService_List[i].callback != (TIMER_CALLBACK_T)0) &&
            (g_TimerService_List[i].pending != 0u))
        {
            g_TimerService_List[i].pending = 0u;

            cb   = g_TimerService_List[i].callback;
            user = g_TimerService_List[i].user_data;

            cb(user);
        }
    }
}

void TimerService_Init(void)
{
    uint8_t i;

    g_ts_tick_ms      = 0ul;
    g_ts_tick_pending = 0u;

    g_TimerEventQueue.head          = 0u;
    g_TimerEventQueue.tail          = 0u;
    g_TimerEventQueue.count         = 0u;
    g_TimerEventQueue.reserved      = 0u;
    g_TimerEventQueue.max_used      = 0u;
    g_TimerEventQueue.overflow_cnt  = 0u;

    for (i = 0u; i < TIMER_SERVICE_MAX_TIMERS; i++)
    {
        g_TimerService_List[i].period_ms  = 0u;
        g_TimerService_List[i].counter_ms = 0u;
        g_TimerService_List[i].active     = 0u;
        g_TimerService_List[i].kind       = TIMER_KIND_QUEUE;
        g_TimerService_List[i].pending    = 0u;
        g_TimerService_List[i].reserved   = 0u;
        g_TimerService_List[i].callback   = (TIMER_CALLBACK_T)0;
        g_TimerService_List[i].user_data  = (void *)0;
    }
}

int8_t TimerService_CreateTimerFlag(uint16_t period_ms,
                                    TIMER_CALLBACK_T cb,
                                    void *user_data)
{
    uint8_t i;

    for (i = 0u; i < TIMER_SERVICE_MAX_TIMERS; i++)
    {
        if (g_TimerService_List[i].callback == (TIMER_CALLBACK_T)0)
        {
            g_TimerService_List[i].period_ms  = period_ms;
            g_TimerService_List[i].counter_ms = 0u;
            g_TimerService_List[i].active     = 0u;
            g_TimerService_List[i].kind       = TIMER_KIND_FLAG;
            g_TimerService_List[i].pending    = 0u;
            g_TimerService_List[i].reserved   = 0u;
            g_TimerService_List[i].callback   = cb;
            g_TimerService_List[i].user_data  = user_data;

            return (int8_t)i;
        }
    }

    return (int8_t)-1;
}

int8_t TimerService_CreateTimerQueue(uint16_t period_ms,
                                     TIMER_CALLBACK_T cb,
                                     void *user_data)
{
    uint8_t i;

    for (i = 0u; i < TIMER_SERVICE_MAX_TIMERS; i++)
    {
        if (g_TimerService_List[i].callback == (TIMER_CALLBACK_T)0)
        {
            g_TimerService_List[i].period_ms  = period_ms;
            g_TimerService_List[i].counter_ms = 0u;
            g_TimerService_List[i].active     = 0u;
            g_TimerService_List[i].kind       = TIMER_KIND_QUEUE;
            g_TimerService_List[i].pending    = 0u;
            g_TimerService_List[i].reserved   = 0u;
            g_TimerService_List[i].callback   = cb;
            g_TimerService_List[i].user_data  = user_data;

            return (int8_t)i;
        }
    }

    return (int8_t)-1;
}

int8_t TimerService_CreateTimer(uint16_t period_ms,
                                TIMER_CALLBACK_T cb,
                                void *user_data)
{
    /* default: queue-based */
    return TimerService_CreateTimerQueue(period_ms, cb, user_data);
}

void TimerService_StartTimer(uint8_t timer_id)
{
    if (timer_id >= TIMER_SERVICE_MAX_TIMERS)
    {
        return;
    }

    if (g_TimerService_List[timer_id].callback == (TIMER_CALLBACK_T)0)
    {
        return;
    }

    g_TimerService_List[timer_id].counter_ms = 0u;
    g_TimerService_List[timer_id].pending    = 0u;
    g_TimerService_List[timer_id].active     = 1u;
}

void TimerService_StopTimer(uint8_t timer_id)
{
    if (timer_id >= TIMER_SERVICE_MAX_TIMERS)
    {
        return;
    }

    g_TimerService_List[timer_id].active  = 0u;
    g_TimerService_List[timer_id].pending = 0u;
}

void TimerService_ChangePeriod(uint8_t timer_id, uint16_t new_period_ms)
{
    if (timer_id >= TIMER_SERVICE_MAX_TIMERS)
    {
        return;
    }

    g_TimerService_List[timer_id].period_ms = new_period_ms;
}

/* main-loop service:
   - consume all pending 1ms ticks
   - update timers
   - dispatch queue + flag timers
*/
void TimerService_Process(void)
{
    uint8_t pending;

    pending = g_ts_tick_pending;

    while (pending > 0u)
    {
        TimerService_OnTick1ms();
        g_ts_tick_ms++;

        /* consume one tick (1 byte, so atomic) */
        g_ts_tick_pending--;
        pending--;
    }

    /* callback execution (main context) */
    TimerService_DispatchQueue();
    TimerService_DispatchFlags();
}

uint32_t TimerService_GetTickMs(void)
{
    return g_ts_tick_ms;
}

imerService_GetQueueMaxUsed(void)
{
    return g_TimerEventQueue.max_used;
}

uint8_t TimerService_GetQueueCurrentUsed(void)
{
    return g_TimerEventQueue.count;
}

uint16_t TimerService_GetQueueOverflowCnt(void)
{
    return g_TimerEventQueue.overflow_cnt;
}

void TimerService_ClearQueueStats(void)
{
    g_TimerEventQueue.max_used     = g_TimerEventQueue.count;
    g_TimerEventQueue.overflow_cnt = 0u;
}


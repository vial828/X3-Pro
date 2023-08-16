#ifndef BAT_TIMER_H
#define BAT_TIMER_H

#include "FreeRTOS.h"
#include "timers.h"

#define TIMER_OPT_ONESHOT   0
#define TIMER_OPT_PERIOD    1

typedef TimerHandle_t ptimer_t;
typedef void (*TIMER_CB_F) (ptimer_t tm);

ptimer_t bat_timer_create(char* timer_name, uint32_t ms, uint32_t type, TIMER_CB_F callback);
void bat_timer_start(ptimer_t tm, uint32_t wt);
void bat_timer_stop(ptimer_t tm, uint32_t wt);
void bat_timer_reset(ptimer_t tm, uint32_t ms, uint32_t wt);
BaseType_t bat_timer_delete(ptimer_t tm, uint32_t wt);
ptimer_t bat_timer_reset_ext(ptimer_t tm, char* timer_name, uint32_t ms, uint32_t type, TIMER_CB_F callback);
#endif    /* BAT_TIMER_H */

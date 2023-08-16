#include "batTimer.h"
#include "log.h"


/*************************************************************************************************
  * @brief    : timer_create
  * @param1   : char *:  timer_name
  * @param2   : int: time to wakeup
  * @param3   : int: period/one shot
  * @param4   : TIMER_CB : call back function
  * @return   : timer handler
  * @Instance : timer_create("main_1", 2000, TIMER_OPT_PERIOD, testTimerCallBack);
  * @Note     : before start a timer
*************************************************************************************************/
ptimer_t bat_timer_create(char* timer_name, uint32_t ms, uint32_t type, TIMER_CB_F callback)
{
    return xTimerCreate(timer_name, pdMS_TO_TICKS(ms), type, NULL, callback);
}

/*************************************************************************************************
  * @brief    : timer_start
  * @param1   : ptimer_t:  timer handle;
  * @param2   : int: waiting time to start a timer
  * @return   : void
  * @Instance : timer_start(testTimerExample,0);
  * @Note     :
*************************************************************************************************/
void bat_timer_start(ptimer_t tm, uint32_t wt)
{
    if (tm) {
        xTimerStart(tm, pdMS_TO_TICKS(wt));
    }
}

/*************************************************************************************************
  * @brief    : timer_stop
  * @param1   : ptimer_t:  timer handle;
  * @param2   : int: waiting time to stop a timer
  * @return   : void
  * @Instance : timer_stop(testTimerExample,0);
  * @Note     :
*************************************************************************************************/
void bat_timer_stop(ptimer_t tm, uint32_t wt)
{
    if (tm) {
        xTimerStop(tm, pdMS_TO_TICKS(wt));
    }
}

/*************************************************************************************************
  * @brief    : timer_reset
  * @param1   : ptimer_t:  timer handle;
  * @param2   : int: time to wakeup
  * @param3   : int: waiting time to start a timer
  * @return   : void
  * @Instance : timer_reset(testTimerExample,0);
  * @Note     :
*************************************************************************************************/
void bat_timer_reset(ptimer_t tm, uint32_t ms, uint32_t wt)
{
    if (tm) {
        xTimerReset(tm, pdMS_TO_TICKS(wt));
        xTimerChangePeriod(tm, pdMS_TO_TICKS(ms), pdMS_TO_TICKS(wt));
    }
}

/*************************************************************************************************
  * @brief    : timer_delete
  * @param1   : ptimer_t:  timer handle;
  * @param2   : int: waiting time to delete a timer
  * @return   : void
  * @Instance : timer_delete(testTimerExample,0);;
  * @Note     :
*************************************************************************************************/
BaseType_t bat_timer_delete(ptimer_t tm, uint32_t wt)
{
    xTimerStop(tm, pdMS_TO_TICKS(wt));
    return xTimerDelete(tm, pdMS_TO_TICKS(wt));
}

/*************************************************************************************************
  * @brief    : timer_change_period
  * @param1   : ptimer_t:  timer handle;
  * @param2   : int: waiting time to change a timer period
  * @return   : void
  * @Instance : timer_change_period(testTimerExample,0,0);;
  * @Note     :
*************************************************************************************************/
void bat_timer_change_period(ptimer_t tm, uint32_t ms, uint32_t wt)
{
    xTimerChangePeriod(tm, pdMS_TO_TICKS(ms), pdMS_TO_TICKS(wt));
}

/*************************************************************************************************
  * @brief    : bat_timer_reset_ext
  * @param1   : ptimer_t:  timer handle;
  * @param2   : int: time to wakeup
  * @param3   : int: waiting time to start a timer
  * @return   : void
  * @Instance : bat_timer_reset_ext(testTimerExample,0);
  * @Note     :
*************************************************************************************************/
ptimer_t bat_timer_reset_ext(ptimer_t tm, char* timer_name, uint32_t ms, uint32_t type, TIMER_CB_F callback)
{
    if (tm) {
        xTimerStop(tm, pdMS_TO_TICKS(portMAX_DELAY));
        if(xTimerDelete(tm, pdMS_TO_TICKS(portMAX_DELAY)) == pdFAIL){
            return NULL;
        }
    }
    return xTimerCreate(timer_name, pdMS_TO_TICKS(ms), type, NULL, callback);
}

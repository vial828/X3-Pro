/*
 * kernel.c
 *
 *  Created on: 2019
 *      Author: Administrator
 */
#include <stdint.h>

#include "kernel.h"
#include <string.h>

static uint32_t tick = 0;
/*************************************************************************************************
 * @brief    :get system tick (ms)
 * @return   :tick
*************************************************************************************************/
uint32_t GetTick(void)
{
    tick = xTaskGetTickCount();
	return tick;
}

/*************************************************************************************************
 * @brief    :get the passed tick since mark tick
 * @param    :mark tick
 * @return   :passed tick
*************************************************************************************************/
uint32_t TicsSince( uint32_t mark)
{
	return tick - mark;
}

/*************************************************************************************************
 * @brief    :Reads the state and parameters of the process
 * @param    :
 * @return   :
*************************************************************************************************/
void printAllTaskStatus(void)
{
        uint32_t TotalRunTime = 0;
        uint32_t ArraySize,x;


        ArraySize=uxTaskGetNumberOfTasks();   // get task number
        TaskStatus_t StatusArray[ArraySize];

        LOGI("T_num, T_Name, T_currStu, T_prioty, T_stackLeft");
        ArraySize = uxTaskGetSystemState(StatusArray,(UBaseType_t)ArraySize,(uint32_t*)&TotalRunTime);
        for(x= (UBaseType_t)0U;x<ArraySize;x++)
        {
                        LOGI("%d, %s, %d, %d, %d",
                        x,StatusArray[x].pcTaskName,StatusArray[x].eCurrentState,
                        StatusArray[x].uxCurrentPriority,StatusArray[x].usStackHighWaterMark);
        }
}

/*************************************************************************************************
 * @brief    :Reads runtime for the process
 * @param    :
 * @return   :
*************************************************************************************************/
uint32_t volatile run_timer_cnt = 0;
void os_run_timer_init(void)
{
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER4);
    /* enable the peripherals clock */
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    /* deinit a TIMER */
    timer_deinit(TIMER4);

    /* TIMER4 configuration */
    timer_initpara.prescaler         = 63;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = 10-1;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER4, &timer_initpara);

    timer_interrupt_flag_clear(TIMER4, TIMER_INT_UP);
    timer_interrupt_enable(TIMER4, TIMER_INT_UP);
    nvic_irq_enable(TIMER4_IRQn, 3, 0);
    timer_enable(TIMER4);
}

void TIMER4_IRQHandler(void)
{
    if(timer_interrupt_flag_get(TIMER4, TIMER_INT_UP) != RESET){
        timer_interrupt_flag_clear(TIMER4, TIMER_INT_UP);
        run_timer_cnt++;
     }
}

uint32_t os_run_timer_cnt_get(void)
{
    return run_timer_cnt;
}

void monitor_func(void *para)
{
    static  uint8_t index = 0;
    static  char  monitor_log[512];
    static  char  monitor_log1[96];
    static  char  monitor_log2[96];
    static  char  monitor_log3[96];

    while(1){
        vTaskDelay(2000 * portTICK_RATE_MS);
        if(configGENERATE_RUN_TIME_STATS == 1){
            vTaskGetRunTimeStats((char *)monitor_log);
        }
        LOGI("task_name,run_time/10us,percentage");

        memcpy(monitor_log1, monitor_log,75);
        memcpy(monitor_log2, &monitor_log[75], 75);
        memcpy(monitor_log3, &monitor_log[150], 75);

        LOGI("%s",monitor_log1);
        LOGI("%s",monitor_log2);
        LOGI("%s",monitor_log3);

        if(index % 5 == 0){
            printAllTaskStatus();
            index = 0;
        }
        index++;
    }
}

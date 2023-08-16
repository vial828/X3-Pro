
#include "HWI_Hal.h"
void hwi_HAL_Delay(uint32_t Delay)
{
	hwi_delay_ms(Delay);
}

void hwi_delay_ms(uint32_t duration)
{
  TIMER_CNT(TIMER2)=0;
  while(TIMER_CNT(TIMER2)<duration*1000);   
}

void  hwi_delay_us(uint32_t duration)
{
  TIMER_CNT(TIMER2)=0;
  while(TIMER_CNT(TIMER2)<duration); 
}


void hwi_HAL_Init(void)
{
	icache_enable();
	
	hwi_RtcEnableDomainRegisterInit();	
	//hwi_power_standby_mode();	

	delay_init();	

	
	//hwi_SysTick_Config();


//	hwi_IWDG_Init();
}

void hwi_SystemClock_Config(void)
{
	SystemInit();
}

void delay_init(void)
{
    timer_parameter_struct timer_initpara;
    rcu_periph_clock_enable(RCU_TIMER2);	
    /* enable the peripherals clock */
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);

    /* deinit a TIMER */
    timer_deinit(TIMER2);
    /* initialize TIMER init parameter struct */
    timer_struct_para_init(&timer_initpara);
    /* TIMER2 configuration */
    timer_initpara.prescaler        = 67;
    timer_initpara.alignedmode      = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period           = 10000000;
    timer_initpara.clockdivision    = TIMER_CKDIV_DIV1;
    timer_init(TIMER2, &timer_initpara);

    /* clear interrupt bit */
    timer_flag_clear(TIMER2, TIMER_INT_FLAG_UP);

    /* enable a TIMER */
    timer_enable(TIMER2);  
}
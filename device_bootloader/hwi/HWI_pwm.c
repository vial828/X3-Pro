#include "pwm_ll.h"


uint32_t g_au32PwmFreq[] = {
		200000,
		220000,
};

void hwi_pwm_set_duty(pwm_out_enum cell,uint8_t duty)
{
  uint32_t handle,Channel;
//  if(duty>65535)
//  {
//    duty=65535;
//  }
  if(cell==pwm_htren1)
  {
	 handle=TIMER3;
	 Channel=TIMER_CH_1;
  }
  else if (cell==pwm_htren2)
  {
	 handle=TIMER0;
	 Channel=TIMER_CH_3;  
  }
  
  timer_channel_output_pulse_value_config(handle,Channel,duty*TIMER_CAR(handle)/100);
  //timer_event_software_generate(handle,TIMER_EVENT_SRC_UPG);
}

void hwi_set_pwm_htr_clock(uint16_t freq, uint8_t coil_num)
{

}

void hwi_pwm_output_init_usr(void)
{

	//PB7:TIMER3_CH1
	{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
    
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    rcu_periph_clock_enable(RCU_TIMER3);

    timer_deinit(TIMER3);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = SystemCoreClock/g_au32PwmFreq[0];
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER3,&timer_initpara);

    /* CH0,CH1 and CH2 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER3,TIMER_CH_1,&timer_ocintpara);

    /* CH1 configuration in PWM mode0,duty cycle 25% */
    timer_channel_output_pulse_value_config(TIMER3,TIMER_CH_1,0);
    timer_channel_output_mode_config(TIMER3,TIMER_CH_1,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER3,TIMER_CH_1,TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER3);
    /* auto-reload preload enable */
    timer_enable(TIMER3);
	}
	
		//PB12:TIMER0_CH3
	{
    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;
    
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    rcu_periph_clock_enable(RCU_TIMER0);

    timer_deinit(TIMER0);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = SystemCoreClock/g_au32PwmFreq[1];
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(TIMER0,&timer_initpara);

    /* CH0,CH1 and CH2 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

    timer_channel_output_config(TIMER0,TIMER_CH_3,&timer_ocintpara);

    /* CH3 configuration in PWM mode0,duty cycle 25% */
    timer_channel_output_pulse_value_config(TIMER0,TIMER_CH_3,0);
    timer_channel_output_mode_config(TIMER0,TIMER_CH_3,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER0,TIMER_CH_3,TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(TIMER0);
	timer_primary_output_config(TIMER0, ENABLE);
    /* auto-reload preload enable */
    timer_enable(TIMER0);
	}
		
}
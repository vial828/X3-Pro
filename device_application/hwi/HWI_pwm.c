#include "dev_pwm_ll.h"

#define  HTR_EN1_PWM_FREQ_DEFAULT  200000
#define  HTR_EN2_PWM_FREQ_DEFAULT  220000
#define  HAPTIC_PWM_FREQ_DEFAULT  20000

void hwi_pwm_set_duty(pwm_out_enum cell,uint8_t duty)
{
    uint32_t handle = 0, Channel = 0;
//  if(duty>65535)
//  {
//    duty=65535;
//  }
//Due to structural problems, temporarily exchange pwm_htren1 and pwm_htren2
    if(cell==pwm_htren1)          //PB0:TIMER2_CH1
    {
       handle=TIMER2;
       Channel=TIMER_CH_1;
    }
    else if (cell==pwm_htren2)    //PB8:TIMER3_CH2
    {
       handle=TIMER3;
       Channel=TIMER_CH_2;
    }
    else if (cell==pwm_haptic)    //PB10:TIMER0_CH1
    {
       handle=TIMER0;
       Channel=TIMER_CH_1;
    }
    else
    {
       return;
    }


  timer_channel_output_pulse_value_config(handle,Channel,duty*TIMER_CAR(handle)/100);
  //timer_event_software_generate(handle,TIMER_EVENT_SRC_UPG);
}

void hwi_set_pwm_htr_clock(pwm_out_enum cell,uint32_t freq)
{
    uint32_t handle = 0, Channel = 0;
    rcu_periph_enum clk_name = RCU_TIMER0;
    if(cell==pwm_htren1)//PB0:TIMER2_CH1
    {
         handle = TIMER2;
         Channel = TIMER_CH_1;
         clk_name = RCU_TIMER2;
    }
    else if (cell==pwm_htren2)//PB8:TIMER3_CH2
    {
         handle = TIMER3;
         Channel = TIMER_CH_2;
         clk_name = RCU_TIMER3;
    }
    else if (cell==pwm_haptic)//PB10:TIMER0_CH1
    {
         handle = TIMER0;
         Channel = TIMER_CH_1;
         clk_name = RCU_TIMER0;
    }
    else
    {
         return;
    }

    //timer_deinit(handle);

    timer_oc_parameter_struct timer_ocintpara;
    timer_parameter_struct timer_initpara;

    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL4);
    rcu_periph_clock_enable(clk_name);

    /* TIMER1 configuration */
    timer_initpara.prescaler         = 0;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection  = TIMER_COUNTER_UP;
    timer_initpara.period            = SystemCoreClock/freq;
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0;
    timer_init(handle,&timer_initpara);
    
   // if(cell== pwm_htren1 || cell== pwm_htren2 || cell== pwm_haptic){
    /* CH0,CH1 and CH2 configuration in PWM mode */
    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(handle,Channel,&timer_ocintpara);
//    }
//    else{
//    timer_ocintpara.outputstate  = TIMER_CCX_ENABLE;
//    timer_ocintpara.outputnstate = TIMER_CCXN_DISABLE;
//    timer_ocintpara.ocpolarity   = TIMER_OC_POLARITY_LOW;
//    timer_ocintpara.ocnpolarity  = TIMER_OCN_POLARITY_LOW;
//    timer_ocintpara.ocidlestate  = TIMER_OC_IDLE_STATE_HIGH;
//    timer_ocintpara.ocnidlestate = TIMER_OCN_IDLE_STATE_HIGH;
//    timer_channel_output_config(handle,Channel,&timer_ocintpara);
//    }
    /* CH1 configuration in PWM mode0,duty cycle 25% */
    timer_channel_output_pulse_value_config(handle,Channel,0);
    timer_channel_output_mode_config(handle,Channel,TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(handle,Channel,TIMER_OC_SHADOW_ENABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(handle);
    if(handle == TIMER0)
    {
        timer_primary_output_config(handle, ENABLE);
		}
    /* auto-reload preload enable */
    timer_enable(handle);
}

void hwi_pwm_output_init_usr(void)
{
    hwi_set_pwm_htr_clock(pwm_htren1,HTR_EN1_PWM_FREQ_DEFAULT);
    hwi_set_pwm_htr_clock(pwm_htren2,HTR_EN2_PWM_FREQ_DEFAULT);

    hwi_set_pwm_htr_clock(pwm_haptic,HAPTIC_PWM_FREQ_DEFAULT);
}

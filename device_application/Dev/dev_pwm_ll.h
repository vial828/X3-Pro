#ifndef __PWM_LL_H
#define __PWM_LL_H


typedef enum
{
    pwm_haptic,
    pwm_dac,
    pwm_htren1,
    pwm_htren2,
//    pwm_htren3,
//    pwm_htren4,
    pwm_led1,
    pwm_led2,
    pwm_led3,
    pwm_led4,
}pwm_out_enum;

#include "HWI_Hal.h"

void dev_pwm_output_init_usr(void);
void dev_pwm_set_duty(pwm_out_enum cell,uint8_t duty);
void dev_set_pwm_htr_clock(pwm_out_enum cell, uint16_t freq);
#endif


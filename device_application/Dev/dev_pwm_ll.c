#include "dev_pwm_ll.h"

#define SYS_CLOCK_FREQ     (64 * 1000 * 1000ul) /*64M*/

#define TIM3_FREQ_LED      1000
#define TIM2_FREQ_LED      1000


/*----------------------------- USR --------------------------------*/
void dev_pwm_output_init_usr(void)
{
    hwi_pwm_output_init_usr();
}

void dev_pwm_set_duty(pwm_out_enum cell,uint8_t duty)
{
    hwi_pwm_set_duty(cell, duty);
}

void dev_set_pwm_htr_clock(pwm_out_enum cell, uint16_t freq)
{
    hwi_set_pwm_htr_clock(cell, 1000*freq);
}



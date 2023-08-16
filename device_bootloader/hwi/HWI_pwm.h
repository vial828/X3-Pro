#ifndef __HWI_PWM_H
#define __HWI_PWM_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "pwm_ll.h"


/***************************************  config - usr  *****************************************/
//extern cyhal_pwm_t pwm_heat1_ctrl;
//extern cyhal_pwm_t pwm_heat2_ctrl;

/****************function****************/
void hwi_pwm_set_duty(pwm_out_enum cell,uint8_t duty);
void hwi_set_pwm_htr_clock(uint16_t freq, uint8_t coil_num);
void hwi_pwm_output_init_usr(void);


#ifdef __cplusplus
}
#endif

#endif


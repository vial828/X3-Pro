#ifndef __HAPTIC_H
#define __HAPTIC_H

#include "stdint.h"


#define HP_MAX_CYCLE_CNT    10
#define HP_MIN_ON_TIME      10  //10ms

//#define HP_MAX_INTENSITY_LEVEL        (17)
//#define HP_HALF_INTENSITY_LEVEL       (6)
//#define HP_INTENSITY_3V_LEVEL       (10)
//#define HP_INTENSITY_3_3V_LEVEL       (13)
#define PWM_FREQ_HAPTIC 20
#define HAPTIC_VOLT_DEFAULT 3300
#define HAPTIC_VOLT_MAX 3600


typedef struct
{
    uint16_t on_time;
    uint16_t off_time;
}one_cycle_buzz_t;

typedef struct
{
    uint8_t cycle_cnt;
    one_cycle_buzz_t one_cycle_buzz_s[HP_MAX_CYCLE_CNT];
}haptic_mode_t;

void app_haptic_enable(uint8_t en);
void app_haptic_shutdown(void);
void app_haptic_buzz(char mode);
//void haptic_buzz_A(void);
//void haptic_buzz_B(void);
//void haptic_buzz_C(void);
void app_haptic_set_parameter(uint8_t mode, uint8_t* pdata, uint16_t data_len);
//void haptic_set_A_para(uint16_t ms);
//void haptic_set_B_para(uint16_t ms, uint16_t gap);
//void haptic_set_C_para(uint16_t ms);
void app_haptic_restore_default(uint8_t ptype);
void app_set_haptic_intensity(uint16_t haptic_volt);
uint16_t app_get_haptic_intensity(void);
#endif


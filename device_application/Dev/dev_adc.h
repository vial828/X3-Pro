#ifndef __ADC_H
#define __ADC_H

#include "HWI_Hal.h"

typedef enum
{

    I_SENSE,          /*PA0   ch:0*/
  //COIL_TEMPB,         /**/
    USB_TEMP,           /*PA1   ch:1*/
    BAT_TEMP,          /*PA2   ch:2*/
    COIL_TEMP,           /*PA3   ch:3*/
    VBAT_VOLT,              /*PC0   ch:4*/
    COLD_JUNC,            /*PC1   ch:5*/
    TC1,                /*PC2   ch:6*/
//  BAT_TEMP_1,         /**/
    TC2,                /*PC3   ch:7*/

  //  mcu_temperature,    /*internal ch:9*/
    vref,               /*internal ch:10*/
    HW_ID,          /*PA4   ch:8*/

    ADC_CH_NUM
}adc_ch_e;
typedef struct {
    float vbat;
    float coil_temp;
    float bat_temp;
    float cold_junc_temp;
    float usb_temp;
    float i_sense;
    float zone1_temp;
    float zone2_temp;
}adc_context_st;


void dev_adc_init(void);
void dev_adc_task(void);
float dev_get_voltage(adc_ch_e adc_ch);
uint16_t dev_get_adc(adc_ch_e adc_ch);
uint16_t get_average_adc(adc_ch_e adc_ch);
//uint16_t get_bat_ID(void);
//float get_vbus_volt(void);
float dev_get_vbat_volt(void);
float dev_get_i_sense(void);
//float get_i_bat(void);
uint32_t dev_get_random_seed(void);
float dev_get_isense_peak(void);
void dev_cal_adc_result(void);
adc_context_st* dev_get_adc_result(void);
void cal_i_sense_b_RE(void);
void record_g_b_adc(void);

#endif


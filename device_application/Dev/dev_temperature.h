#ifndef __TEMPERATURE_H
#define __TEMPERATURE_H

#include "stdint.h"

typedef struct
{
    int16_t bat_temp;
    uint16_t bat_mv;
    int16_t zone1_temp;
    int16_t zone2_temp;
    int16_t usb_temp;
    int16_t i_sense_ma;
    int16_t coil_temp;
    int16_t coil_junc_temp;
    uint16_t pwm_dac;
    int16_t tc1_temp;
    int16_t tc2_temp;
    uint16_t bat_id;
    uint16_t gas_soc;
    uint16_t phy_val_get_pos;
}physical_value_s;


typedef enum
{
    BAT_TEMP_E = 0,
    BAT_V_E,
    ZONE1_TEMP_E,
    ZONE2_TEMP_E,
    USB_TEMP_E,
    I_SENSE_E,
    COIL_TEMP_E,
    COIL_JUNC_TEMP_E,
    PWM_DAC_E,
    TC1_TEMP_E,
    TC2_TEMP_E,
    BAT_ID_E,
    GAS_SOC_E,
    DEL_ALL = 0xFF
}physical_value_e;

typedef struct{
    int16_t temp_value;
    float   ntc_ohm;
}cic_ntc_map_t;
void test_convert_map(void);
float dev_get_coil_temp(void);
float dev_get_cold_junc_temp(void);
float dev_get_usb_temp(void);
float dev_get_bat_temp(void);
float dev_get_tc_temp(uint8_t num);
float dev_get_zone_temp(uint8_t num);
float get_coil_board_temp(void);
void dev_print_all_adc(void);
uint16_t dev_get_phy_val_pos(void);
int16_t dev_get_phy_val(physical_value_e phy_val_e);
int16_t dev_convert_ntc_ohm(const cic_ntc_map_t *pmap, uint16_t len, float ohm);
float dev_get_bat_temp_gauge(void);

extern physical_value_s phy_value;

#endif



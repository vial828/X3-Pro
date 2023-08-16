#ifndef __ERROR_CODE_H
#define __ERROR_CODE_H
#include "error_code_led_output.h"

/* 111111111111111100000000 */
#define CONTINUOUS_MASK 0xFFFFFF00

/*ERROR CODE 20201209*/
typedef enum{
	disable_s = 0,
	enable_s = 1,
}error_check_e;

/*oneshot*/
#define FLT_DE_BAT_HOT_40_PRE_SES           (1u<<flt_de_bat_hot_40_pre_ses)
#define WAR_DE_BAT_EMPTY                    (1u<<war_de_bat_empty)
#define WAR_DE_BAT_LOW                      (1u<<war_de_bat_low)
#define FLT_DE_TC_ZONE_IMBALANCE            (1u<<flt_de_tc_zone_imbalance)
#define FLT_DE_TC_SPIKE                     (1u<<flt_de_tc_spike)
//#define FLT_DE_TC_HIGH_CJR                  (1u<<flt_de_tc_high_cjr)
#define FLT_DE_TC_ZONE1_HEATING_ABNORMAL    (1u<<flt_de_tc_zone1_heating_abnormal)
#define FLT_DE_TC_ZONE2_HEATING_ABNORMAL    (1u<<flt_de_tc_zone2_heating_abnormal)
#define FLT_DE_BAT_I_SENSE_DAMAGE           (1u<<flt_de_bat_i_sense_damage)
//#define WAR_DE_TC_B2B_OVERHEAT              (1u<<war_de_tc_b2b_overheat)

/*continuous*/
#define FLT_DE_BAT_DAMAGE                   (1u<<flt_de_bat_damage)
#define FLT_DE_CIC_OUTPUT_VOLTAGE           (1u<<flt_de_cic_output_voltage)
#define FLT_DE_END_OF_LIFE                  (1u<<flt_de_end_of_life)
#define FLT_DE_BAT_DISCHARGE_CURRENT_OVER   (1u<<flt_de_bat_discharge_current_over)
#define FLT_DE_BAT_CHARGE_CURRENT_OVER      (1u<<flt_de_bat_charge_current_over)
#define FLT_DE_CIC_CONFIG_ERROR             (1u<<flt_de_cic_config_error)
#define FLT_DE_BAT_HOT_55                   (1u<<flt_de_bat_hot_55)
#define FLT_DE_BAT_COLD_HEAT                (1u<<flt_de_bat_cold_heat)
#define FLT_DE_TC_ZONE1_HOT                 (1u<<flt_de_tc_zone1_hot)
#define FLT_DE_TC_ZONE2_HOT                 (1u<<flt_de_tc_zone2_hot)
#define FLT_DE_USB_HOT                      (1u<<flt_de_usb_hot)
#define FLT_DE_CO_HOT_SW                    (1u<<flt_de_co_hot_sw)
#define FLT_DE_BAT_COLD_CHARGE              (1u<<flt_de_bat_cold_charge)
#define FLT_DE_BAT_VOLTAGE_OVER             (1u<<flt_de_bat_voltage_over)
#define FLT_DE_CO_JUNC_HOT                  (1u<<flt_de_co_junc_hot)
#define FLT_DE_BAT_HOT_50_CHARGING          (1u<<flt_de_bat_hot_50_charging)
//#define FLT_DE_CIC_CHARGE_TIMEOUT           (1u<<flt_de_cic_charge_timeout)
//#define FLT_DE_HW_CO_I_TC1_TC2_ERROR        (1u<<flt_de_hw_co_i_tc1_tc2_error)
//#define FLT_DE_CO_COLD                      (1u<<flt_de_co_cold)
//#define FLT_DE_USB_COLD                     (1u<<flt_de_usb_cold)
//#define FLT_DE_TC_ZONE1_COLD                (1u<<flt_de_tc_zone1_cold)
//#define FLT_DE_TC_ZONE2_COLD                (1u<<flt_de_tc_zone2_cold)
//#define FLT_DE_CIC_INPUT_VOLTAGE            (1u<<flt_de_cic_input_voltage)
//#define FLT_DE_CIC_TEMP_OUT_OF_RANGE        (1u<<flt_de_cic_temp_out_of_range)
//#define WAR_DE_CIC_COLD                     (1u<<war_de_cic_cold)
//#define WAR_DE_CIC_HOT                      (1u<<war_de_cic_hot)
//#define WAR_DE_CO_JUNC_COLD                 (1u<<war_de_co_junc_cold)


void read_error_parameter(void);
void heating_off_pin_init(void);
void error_eol_check(void);
void check_charge_task(void);
void check_heat_task(void);
void error_check_task(void);
void set_error_check_status(error_check_e status);
error_check_e get_error_check_status(void);
void check_cic_config_error(void);

#endif

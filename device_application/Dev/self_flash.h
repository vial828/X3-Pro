#ifndef __SELF_FLASH_H
#define __SELF_FLASH_H

#include "HWI_Hal.h"
#include "stdint.h"
#include "app_heat.h"
#include "app_haptic.h"

#define STICK_SENSOR_ON  1
#define STICK_SENSOR_OFF 0

#define EXTEND_UI_ON   1
#define EXTEND_UI_OFF  0

#define SN_LEN   14

#define USR_DATA                   0
#define DATA_CHANGE_FREQUENT       1
#define BOOT_RECORD                2

#define SESSION_DATA                1
#define NONE_SESSION_DATA           0

#define INVALID                     0

#define POWERON_COLD_BOOT   0
#define POWERON_BUTTON_WAKEUP   1
#define POWERON_USB_WAKEUP  2
#define POWERON_OBLRST  3
#define POWERON_IWDGRST  4
#define POWERON_SFTRST  5
#define POWERON_HALL_WAKEUP  6


#define USER_DATA_FLASH_PAGES_NUM     1
#define FREQUENT_DATA_FLASH_PAGES_NUM 4
#define FREQUENT_DATA_FLASH_ALL_PAGE 0xFF
#define RECORD_TIMES_WHEN_ERROR     10

#define INI_VERSION "BAT_V05_202307311"
#define PRE_SESSION_TEMP_LIMIT     49
#define BAT_HOT_CHARGING   45
#define BAT_HOT_CHARGE_STEP2   43
#define BAT_HOT_PROTECT_THRESHOLD   43
#define BAT_HOT_PROTECT_THRESHOLD_STEP2   41
#define BAT_HOT_PROTECT_RELEASE   40
#define BAT_HOT_CHARGING_CLEAR   40
#define BAT_COLD_CHARGE_TEMP   1
#define BAT_COLD_CHARGE_TEMP_CLEAR   3
#define BAT_COLD_HEAT_TEMP   -8
#define BAT_COLD_HEAT_TEMP_CLEAR   -5
#define BAT_HOT_TEMP 58
#define BAT_HOT_TEMP_CLEAR 46
#define PRE_HEAT_CUTOFF_VOLT     3680
#define PRE_HEAT_CUTOFF_VOLT_SOC 6
#define DURING_SESSION_EMPTY_VOLT     3000
#define BAT_VOLT_DAMAGE_PROTECT       2300
#define TC_ZONE1_HOT     325
#define TC_ZONE1_HOT_CLEAR     323
#define TC_ZONE2_HOT     325
#define TC_ZONE2_HOT_CLEAR     323
//#define TC_ZONE1_COLD     -12
//#define TC_ZONE1_COLD_CLEAR     -8
//#define TC_ZONE2_COLD     -12
//#define TC_ZONE2_COLD_CLEAR     -8
#define BAT_VOLTAGE_OVER     4450
#define BAT_VOLTAGE_OVER_CLEAR     4350
#define BAT_DISCHARGE_CURR_OVER     8000
#define BAT_CHARGE_CURR_OVER     4000
#define CO_JUNC_HOT     80
#define CO_JUNC_HOT_CLEAR     60
//#define CO_JUNC_COLD     -12
//#define CO_JUNC_COLD_CLEAR     -8
#define BAT_I_SENSE_DAMAGE     1000
#define CHARGE_TIMEOUT     480
#define COIL_HOT_TEMP     100
#define COIL_HOT_TEMP_CLEAR     80
#define USB_HOT_TEMP     70
#define USB_HOT_TEMP_CLEAR     55
//#define USB_COLD_TEMP    -12
//#define USB_COLD_TEMP_CLEAR    -8
#define EOL_DEFAULT              10000
#define STEP1_NUMS_DEFAULT              2000
#define STEP2_NUMS_DEFAULT              4000
#define STEP3_NUMS_DEFAULT              6000
#define STEP4_NUMS_DEFAULT              8000
#define SLOW_CHG_DEFAULT_MA      350
#define SLOW_DEFAULT_H_BATV    4300
#define SLOW_DEFAULT_L_BATV    3000
#define WRONG_CHG_H_MV           11000

typedef enum
{
    NO_UPDATE_INI_VER = 0,
    UPDATE_INI_VER = 1,
    ERR_INI_VER = 2,
}INI_VER_STATUS;

typedef struct
{
    int16_t pre_session_temp_limit;
    int16_t charge_temp_limit;
    int16_t charge_temp_limit_clear;
    int16_t bat_cold_charge_temp;
    int16_t bat_cold_charge_temp_clear;
    int16_t bat_hot_temp;
    int16_t bat_hot_temp_clear;
    int16_t bat_cold_heat_temp;
    int16_t bat_cold_heat_temp_clear;
    int16_t heat_empty_volt;
    int16_t heat_cutoff_volt;
    int16_t heat_cutoff_volt_soc;
    int16_t bat_volt_damage_protect;
    int16_t tc_zone1_hot;
    int16_t tc_zone1_hot_clear;
    int16_t tc_zone2_hot;
    int16_t tc_zone2_hot_clear;
//    int16_t tc_zone1_cold;
//    int16_t tc_zone1_cold_clear;
//    int16_t tc_zone2_cold;
//    int16_t tc_zone2_cold_clear;
    int16_t bat_volt_over;
    int16_t bat_volt_over_clear;
    int16_t discharge_current_over;
    int16_t charge_current_over;
    int16_t co_junc_hot;
    int16_t co_junc_hot_clear;
//    int16_t co_junc_cold;
//    int16_t co_junc_cold_clear;
    int16_t i_sense_damage;
    int16_t charge_timeout;
    int16_t coil_hot_temp;
    int16_t coil_hot_temp_clear;
    int16_t usb_hot_temp;
    int16_t usb_hot_temp_clear;
//    int16_t usb_cold_temp;
//    int16_t usb_cold_temp_clear;

}error_parameter_t;


typedef enum{
    max_bat_chg_vol = 0,
    max_bat_chg_current,
    max_susceptor_temp1_life,
    max_susceptor_temp2_life,
    max_bat_temp_life,
    total_chg_time,
    max_chg_time,
    chag_times,
    power_on_reason,
    session_starts,
    sessions_finished_normally,
    sessions_aborted,
    chag_sessions_begin_dark,
    chag_sessions_got_full_battery,
    button_presses,
    hall_toggle,
    in_WH,
    out_WH,
    total_heat_time,
    life_cycle_log_max
}life_cycle_e;

typedef enum{
    /*oneshot*/
    flt_de_bat_hot_40_pre_ses = 0,
    war_de_bat_empty,
    war_de_bat_low,
    flt_de_tc_zone_imbalance,
    flt_de_tc_spike,
//    flt_de_tc_high_cjr,
    flt_de_tc_zone1_heating_abnormal,
    flt_de_tc_zone2_heating_abnormal,
    flt_de_bat_i_sense_damage,
//    war_de_tc_b2b_overheat,
    /*continuous,return*/
    flt_de_bat_damage,
    flt_de_cic_output_voltage,
    flt_de_end_of_life,
    /*continuous,reset*/
    flt_de_bat_discharge_current_over,
    flt_de_bat_charge_current_over,
    flt_de_cic_config_error,
    /*continuous,wait*/
    flt_de_bat_hot_55,
    flt_de_bat_cold_heat,
    flt_de_tc_zone1_hot,
    flt_de_tc_zone2_hot,
    flt_de_usb_hot,
    flt_de_co_hot_sw,
    flt_de_bat_cold_charge,
    flt_de_bat_voltage_over,
    flt_de_co_junc_hot,
    flt_de_bat_hot_50_charging,
//    flt_de_cic_charge_timeout,
//    flt_de_co_cold,
//    flt_de_usb_cold,
//    flt_de_tc_zone1_cold,
//    flt_de_tc_zone2_cold,
//    war_de_co_junc_cold,
//    flt_de_hw_co_i_tc1_tc2_error,
    /*charge IC*/
//    flt_de_cic_input_voltage,
//    flt_de_cic_temp_out_of_range,
//    war_de_cic_cold,
//    war_de_cic_hot,
    error_max
}errorCode_e;


#pragma pack(1)
typedef struct
{
    uint8_t bootloader_version[32];
    uint64_t app_update_flag;
    uint64_t app_file_size;
    char app_file_name[48];
    uint8_t app_signature[32];

    /* Serial Comm lock flag*/
    uint32_t comm_lock_flag;

    /* error record */
    uint16_t return_err;
    uint16_t reset_err;
    uint32_t error_pos;
}boot_record_t;
#pragma pack()

#pragma pack(1)
typedef struct
{
    uint8_t sn[SN_LEN];
    uint8_t smt_sn[SN_LEN];
    uint32_t gauge_version;
    uint16_t g_b_adc;
    uint8_t ini_version[32];
    uint8_t n_heat_name[32];
    heat_profile_t zone1_n;
    heat_profile_t zone2_n;
    uint8_t b_heat_name[32];
    heat_profile_t zone1_b;
    heat_profile_t zone2_b;
    uint16_t pwm_dac_duty;
    uint8_t pwm_htr_duty_coil1;
    uint8_t pwm_htr_duty_coil2;
    uint16_t pwm_htr_clock_coil1;
    uint16_t pwm_htr_clock_coil2;
    uint16_t haptic_pwm_freq;
    /*haptic buzz*/
    haptic_mode_t haptic_a;
    haptic_mode_t haptic_b;
    haptic_mode_t haptic_c;
    haptic_mode_t haptic_d;

    uint16_t haptic_volt;
    uint32_t b2b_batt_temp_limit;
    uint16_t  stick_sensor_status;
    int16_t charge_temp_protect;
    int16_t charge_temp_protect_relesae;
    uint8_t extend_mode;
    uint16_t eol_session;
    uint16_t step1_session_nums;
    uint16_t step2_session_nums;
    uint16_t step3_session_nums;
    uint16_t step4_session_nums;
    uint16_t slow_chg_isense;
    uint16_t slow_batv_h;
    uint16_t slow_batv_l;
    uint16_t wrong_chg_h_mv;
    uint16_t step1_chg_curr;
    uint16_t step1_chg_volt;
    uint16_t step2_chg_curr;
    uint16_t step2_chg_volt;
    uint16_t step3_chg_curr;
    uint16_t step3_chg_volt;
    uint16_t step4_chg_curr;
    uint16_t step4_chg_volt;
    uint16_t step2_chg_hot_limit;
    uint16_t step2_chg_hot_protect;
    error_parameter_t error_parameter;

    //uint16_t reserve[1];//Used for byte completion, the struct remains divisible by 4; note the number of int16 in the argument
    uint64_t flag_crc;
}flash_record_t;
#pragma pack()

typedef enum{
	session_duration = 0,
	max_susceptor_temp1_session,
	max_susceptor_temp2_session,
	max_bat_temp_session,
	max_cold_junc_temp_session,
	session_log_max
}session_e;

typedef struct
{
    float num[session_log_max];
}session_t;

typedef struct{
    uint32_t vbat_adc;
    uint32_t coil_t_adc;
    uint32_t bat_t_adc;
    uint32_t cold_junc_t_adc;
    uint32_t usb_t_adc;
    uint32_t i_sense_adc;
    uint32_t tc1_adc;
    uint32_t tc2_adc;
    uint32_t CHG_IC_REG_0B;
    uint32_t CHG_IC_REG_0C;
}records_when_error_s;

typedef struct{
    uint32_t completed_sessions;
    uint32_t aborted_sessions;
    uint32_t b2b_sessions;
}records_session_data_s;

#pragma pack(1)
typedef struct{
    uint32_t errorCodeData[error_max];
    int32_t lifeCycleData[life_cycle_log_max];
    records_when_error_s records_when_err[RECORD_TIMES_WHEN_ERROR];
    uint32_t g_sessionNum;
    uint32_t g_sessionWritePos;
    session_t session[25];
    uint32_t serialNum;
    uint32_t programeNum;
    records_session_data_s  session_data;
    uint32_t reserve;
    uint32_t last_cleaning_num;
    uint64_t crc;
}data_change_frequent_t;
#pragma pack()

boot_record_t* get_boot_record_from_flash(void);
boot_record_t* get_boot_record_from_ram(void);
flash_record_t* get_self_flash_record(void);
flash_record_t* get_self_flash_record_from_ram(void);
data_change_frequent_t* get_data_change_frequent_from_flash(void);
data_change_frequent_t* get_data_change_frequent_from_ram(void);

session_t* get_100sessions_from_ram(void);
void write_life_cycle_value_to_ram(life_cycle_e type, int32_t num);
void write_session_to_ram(session_e type, float value);
void write_error_occur_num_to_ram(errorCode_e error_code, uint16_t num);

void update_data_flash(uint8_t dataType, uint8_t sessionDataFlag);
void record_life_cycle_task(void);
void erase_frquent_data_flash(uint8_t pageNum);
void erase_data_flash(void);
void test_error_code_read_write(void);
void test_lifeCycle_read_write(void);
void test_100sessions_read_write(void);

#endif


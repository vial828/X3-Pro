#ifndef __HEAT_H
#define __HEAT_H

#include "stdint.h"
#include "stratos_defs.h"

#define BEFORE_FINISH_TIME   (10*1000)        //10s before end
#define SESSION_EXTEND_PERIOD_START   (60*1000)        //60s before end
#define SESSION_EXTEND_PERIOD   (30*1000)

#define MAX_HEAT_PARA_LEN   31
#define HEAT_ONE_CNT_TIME   16


#if defined (HTR_EN_1) && defined (HTR_EN_2)
    #define NH_WARMUP_FINISH_TIME (20*1000)  // 20s
    #define BH_WARMUP_FINISH_TIME (15*1000)    // 15s
#else
    #define NH_WARMUP_FINISH_TIME (30*1000)  // 30s
    #define BH_WARMUP_FINISH_TIME (20*1000)    // 20s
#endif

#if defined (HTR_EN_1) && defined (HTR_EN_2)
    #define PWM_FREQ_COIL1  500
    #define PWM_FREQ_COIL2  450
    #define PWM_DUTY_COIL1  50
    #define PWM_DUTY_COIL2  50
#else
    #define PWM_FREQ_COIL1  360
    #define PWM_FREQ_COIL2  0
    #define PWM_DUTY_COIL1  50
    #define PWM_DUTY_COIL2  0
#endif

typedef struct{
 uint16_t time;
 uint16_t temp;
}heat_para_t;

typedef struct{
 uint8_t count;
 heat_para_t heat_para[MAX_HEAT_PARA_LEN];
}heat_profile_t;

typedef struct{
 uint8_t g_chan;
 float log_i;
}heat_chan_t;

typedef struct{
    uint32_t session_duration;
    float max_susceptor_temp1;
    float max_susceptor_temp2;
    float max_bat_temp_session;
    float max_cold_junc_temp;
}heat_session_log_t;

uint8_t app_get_heat_state(void);
uint16_t app_get_heat_cnt(void);
uint8_t get_heat_lock_state(void);
void app_get_session_result_params(heat_session_log_t * ptr);
void app_heat_init(void);
void start_hlh(void);
void app_heat1_enable(uint8_t en);
void app_heat2_enable(uint8_t en);
void app_get_duty(void);
void app_parse_heat_profile_cmd(uint8_t *pdata, uint16_t len);
void app_get_heat_profile(uint8_t type, uint8_t zone, heat_profile_t *pZone);
void app_start_boost_heat(void);
void app_start_normal_heat(void);
void app_start_heat(char heat_type);
void app_stop_heat(void);
void app_during_session_errorCode_check(void);
void app_session_extend_function_on(void);
uint8_t app_get_session_extend_flag(void);
float app_get_heat_CJR(void);
int16_t app_get_pv_value(uint8_t number);
int16_t app_get_sp_value(uint8_t number);
heat_chan_t* app_get_chan_value(void);
uint16_t app_get_totalJ_value(void);
void app_start_tc_heat_test(uint8_t number,uint8_t time);
void app_start_quick_heat_test(void);
uint32_t app_get_base_heat_duration(void);
uint32_t app_get_boost_heat_duration(void);
void app_auto_clear_b2b_error(void);
uint8_t get_b2b_flag(void);
void app_set_default_heat_profile(heat_profile_t* pzone,uint8_t type, uint8_t zone);
#endif



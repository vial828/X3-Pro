#include <stdlib.h>
#include <string.h>
#include "HWI_Hal.h"
#include "kernel.h"
#include "app_heat.h"
#include "log.h"
#include "dev_pwm_ll.h"
#include "dev_temperature.h"
#include "dev_adc.h"
#include "self_flash.h"
#include "manager.h"
#include "error_code.h"
#include "usr_cmd.h"
#include "app_charge.h"
#include "HWI_gpio.h"
#include "stratos_defs.h"
#include "batTimer.h"
#include "app_oled_display.h"
#include "dev_gauge_bq27z561r2.h"
//#define NICO_HEAT

#define CMD_DATA_MIN    3
#define HEAT_CHECK_TIEM (15*1000/16)

typedef struct{
    uint32_t pre_heat_finish_tick;    //last heating complete tick
    uint32_t error_tick;       //the tick of b2b error occurred that caused by battery temperature over
    uint8_t  count;
    uint8_t  b2b_error_flag;
}b2b_t;

static b2b_t b2b = {
    0,0,0,0
};

//static timer_t * heat_unit_timer;
//static timer_t * heat_lock_timer;
static ptimer_t heat_unit_timer;
static ptimer_t heat_lock_timer;

heat_profile_t pzone1_prof;
heat_profile_t pzone2_prof;

uint32_t heat_all_time=260*1000;
uint32_t heat_all_cnt= 260*1000/HEAT_ONE_CNT_TIME;
int16_t heat_cnt=0;
//int16_t heat_frame=0;
int16_t heat_up_count_down = 0;
int16_t session_count_down = 0;
int16_t heat_coil1_checkout_cnt=0;
int16_t heat_coil2_checkout_cnt=0;
int16_t heat_zone1_stage=0;         /*coli1 heat stage*/
int16_t heat_zone2_stage=0;         /*coli2 heat stage*/
uint8_t coil1_start_state=0;
uint8_t coil2_start_state=0;
uint8_t coil1_need_heat=0;
uint8_t coil2_need_heat=0;
float total_joules=0.0;
float old_total_joules=0.0;
uint8_t heat_chan=0;
uint8_t now_pwm=0;
float zone1_temp=0;
float zone2_temp=0;
float old_zone1_temp=0;
float old_zone2_temp=0;
uint8_t heat_lock_state=0;
uint8_t heat_complete_state=0;
//float max_zone1_temp=0;
//float max_zone2_temp=0;
//float max_bat_temp=0;
//float max_cold_junc_temp=0;
uint8_t isense_damage_error_cnt=0;
//uint8_t tc_spike_error_cnt=0;
uint8_t heat_state=0;
uint8_t pwm_dac_duty=30;
float zone1Temp[32]={0};
float zone2Temp[32]={0};
float totalJoule[32]={0.0};
int16_t pv1_array[62]={0};
int16_t pv1_sum=0;
int16_t pv2_array[62]={0};
int16_t pv2_sum=0;
int16_t pv1,pv2;
uint8_t heat_enable_flag_1s=0;
uint8_t heat_times = 0;
uint32_t pre_heat_stop_time=0;
uint32_t next_heat_start_time=0;
uint8_t g_chan=0;
uint8_t g_session_extend_flag = NONE_SESSION_EXTEND;
//uint8_t  heat_warmup_session_flag = 0;
static heat_chan_t heat_chan_data;


uint8_t pwm_htr_duty_coil1=PWM_DUTY_COIL1;
uint8_t pwm_htr_duty_coil2=PWM_DUTY_COIL2;

extern float heat_cutoff_volt;
extern float heat_empty_volt;
extern int16_t pre_session_temp_limit;
extern float i_sense_damage;
extern int16_t heat_cutoff_volt_soc;

/*************************************************************************************************
  * @brief    : get base heat duration
  * @return   : time(ms)
*************************************************************************************************/
uint32_t app_get_base_heat_duration(void)
{
    return heat_all_time;
}
/*************************************************************************************************
  * @brief    : get boost heat duration
  * @return   : time(ms)
*************************************************************************************************/
uint32_t app_get_boost_heat_duration(void)
{
    return heat_all_time;
}
/*************************************************************************************************
  * @brief    : Get now heat state
  * @return   : 0:heat/1:on_heat
*************************************************************************************************/
uint8_t app_get_heat_state(void)
{
    return heat_state;
}
/*************************************************************************************************
  * @brief    : Get now heat how many counts(16ms)
  * @return   : heat counts
*************************************************************************************************/
uint16_t app_get_heat_cnt(void)
{
    return heat_cnt;
}
/*************************************************************************************************
  * @brief    : Get heat state if lock
  * @return   : 0:on_lock/1:lock
*************************************************************************************************/
uint8_t get_heat_lock_state(void)
{
    return heat_lock_state;
}

/*************************************************************************************************
 * @brief   :parse heat profie command from cli
 *  zone1 count ... zone2 count ....
    byte0 byte1 ... byte(2+2*zone1_count)
 * @param   :command pointer
 * @param   :command lenght
 * @return  :none
*************************************************************************************************/
void app_parse_heat_profile_cmd(uint8_t *pdata, uint16_t len)
{
    uint8_t zone1_count, zone2_count;
    uint16_t min_len;
    flash_record_t* pfrt = get_self_flash_record_from_ram();

    min_len = CMD_DATA_MIN; //min data len
    if(len < min_len)
    {
        LOGD("heat profile cmd min_len error\r\n");
        return;
    }
    if(pdata[1]!=1 && pdata[1]!=2){  //zone byte must be 0x01 or 0x02
        LOGD("heat profile cmd byte!=1,!=2 error\r\n");
        return;
    }

    if(pdata[1] == 1)
    {
        zone1_count = pdata[2]; //get zone1 section count
        //min_len = min_len+4*zone1_count;
        //if(len < min_len || zone1_count>MAX_HEAT_PARA_LEN){
        if(zone1_count>MAX_HEAT_PARA_LEN){
            LOGD("heat profile mcd zone1 count error\r\n");
            return;
        }

        /*normal heat profile*/
        if(pdata[0] == 'n')
        {
            LOGD("write normal heat porfile\r\n");

            pfrt->zone1_n.count = zone1_count;

            /*get zone1 heat profile*/
            for(uint8_t i=0; i<zone1_count; i++)
            {
                uint8_t index;
                index = 3+i*4;
                pfrt->zone1_n.heat_para[i].time = (pdata[index]<<8 | pdata[index+1]);
                pfrt->zone1_n.heat_para[i].temp = (pdata[index+2]<<8 | pdata[index+3]);
            }

            /*print some debug log*/
            LOGD("heat profile zone1: \r\n");
            for(uint8_t i=0; i<zone1_count; i++)
            {
                LOGD("  %d time:%d temp:%d\r\n", i, pfrt->zone1_n.heat_para[i].time, pfrt->zone1_n.heat_para[i].temp);
            }

        }
        else if(pdata[0] == 'b') /*boost heat profile*/
        {
            LOGD("write boost heat porfile\r\n");
            pfrt->zone1_b.count = zone1_count;

            /*get zone1 heat profile*/
            for(uint8_t i=0; i<zone1_count; i++)
            {
                uint8_t index;
                index = 3+i*4;
                pfrt->zone1_b.heat_para[i].time = (pdata[index]<<8 | pdata[index+1]);
                pfrt->zone1_b.heat_para[i].temp = (pdata[index+2]<<8 | pdata[index+3]);
            }

            /*print some debug log*/
            LOGD("heat profile zone1: \r\n");
            for(uint8_t i=0; i<zone1_count; i++)
            {
                LOGD("  %d time:%d temp:%d\r\n", i, pfrt->zone1_b.heat_para[i].time, pfrt->zone1_b.heat_para[i].temp);
            }

        }
    }
    if(pdata[1] == 2)
    {
        zone2_count = pdata[2]; //get zone2 heat section count
        //min_len = min_len+4*zone2_count;
        //if(len != min_len){
        if(zone2_count>MAX_HEAT_PARA_LEN){
            LOGD("heat profile cmd zone2 count error\r\n");
            return;
        }

        if(pdata[0] == 'n')
        {
            LOGD("write normal heat porfile\r\n");

            pfrt->zone2_n.count = zone2_count;

            /*get zone2 heat profile*/
            for(uint8_t i=0; i<zone2_count; i++)
            {
                uint8_t index;
                index = 3+i*4;
                pfrt->zone2_n.heat_para[i].time = (pdata[index]<<8 | pdata[index+1]);
                pfrt->zone2_n.heat_para[i].temp = (pdata[index+2]<<8 | pdata[index+3]);
            }

            /*print some debug log*/
            LOGD("heat profile zone2: \r\n");
            for(uint8_t i=0; i<zone2_count; i++)
            {
                LOGD("  %d time:%d temp:%d\r\n", i, pfrt->zone2_n.heat_para[i].time, pfrt->zone2_n.heat_para[i].temp);
            }
        }
        else if(pdata[0] == 'b') /*boost heat profile*/
        {
            LOGD("write boost heat porfile\r\n");
            pfrt->zone2_b.count = zone2_count;

            /*get zone2 heat profile*/
            for(uint8_t i=0; i<zone2_count; i++)
            {
                uint8_t index;
                index = 3+i*4;
                pfrt->zone2_b.heat_para[i].time = (pdata[index]<<8 | pdata[index+1]);
                pfrt->zone2_b.heat_para[i].temp = (pdata[index+2]<<8 | pdata[index+3]);
            }
        }
    }
    //todo write it to flash
//    update_data_flash(USR_DATA,INVALID);
}
/*************************************************************************************************
  * @brief    : set default heat profile
  * @param1   : the heat profile struct point
  * @param2   : 'n':normal/'b':boost
  * @param3   : the zone number
  * @return   : None
*************************************************************************************************/
void app_set_default_heat_profile(heat_profile_t* pzone,uint8_t type, uint8_t zone)
{
    if(type == 'n'){
#if defined (HTR_EN_1) && defined (HTR_EN_2)
        if(zone==1)
        {
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=7;
            pzone->heat_para[0].time=25;   pzone->heat_para[0].temp=285;
            pzone->heat_para[1].time=30;   pzone->heat_para[1].temp=285;
            pzone->heat_para[2].time=50;   pzone->heat_para[2].temp=260;
            pzone->heat_para[3].time=75;   pzone->heat_para[3].temp=260;
            pzone->heat_para[4].time=170;  pzone->heat_para[4].temp=260;
            pzone->heat_para[5].time=260;  pzone->heat_para[5].temp=240;
            pzone->heat_para[6].time=290;  pzone->heat_para[6].temp=100;
        }else if(zone==2){
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=7;
            pzone->heat_para[0].time=25;   pzone->heat_para[0].temp=0;
            pzone->heat_para[1].time=30;   pzone->heat_para[1].temp=50;
            pzone->heat_para[2].time=50;   pzone->heat_para[2].temp=50;
            pzone->heat_para[3].time=75;   pzone->heat_para[3].temp=120;
            pzone->heat_para[4].time=170;  pzone->heat_para[4].temp=160;
            pzone->heat_para[5].time=260;  pzone->heat_para[5].temp=250;
            pzone->heat_para[6].time=290;  pzone->heat_para[6].temp=250;


        }
#else
        if(zone==1)
        {
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=4;
            pzone->heat_para[0].time=30;   pzone->heat_para[0].temp=230;
            pzone->heat_para[1].time=192;  pzone->heat_para[1].temp=240;
            pzone->heat_para[2].time=219;  pzone->heat_para[2].temp=255;
            pzone->heat_para[3].time=270;  pzone->heat_para[3].temp=265;
        }else if(zone==2){
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=4;
            pzone->heat_para[0].time=30;   pzone->heat_para[0].temp=0;
            pzone->heat_para[1].time=192;  pzone->heat_para[1].temp=0;
            pzone->heat_para[2].time=219;  pzone->heat_para[2].temp=0;
            pzone->heat_para[3].time=270;  pzone->heat_para[3].temp=0;
        }
#endif
    }else if(type == 'b'){
#if defined (HTR_EN_1) && defined (HTR_EN_2)
        if(zone==1)
        {
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=8;
            pzone->heat_para[0].time=8;  pzone->heat_para[0].temp=300;
            pzone->heat_para[1].time=20;  pzone->heat_para[1].temp=300;
            pzone->heat_para[2].time=30;  pzone->heat_para[2].temp=250;
            pzone->heat_para[3].time=75;  pzone->heat_para[3].temp=250;
            pzone->heat_para[4].time=100; pzone->heat_para[4].temp=250;
            pzone->heat_para[5].time=125; pzone->heat_para[5].temp=250;
            pzone->heat_para[6].time=185; pzone->heat_para[6].temp=250;
            pzone->heat_para[7].time=195; pzone->heat_para[7].temp=250;
        }else if(zone==2){
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=8;
            pzone->heat_para[0].time=8;  pzone->heat_para[0].temp=0;
            pzone->heat_para[1].time=15;  pzone->heat_para[1].temp=0;
            pzone->heat_para[2].time=30;  pzone->heat_para[2].temp=250;
            pzone->heat_para[3].time=75;  pzone->heat_para[3].temp=250;
            pzone->heat_para[4].time=100; pzone->heat_para[4].temp=250;
            pzone->heat_para[5].time=125; pzone->heat_para[5].temp=250;
            pzone->heat_para[6].time=185; pzone->heat_para[6].temp=250;
            pzone->heat_para[7].time=195; pzone->heat_para[7].temp=250;

        }
#else
        if(zone==1)
        {
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=4;
            pzone->heat_para[0].time=20;   pzone->heat_para[0].temp=240;
            pzone->heat_para[1].time=128;  pzone->heat_para[1].temp=260;
            pzone->heat_para[2].time=182;  pzone->heat_para[2].temp=270;
            pzone->heat_para[3].time=200;  pzone->heat_para[3].temp=260;
        }else if(zone==2){
            memset(pzone,0,sizeof(heat_profile_t));
            pzone->count=4;
            pzone->heat_para[0].time=20;   pzone->heat_para[0].temp=0;
            pzone->heat_para[1].time=128;  pzone->heat_para[1].temp=0;
            pzone->heat_para[2].time=182;  pzone->heat_para[2].temp=0;
            pzone->heat_para[3].time=200;  pzone->heat_para[3].temp=0;

        }
#endif
    }
}

/*************************************************************************************************
 * @brief   :get heat profile
 * @param   :normal or boost
 * @param   :zone number
 * @param   :heat profile point
 * @return  :none
*************************************************************************************************/
void app_get_heat_profile(uint8_t type, uint8_t zone, heat_profile_t *pZone)
{
    uint8_t i = 0;
    /*read heat profile from flash(ram)*/
    flash_record_t* pfrt = get_self_flash_record_from_ram();
    if(type == 'n'){
        if(zone==1){
            /*make sure the data read from flash is vailibale*/
            if(pfrt->zone1_n.count > 0 && pfrt->zone1_n.count < MAX_HEAT_PARA_LEN
                && (pfrt->zone1_n.heat_para[0].time<500) &&
                (pfrt->zone1_n.heat_para[0].temp<315)){
                pZone->count = pfrt->zone1_n.count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pZone->heat_para[i].temp = pfrt->zone1_n.heat_para[i].temp;
                    pZone->heat_para[i].time = pfrt->zone1_n.heat_para[i].time;
                    i++;
                }
                return ;
            }else{ /*get default heat profile*/
                app_set_default_heat_profile(pZone,'n',1);
                pfrt->zone1_n.count = pZone->count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pfrt->zone1_n.heat_para[i].temp = pZone->heat_para[i].temp;
                    pfrt->zone1_n.heat_para[i].time = pZone->heat_para[i].time;
                    i++;
                }
                memcpy(pfrt->n_heat_name,"default", 8);
                /*store the date to flash*/
                update_data_flash(USR_DATA,INVALID);

                return;
            }
        }else if(zone==2){
            /*make sure the data read from flash is vailibale*/
            if(pfrt->zone2_n.count > 0 && pfrt->zone2_n.count < MAX_HEAT_PARA_LEN
                && (pfrt->zone2_n.heat_para[0].time<500) &&
                (pfrt->zone2_n.heat_para[0].temp<315)){
                pZone->count = pfrt->zone2_n.count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pZone->heat_para[i].temp = pfrt->zone2_n.heat_para[i].temp;
                    pZone->heat_para[i].time = pfrt->zone2_n.heat_para[i].time;
                    i++;
                }
                return;
            }else{/*get default heat profile*/
                app_set_default_heat_profile(pZone,'n',2);
                pfrt->zone2_n.count = pZone->count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pfrt->zone2_n.heat_para[i].temp = pZone->heat_para[i].temp;
                    pfrt->zone2_n.heat_para[i].time = pZone->heat_para[i].time;
                    i++;
                }
                memcpy(pfrt->n_heat_name,"default", 8);
                /*store the date to flash*/
                update_data_flash(USR_DATA,INVALID);
                return;
            }
        }
    }else if(type == 'b'){
        if(zone==1){
            /*make sure the data read from flash is vailibale*/
            if(pfrt->zone1_b.count > 0 && pfrt->zone1_b.count < MAX_HEAT_PARA_LEN
                && (pfrt->zone1_b.heat_para[0].time<500) &&
                (pfrt->zone1_b.heat_para[0].temp<315)){
                pZone->count = pfrt->zone1_b.count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pZone->heat_para[i].temp = pfrt->zone1_b.heat_para[i].temp;
                    pZone->heat_para[i].time = pfrt->zone1_b.heat_para[i].time;
                    i++;
                }
                return;
            }else{
                app_set_default_heat_profile(pZone,'b',1);
                pfrt->zone1_b.count = pZone->count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pfrt->zone1_b.heat_para[i].temp = pZone->heat_para[i].temp;
                    pfrt->zone1_b.heat_para[i].time = pZone->heat_para[i].time;
                    i++;
                }
                memcpy(pfrt->b_heat_name,"default", 8);
                /*store the date to flash*/
                update_data_flash(USR_DATA,INVALID);
                return;
            }
        }else if(zone==2){
            /*make sure the data read from flash is vailibale*/
            if(pfrt->zone2_b.count > 0 && pfrt->zone2_b.count < MAX_HEAT_PARA_LEN
                && (pfrt->zone2_b.heat_para[0].time<500) &&
                (pfrt->zone2_b.heat_para[0].temp<315)){
                pZone->count = pfrt->zone2_b.count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pZone->heat_para[i].temp = pfrt->zone2_b.heat_para[i].temp;
                    pZone->heat_para[i].time = pfrt->zone2_b.heat_para[i].time;
                    i++;
                }
                return;
            }else{
                app_set_default_heat_profile(pZone,'b',2);
                pfrt->zone2_b.count = pZone->count;
                i = 0;
                while (i < MAX_HEAT_PARA_LEN)
                {
                    pfrt->zone2_b.heat_para[i].temp = pZone->heat_para[i].temp;
                    pfrt->zone2_b.heat_para[i].time = pZone->heat_para[i].time;
                    i++;
                }
                /*store the date to flash*/
                memcpy(pfrt->b_heat_name,"default", 8);
                update_data_flash(USR_DATA,INVALID);
                return;
            }
        }

	}
}
/*************************************************************************************************
  * @brief    : Get now set heat duty
  * @return   : none
*************************************************************************************************/
void app_get_duty(void)
{
    //flash_record_t* pfrt =get_self_flash_record_from_ram();
    pwm_dac_duty = get_self_flash_record_from_ram()->pwm_dac_duty;
    if(pwm_dac_duty>100)
    {
        pwm_dac_duty=30;
    }
    LOGD("pwm_dac_duty is %d \n\r",pwm_dac_duty);
#ifdef HTR_EN_1
    pwm_htr_duty_coil1 = get_self_flash_record_from_ram()->pwm_htr_duty_coil1;
    if(pwm_htr_duty_coil1>100)
    {
        pwm_htr_duty_coil1=PWM_DUTY_COIL1;
    }
    LOGD("pwm_htr_duty_coil1 is %d \n\r",pwm_htr_duty_coil1);
#endif
#ifdef HTR_EN_2
    pwm_htr_duty_coil2 = get_self_flash_record_from_ram()->pwm_htr_duty_coil2;
    if(pwm_htr_duty_coil2>100)
    {
        pwm_htr_duty_coil2=PWM_DUTY_COIL2;
    }
    LOGD("pwm_htr_duty_coil2 is %d \n\r",pwm_htr_duty_coil2);
#endif

#ifdef HTR_EN_1
    uint16_t pwm_htr_clock_coil1 = get_self_flash_record_from_ram()->pwm_htr_clock_coil1;
    dev_set_pwm_htr_clock(pwm_htren1,pwm_htr_clock_coil1);
//    dev_set_pwm_htr_clock(pwm_htren3,pwm_htr_clock_coil1);
    LOGD("pwm_htr_clock_coil1 is %d \n\r",pwm_htr_clock_coil1);
#endif
#ifdef HTR_EN_2
    uint16_t pwm_htr_clock_coil2 = get_self_flash_record_from_ram()->pwm_htr_clock_coil2;
    dev_set_pwm_htr_clock(pwm_htren2,pwm_htr_clock_coil2);
//    dev_set_pwm_htr_clock(pwm_htren4,pwm_htr_clock_coil2);
    LOGD("pwm_htr_clock_coil2 is %d \n\r",pwm_htr_clock_coil2);
#endif
}
/*************************************************************************************************
  * @brief    : enable coil1 heat
  * @return   : none
*************************************************************************************************/
void start_coil1_heat(void)
{
    if(coil1_start_state==0)
    {
#ifdef HTR_EN_2
        app_heat2_enable(0);
        coil2_start_state=0;
#endif

#ifdef HTR_EN_1
        app_heat1_enable(1);
        coil1_start_state=1;
#endif
        //start_hlh();
    }
}
/*************************************************************************************************
  * @brief    : disable coil1 heat
  * @return   : none
*************************************************************************************************/
void stop_coil1_heat(void)
{
    app_heat1_enable(0);
    coil1_start_state=0;
}
/*************************************************************************************************
  * @brief    : enable coil2 heat
  * @return   : none
*************************************************************************************************/
void start_coil2_heat(void)
{
    if(coil2_start_state==0)
    {
#ifdef HTR_EN_1
        app_heat1_enable(0);
        coil1_start_state=0;
#endif

#ifdef HTR_EN_2
        app_heat2_enable(1);
        coil2_start_state=1;
#endif
        //start_hlh();

    }
}
/*************************************************************************************************
  * @brief    : disable coil2 heat
  * @return   : none
*************************************************************************************************/
void stop_coil2_heat(void)
{
    app_heat2_enable(0);
    coil2_start_state=0;
}
/*************************************************************************************************
  * @brief    : enable a start signal
  * @return   : none
*************************************************************************************************/
void start_hlh(void)
{
#if 0
    __disable_irq();
//  HAL_GPIO_WritePin(START_PORT, START_PIN, GPIO_PIN_RESET);
    hwi_GPIO_WritePin(, HWI_PIN_RESET);

//  HAL_GPIO_WritePin(START_PORT, START_PIN, GPIO_PIN_SET);
    hwi_GPIO_WritePin(, HWI_PIN_SET);

    __enable_irq();
#endif
}

void app_heat1_enable(uint8_t en){
    if(en){
        //HAL_GPIO_WritePin(HEAT1_PORT, HEAT_EN_1_PIN, GPIO_PIN_SET);
        dev_pwm_set_duty(pwm_htren1,pwm_htr_duty_coil1);
        hwi_GPIO_WritePin(FULL_B_EN1, HWI_PIN_SET);
    }else{
        dev_pwm_set_duty(pwm_htren1,0);
        hwi_GPIO_WritePin(FULL_B_EN1, HWI_PIN_RESET);
    }
}

void app_heat2_enable(uint8_t en){
    if(en){
        //HAL_GPIO_WritePin(HEAT2_PORT, HEAT_EN_2_PIN, GPIO_PIN_SET);
        dev_pwm_set_duty(pwm_htren2,pwm_htr_duty_coil2);
        hwi_GPIO_WritePin(FULL_B_EN2, HWI_PIN_SET);
    }else{
        //HAL_GPIO_WritePin(HEAT2_PORT, HEAT_EN_2_PIN, GPIO_PIN_RESET);
        dev_pwm_set_duty(pwm_htren2,0);
        hwi_GPIO_WritePin(FULL_B_EN2, HWI_PIN_RESET);
    }
}
/*************************************************************************************************
  * @brief    : heat init
  * @return   : none
*************************************************************************************************/
void app_heat_init(void)
{
#ifdef HTR_EN_1
    app_heat1_enable(0);
#endif
#ifdef HTR_EN_2
    app_heat2_enable(0);
#endif
    /*get heat cutoff voltage from flash*/
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    int16_t heat_cutoff_volt = pfrt->error_parameter.heat_cutoff_volt;
    int16_t heat_cutoff_volt_soc = pfrt->error_parameter.heat_cutoff_volt_soc;
    if(heat_cutoff_volt > 4000 || heat_cutoff_volt <= 3000)
    {
        pfrt->error_parameter.heat_cutoff_volt = PRE_HEAT_CUTOFF_VOLT;
        update_data_flash(USR_DATA,INVALID);
    }
    if(heat_cutoff_volt_soc > 99 || heat_cutoff_volt_soc < 0)
    {
        LOGD("cutoff_volt_soc need 0~99,return to default");
        pfrt->error_parameter.heat_cutoff_volt_soc = PRE_HEAT_CUTOFF_VOLT_SOC;
        update_data_flash(USR_DATA,INVALID);
    }
}
/*************************************************************************************************
  * @brief    : get heat CJR value
  * @return   : CJR value
*************************************************************************************************/
float app_get_heat_CJR()
{
    float cjr=0.0;
    int16_t zone1max,zone1min;
    int16_t zone2max,zone2min;
    int16_t zoneTempChange=0;
    /*the begin 1s datas record*/
    zone1max=zone1min=zone2max=zone2min=0;
    zone1Temp[heat_cnt%32]=zone1_temp;
    zone2Temp[heat_cnt%32]=zone2_temp;
    totalJoule[heat_cnt%32]=total_joules;
    if(heat_cnt>=31)
    {
        zone1max=zone1min=zone1Temp[0];
        zone2max=zone2min=zone2Temp[0];
        for (int i = 0; i < 32; i++) {
            if (zone1Temp[i] > zone1max) {
                zone1max = zone1Temp[i];
            }
            if (zone1Temp[i] < zone1min) {
                zone1min = zone1Temp[i];
            }

            if (zone2Temp[i] > zone2max) {
                zone2max = zone2Temp[i];
            }
            if (zone2Temp[i] < zone2min) {
                zone2min = zone2Temp[i];
            }
        }
        zoneTempChange=(zone1max-zone1min)+(zone2max-zone2min);
        if(zoneTempChange>0){
            cjr=(total_joules-totalJoule[heat_cnt%31])/(float)(zoneTempChange);
        }else{
            /*if 32*16ms zone temp change less than 1c, set cjr to a max vaule*/
            cjr=255.0;
        }
        //LOGD("------%f %f--%d---%f------\n\r",total_joules,totalJoule[heat_cnt%31],zoneTempChange,cjr);
    }
    return cjr;
}

static void heat_lock_90s(const ptimer_t tm)
{
    if(heat_lock_timer){
        if(bat_timer_delete(heat_lock_timer, portMAX_DELAY)==pdPASS){
            heat_lock_timer = NULL;
        }
    }
    heat_lock_state=0;
}

/*************************************************************************************************
  * @brief    : During session errorCode check
  * @return   : none
*************************************************************************************************/
void app_during_session_errorCode_check()
{
    uint16_t target_temp;

    if(get_error_check_status()==disable_s){
        return;
    }

    if(heat_cnt <= 0)
    {
        return ;
    }
    adc_context_st * adc_c = dev_get_adc_result();
    float batv_check = adc_c->vbat;
    float zone1_temp_check = adc_c->zone1_temp;
    float zone2_temp_check = adc_c->zone2_temp;
    float tc1_temp_check = dev_get_tc_temp(1);
    float tc2_temp_check = dev_get_tc_temp(2);
    if(batv_check<=heat_empty_volt) //&& (dev_get_i_sense()<5.0))
    {
        /*1.WAR_DE_BAT_EMPTY*/
        app_stop_heat();
        post_msg_to_manager_with_arg(op_error_occur,war_de_bat_empty);
        LOGE("heatERROR:WAR_DE_BAT_EMPTY,vbat:%.3f", batv_check);
    }
    if(((zone1_temp_check-old_zone1_temp)>20) || ((zone2_temp_check-old_zone2_temp)>20) ||
        ((zone1_temp_check-old_zone1_temp)<-20) || ((zone2_temp_check-old_zone2_temp)<-20))
    {
        //tc_spike_error_cnt++;
        /*4.FLT_DE_TC_SPIKE*/
        //if(tc_spike_error_cnt>1)
        //{
            app_stop_heat();
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_spike);
            LOGE("heatERROR:FLT_DE_TC_SPIKE,zone1t:%.3f,old zone1t:%.3f,zone2t:%.3f,old zone2t:%.3f",
                zone1_temp_check,old_zone1_temp,zone2_temp_check,old_zone2_temp);
            /*lock 90s*/
            //TIMER_SAFE_RESET(heat_lock_timer, 90*1000, TIMER_OPT_ONESHOT, heat_lock_90s, NULL);
            heat_lock_timer = bat_timer_reset_ext(heat_lock_timer, "heat_lock_timer", 90*1000, TIMER_OPT_ONESHOT, heat_lock_90s);
            bat_timer_start(heat_lock_timer, portMAX_DELAY);
            heat_lock_state=1;
        //}
    }
//    if((((zone1_temp_check-old_zone1_temp)>=1) && ((total_joules-old_total_joules)/(zone1_temp_check-old_zone1_temp)>4.0)) ||
//        (((zone2_temp_check-old_zone2_temp)>=1) && ((total_joules-old_total_joules)/(zone2_temp_check-old_zone2_temp)>4.0)))
//    {
//        /*5.FLT_DE_TC_HIGH_CJR*/
//        app_stop_heat();
//        post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_high_cjr);
//        LOGE("heat error stop: FLT_DE_TC_HIGH_CJR\n\r");
//        LOGD("zone1t: %.3f, old zone1t: %.3f, zone2t: %.3f, old zone2t: %.3f, total_j: %.3f, old_total_j: %.3f",
//            zone1_temp,old_zone1_temp,zone2_temp_check,old_zone2_temp,total_joules,old_total_joules);
//        /*lock 90s*/
//        TIMER_SAFE_RESET(heat_lock_timer, 90*1000, TIMER_OPT_ONESHOT, heat_lock_90s, NULL);
//        heat_lock_state=2;
//    }
#ifdef HTR_EN_1
    target_temp = pzone1_prof.heat_para[heat_zone1_stage].temp;
    if(heat_coil1_checkout_cnt>= HEAT_CHECK_TIEM)
    {
        /*7.FLT_DE_TC_ZONE1_HEATING_ABNORMAL*/
        if((zone1_temp_check<(int16_t)(target_temp -20)) ||
            ((zone1_temp_check>(target_temp +20)) &&
            (target_temp > 150)))
        {
            app_stop_heat();
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone1_heating_abnormal);
            LOGE("heatERROR:FLT_DE_TC_ZONE1_HEATING_ABNORMAL,coil1_cnt:%d,heat_zone1_stage:%d,zone1t:%.3f", heat_coil1_checkout_cnt,heat_zone1_stage,zone1_temp_check);
        }
        //now_pwm=18;
        //dev_pwm_set_duty(pwm_dac, 18);
    }
#endif
#ifdef HTR_EN_2
    target_temp = pzone2_prof.heat_para[heat_zone2_stage].temp;
    if(heat_coil2_checkout_cnt>= HEAT_CHECK_TIEM)
    {
        /*8.FLT_DE_TC_ZONE2_HEATING_ABNORMAL*/
        if((zone2_temp_check<(int16_t)(target_temp -20)) ||
            ((zone2_temp_check>(target_temp +20)) &&
            (target_temp >150)))
        {
            app_stop_heat();
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone2_heating_abnormal);
            LOGE("heatERROR:FLT_DE_TC_ZONE2_HEATING_ABNORMAL,coil2_cnt:%d,heat_zone2_stage:%d,zone2t:%.3f", heat_coil2_checkout_cnt,heat_zone2_stage,zone2_temp_check);
        }
        //now_pwm=18;
        //dev_pwm_set_duty(pwm_dac, now_pwm);
    }
#endif
    if((heat_cnt*16 > 1000 ) && (heat_cnt*16 <= 10*1000))//1s after heating starts to 10s after heating starts
    {
#ifdef HTR_EN_1
        if((tc1_temp_check < 10) && (pzone1_prof.heat_para[heat_zone1_stage].temp > 100))
        {
            app_stop_heat();
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone1_heating_abnormal);
            LOGE("heatERROR:FLT_DE_TC_ZONE1_HEATING_ABNORMAL_1s,tc1t:%.3f", tc1_temp_check);
        }
#endif
#ifdef HTR_EN_2
        if((tc2_temp_check < 10) && (pzone2_prof.heat_para[heat_zone2_stage].temp > 100))
        {
            app_stop_heat();
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone2_heating_abnormal);
            LOGE("heatERROR:FLT_DE_TC_ZONE2_HEATING_ABNORMAL_1s,tc2t:%.3f", tc2_temp_check);
        }
#endif
    }
    if(heat_cnt*16 > 10*1000)//git 10s after heating starts
    {
#ifdef HTR_EN_1
        if(tc1_temp_check < 10)
        {
            app_stop_heat();
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone1_heating_abnormal);
            LOGE("heatERROR:FLT_DE_TC_ZONE1_HEATING_ABNORMAL_10s,tc1t:%.3f", tc1_temp_check);
        }
#endif
#ifdef HTR_EN_2
        if(tc2_temp_check < 10)
        {
            app_stop_heat();
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone2_heating_abnormal);
            LOGE("heatERROR:FLT_DE_TC_ZONE2_HEATING_ABNORMAL_10s,tc2t:%.3f", tc2_temp_check);
        }

#endif
    }
    if(heat_times < 64)
    {
        static float i_sense_max = 0;
        i_sense_max = i_sense_max > dev_get_isense_peak() ? i_sense_max : dev_get_isense_peak();
        heat_times++;
        if(coil1_start_state || coil2_start_state)
        {
            heat_enable_flag_1s=1;
        }
        if(heat_times == 64)
        {
            if((i_sense_max < i_sense_damage) && heat_enable_flag_1s &&
                            (old_zone1_temp < pzone1_prof.heat_para[heat_zone1_stage].temp||
                            old_zone2_temp < pzone2_prof.heat_para[heat_zone2_stage].temp))
            {
                /*9.FLT_DE_BAT_I_SENSE_DAMAGE*/
                app_stop_heat();
                post_msg_to_manager_with_arg(op_error_occur,flt_de_bat_i_sense_damage);
                LOGE("heatERROR:FLT_DE_BAT_I_SENSE_DAMAGE,i_sense_max is:%.3f,coil1_start_state[%d],coil2_start_state[%d]", i_sense_max,coil1_start_state,coil2_start_state);
            }
            heat_times = 0;
            i_sense_max = 0;
            heat_enable_flag_1s=0;
        }
    }
    old_zone1_temp = zone1_temp_check;
    old_zone2_temp = zone2_temp_check;
}
/*************************************************************************************************
  * @brief    : Before session errorCode check
  * @return   : none
*************************************************************************************************/
uint8_t pre_session_errorCode_check()
{
    read_error_parameter();
    uint8_t bat_soc;

    if(get_error_check_status()==disable_s){
        return 0;
    }
    adc_context_st * adc_c = dev_get_adc_result();

    if(adc_c->bat_temp >= (pre_session_temp_limit/1.0))
    {
        /*1.FLT_DE_BAT_HOT_40_PRE_SES*/
        LOGE("heatERROR:FLT_DE_BAT_HOT_PRE_SES");
        post_msg_to_manager_with_arg(op_error_occur,flt_de_bat_hot_40_pre_ses);
        return 1;
    }
    bat_soc = app_get_bat_left();
#ifdef ENABLE_FUEL_GAUGE
    if(bat_soc<heat_cutoff_volt_soc)
#else
    if(adc_c->vbat<=heat_cutoff_volt)
#endif
    {
        LOGE("heatERROR:WAR_DE_BAT_LOW,cutoff_volt_soc:%d", bat_soc);
        //LOGE("discharge_cutoff_volt is %.3f\r\n", dev_get_vbat_volt());
        /*5.WAR_DE_BAT_LOW*/
        post_msg_to_manager_with_arg(op_error_occur,war_de_bat_low);
        return 1;
    }
//    if((dev_get_tc_temp(1)-dev_get_tc_temp(2))>20 || (dev_get_tc_temp(2)-dev_get_tc_temp(1))>20||
//        ((zone1_temp-old_zone1_temp)<-20) || ((zone2_temp-old_zone2_temp)<-20))
//    {
//        /*8.FLT_DE_TC_ZONE_IMBALANCE*/
//        LOGE("heat error stop:FLT_DE_TC_ZONE_IMBALANCE \n\r");
//        post_msg_to_manager(op_heat_finish);
//        post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone_imbalance);
//        return 1;
//    }
    if(next_heat_start_time > pre_heat_stop_time){
        /*check need two intervals of more than 120 s*/
        if((pre_heat_stop_time==0)||(next_heat_start_time-pre_heat_stop_time>120000))
        {
    #if defined (HTR_EN_1) && defined (HTR_EN_2)
            if((adc_c->zone1_temp - adc_c->zone2_temp > 20) || (adc_c->zone2_temp - adc_c->zone1_temp > 20))
            {
                /*8.FLT_DE_TC_ZONE_IMBALANCE*/
                LOGE("heatERROR:FLT_DE_TC_ZONE_IMBALANCE");
                post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_zone_imbalance);
                return 1;
            }
    #endif
        }
    }
    /*if(b2b.b2b_error_flag){
        post_msg_to_manager_with_arg(op_error_occur,war_de_tc_b2b_overheat);
        LOGE("heat error stop:B2B_ERROR \n\r");
        return 1;
    }else{
        if(b2b.pre_heat_finish_tick != 0 && TicsSince(b2b.pre_heat_finish_tick)<30*1000){
            b2b.count++;
            LOGD("No. %d b2b session", b2b.count);
            flash_record_t *frt = get_self_flash_record_from_ram();
            if(b2b.count>=3 && dev_get_bat_temp() >= frt->b2b_batt_temp_limit){
                post_msg_to_manager_with_arg(op_error_occur,war_de_tc_b2b_overheat);
                LOGE("heat error stop:B2B_OVERHEAT batt temp limit: %d", frt->b2b_batt_temp_limit);
                b2b.error_tick = GetTick(); //record b2b error tick
                b2b.b2b_error_flag = 1;
                return 1;
            }
        }else{
            b2b.count = 0;
        }
    }*/
    return 0;
}


/*************************************************************************************************
  * @brief    : Clear Back to Back session error flag if the error already exists for 60s
  * @return   : none
*************************************************************************************************/
void app_auto_clear_b2b_error()
{
    if(b2b.b2b_error_flag){
        if(TicsSince(b2b.error_tick)>=60*1000){
            b2b.b2b_error_flag = 0;
        }
    }
}

/*************************************************************************************************
  * @brief    : get b2b flag
  * @return   : b2b_error_flag
*************************************************************************************************/
uint8_t get_b2b_flag()
{
   return b2b.b2b_error_flag;
}

/*************************************************************************************************
  * @brief    : calculate every(16*62ms) pv value
  * @return   : none
*************************************************************************************************/
void calculate_pv_value(void)
{
    pv1=pv2=0;
    pv1_array[heat_cnt%62]=(int16_t)zone1_temp;//get_cold_junc_average_temp() + get_tc_average_temp(1);
    pv2_array[heat_cnt%62]=(int16_t)zone2_temp;//get_cold_junc_average_temp() + get_tc_average_temp(2);

    if(heat_cnt<62)
    {
        pv1_sum+=pv1_array[heat_cnt];
        pv1=pv1_sum/(heat_cnt+1);

        pv2_sum+=pv2_array[heat_cnt];
        pv2=pv2_sum/(heat_cnt+1);
    }else{
        pv1_sum=pv2_sum=0;
        for(int i=0;i<62;i++)
        {
            pv1_sum+= pv1_array[i];
            pv1= pv1_sum/62;

            pv2_sum+= pv2_array[i];
            pv2= pv2_sum/62;
        }
    }
}
int16_t app_get_pv_value(uint8_t number)
{
    if(number==1){
        return pv1;
    }else{
        return pv2;
    }
}
int16_t app_get_sp_value(uint8_t number)
{
    if(number==1)
    {
        return pzone1_prof.heat_para[heat_zone1_stage].temp;
    }else{
        return pzone2_prof.heat_para[heat_zone2_stage].temp;
    }
}
heat_chan_t* app_get_chan_value(void)
{
    return &heat_chan_data;
}
uint16_t app_get_totalJ_value()
{
    return (uint16_t)total_joules;
}
/*************************************************************************************************
  * @brief    : heat one count every 16ms
  * @param    : ptimer_t struct
  * @return   : None
*************************************************************************************************/
static void heat_one_cnt(const ptimer_t tm)
{
    float heat_time,onecnt_joule;
    uint8_t heat_stop_chan1,heat_stop_chan2;
    uint16_t heat_stop_time;
#if defined(HTR_EN_1)
    heat_stop_chan1=0;
#endif
#if defined(HTR_EN_2)
    heat_stop_chan2=0;
#endif
    //heat_stop_chan1=heat_stop_chan2=0;
    heat_chan_data.g_chan = heat_chan;
    heat_chan_data.log_i = dev_get_adc_result()->i_sense;
    //zone1_temp=dev_get_cold_junc_temp() + dev_get_tc_temp(1);
    //zone2_temp=dev_get_cold_junc_temp() + dev_get_tc_temp(2);
    zone1_temp = dev_get_adc_result()->zone1_temp;
    zone2_temp = dev_get_adc_result()->zone2_temp;

    heat_time = (float)(heat_cnt*HEAT_ONE_CNT_TIME)/1000.0f;
//    onecnt_power=(float)(dev_get_vbat_volt()*dev_get_i_sense());
    onecnt_joule = (float)((dev_get_adc_result()->vbat)*(dev_get_adc_result()->i_sense)*16.0f)/1000.0f;
    total_joules+=onecnt_joule;
    calculate_pv_value();
    if(heat_cnt==0)
    {
        //LOGD("heat: time zone1t zone2t batt usbt coilt junct tc1t tc2t vbusv vbatv isenseA onecntj totalj power pwm\n\r");
    }
    /*every 62*16ms printf debug log*/
    if(heat_cnt%62==0){
     /*LOGD("heat: %0.3f %d %d %f %f %f %f %d %d %0.3f %0.3f %0.3f %f %f %f %d\n\r",
            heat_time,zone1_temp,zone2_temp, dev_get_bat_temp(), dev_get_usb_temp(),
            dev_get_coil_temp(),
            dev_get_cold_junc_temp(),dev_get_tc_temp(1),dev_get_tc_temp(2),app_GetVbusVolt(),dev_get_vbat_volt(),
            dev_get_i_sense(),onecnt_joule,total_joules,onecnt_power,now_pwm);*/
    }
#if defined(HTR_EN_1) && defined(HTR_EN_2)
    /*every 16ms change coile heat*/
    if((heat_cnt%2==0) && (coil1_need_heat==0))
    {
        /*if coil1 need heat*/
        if((zone1_temp < pzone1_prof.heat_para[heat_zone1_stage].temp) &&
                (pzone1_prof.heat_para[heat_zone1_stage].temp>0))
        {
            start_coil1_heat();
            heat_chan=1;
        }
        else if(zone1_temp >=pzone1_prof.heat_para[heat_zone1_stage].temp)//coil1 ont need heat
        {
            stop_coil1_heat();
            heat_stop_chan1=1;
            //if coil2 need start or stop when coil1 stop
            if(coil2_need_heat==0){
                if((zone2_temp < pzone2_prof.heat_para[heat_zone2_stage].temp) &&
                    (pzone2_prof.heat_para[heat_zone2_stage].temp>0))
                {
                    start_coil2_heat();
                    heat_chan=2;
                }
                if(zone2_temp >=pzone2_prof.heat_para[heat_zone2_stage].temp)
                {
                    stop_coil2_heat();
                    heat_stop_chan2=1;
                }
            }
        }
        /*if zone1 heat need change to next stage*/
        if(pzone1_prof.heat_para[heat_zone1_stage].time *1000 <=(HEAT_ONE_CNT_TIME*heat_cnt))
        {
            heat_coil1_checkout_cnt=0;
            heat_zone1_stage++;
            //now_pwm=30;
            //dev_pwm_set_duty(pwm_dac, 30);
        }
       
        //for SESSION_EXTEND_FUCTION_ON in normal heat, and NONE SESSSION EXTEND IN BOOST mode
        if(g_session_extend_flag == SESSION_EXTEND_FUCTION_ON || g_session_extend_flag == NONE_SESSION_EXTEND) 
        {        
            heat_stop_time = pzone1_prof.heat_para[pzone1_prof.count-1].time;
        }
        else
        {
            heat_stop_time = pzone1_prof.heat_para[pzone1_prof.count-2].time;
        }
        
        if((HEAT_ONE_CNT_TIME*heat_cnt)>= heat_stop_time*1000)
        {
            LOGD("stop coil1 heat");
            coil1_need_heat=1;
            stop_coil1_heat();
            heat_stop_chan1=1;
        }       

    }else if(coil2_need_heat==0){//if coil2 need heat
        if((zone2_temp < pzone2_prof.heat_para[heat_zone2_stage].temp) &&
            (pzone2_prof.heat_para[heat_zone2_stage].temp>0))
        {
            start_coil2_heat();
            heat_chan=2;
        }
        /*coil2 need stop heat*/
        else if(zone2_temp >=pzone2_prof.heat_para[heat_zone2_stage].temp)
        {
            stop_coil2_heat();
            heat_stop_chan2=1;
            //if coil1 need start or stop when coil2 stop
            if(coil1_need_heat==0){
                if((zone1_temp < pzone1_prof.heat_para[heat_zone1_stage].temp) &&
                    (pzone1_prof.heat_para[heat_zone1_stage].temp>0))
                {
                    start_coil1_heat();
                    heat_chan=1;
                }
                else if(zone1_temp >=pzone1_prof.heat_para[heat_zone1_stage].temp)
                {
                    stop_coil1_heat();
                    heat_stop_chan1=1;
                }
            }
        }
        /*if zone2 heat need change to next stage*/
        if(pzone2_prof.heat_para[heat_zone2_stage].time*1000 <=(HEAT_ONE_CNT_TIME*heat_cnt))
        {
            heat_coil2_checkout_cnt=0;
            heat_zone2_stage++;
            //now_pwm=30;
            //dev_pwm_set_duty(pwm_dac, 30);
        }      
        
        
            
        //for SESSION_EXTEND_FUCTION_ON in normal heat, and NONE SESSSION EXTEND IN BOOST mode
        if(g_session_extend_flag == SESSION_EXTEND_FUCTION_ON || g_session_extend_flag == NONE_SESSION_EXTEND) 
        {        
            heat_stop_time = pzone2_prof.heat_para[pzone2_prof.count-1].time;
        }
        else
        {
            heat_stop_time = pzone2_prof.heat_para[pzone2_prof.count-2].time;
        }
        
        if((HEAT_ONE_CNT_TIME*heat_cnt)>= heat_stop_time*1000)
        {
            LOGD("stop coil2 heat");
            coil2_need_heat=1;
            stop_coil2_heat();
            heat_stop_chan2=1;
        }  

    }
#elif defined(HTR_EN_1)
    if(coil1_need_heat==0)
    {
        /*if coil1 need heat*/
        if((zone1_temp < pzone1_prof.heat_para[heat_zone1_stage].temp) &&
                (pzone1_prof.heat_para[heat_zone1_stage].temp>0))
        {
            start_coil1_heat();
            heat_chan=1;
        }
        else if(zone1_temp >=pzone1_prof.heat_para[heat_zone1_stage].temp)//coil1 ont need heat
        {
            stop_coil1_heat();
            heat_stop_chan1=1;
        }
        /*if zone1 heat need change to next stage*/
        if(pzone1_prof.heat_para[heat_zone1_stage].time<=(HEAT_ONE_CNT_TIME*heat_cnt/1000))
        {
            heat_coil1_checkout_cnt=0;
            heat_zone1_stage++;
            //now_pwm=30;
            //dev_pwm_set_duty(pwm_dac, 30);
        }
        if((HEAT_ONE_CNT_TIME*heat_cnt/1000)>= pzone1_prof.heat_para[pzone1_prof.count-1].time)
        {
            LOGD("stop coil1 heat");
            coil1_need_heat=1;
            stop_coil1_heat();
            heat_stop_chan1=1;
        }
    }
#elif defined(HTR_EN_2)
    if(coil2_need_heat==0)
    {
        if((zone2_temp < pzone2_prof.heat_para[heat_zone2_stage].temp) &&
            (pzone2_prof.heat_para[heat_zone2_stage].temp>0))
        {
            start_coil2_heat();
            heat_chan=2;
        }
        /*coil2 need stop heat*/
        else if(zone2_temp >=pzone2_prof.heat_para[heat_zone2_stage].temp)
        {
            stop_coil2_heat();
            heat_stop_chan2=1;
        }
        /*if zone2 heat need change to next stage*/
        if(pzone2_prof.heat_para[heat_zone2_stage].time<=(HEAT_ONE_CNT_TIME*heat_cnt/1000))
        {
            heat_coil2_checkout_cnt=0;
            heat_zone2_stage++;
            //now_pwm=30;
            //dev_pwm_set_duty(pwm_dac, 30);
        }

        if((HEAT_ONE_CNT_TIME*heat_cnt/1000)>= pzone2_prof.heat_para[pzone2_prof.count-1].time)
        {
            coil2_need_heat=1;
            stop_coil2_heat();
            heat_stop_chan2=1;
        }
    }
#endif
    /*record now which chan is in heating*/
#if defined(HTR_EN_1) && defined(HTR_EN_2)
    if(heat_stop_chan1==1 && heat_stop_chan2==1)
    {
        heat_chan=0;
    }
#elif defined(HTR_EN_1)
    if(heat_stop_chan1==1)
    {
        heat_chan=0;
    }
#elif defined(HTR_EN_2)
    if(heat_stop_chan2==1)
    {
        heat_chan=0;
    }
#endif
    //app_during_session_errorCode_check();
    //old_zone1_temp=zone1_temp;
    //old_zone2_temp=zone2_temp;
    /*record max datas during heat*/
    old_total_joules=total_joules;

/*    if(max_zone1_temp<zone1_temp)
    {
        max_zone1_temp=zone1_temp;
    }
    if(max_zone2_temp<zone2_temp)
    {
        max_zone2_temp=zone2_temp;
    }
    if(max_bat_temp<dev_get_bat_temp())
    {
        max_bat_temp=dev_get_bat_temp();
    }
    if(max_cold_junc_temp<dev_get_cold_junc_temp())
    {
        max_cold_junc_temp=dev_get_cold_junc_temp();
    }*/
    heat_cnt++;

#ifdef HTR_EN_1
    heat_coil1_checkout_cnt++;
#endif
#ifdef HTR_EN_2
    heat_coil2_checkout_cnt++;
#endif
    /*post op_heat_session_extend measage*/
    if(g_session_extend_flag == HAVE_SESSION_EXTEND)
    {
        if(heat_cnt*HEAT_ONE_CNT_TIME >= (heat_all_time- SESSION_EXTEND_PERIOD_START))
        {
            post_msg_to_manager_with_arg(op_heat_session_extend,SESSION_EXTEND_TIME_START);
            g_session_extend_flag = SESSION_EXTEND_TIME_START;
        }
    }
//    else if(g_session_extend_flag == SESSION_EXTEND_TIME_START)
//    {
//        if(heat_cnt == (heat_all_time- (SESSION_EXTEND_PERIOD_START - SESSION_EXTEND_PERIOD))/HEAT_ONE_CNT_TIME)
//        {
//            post_msg_to_manager_with_arg(op_heat_session_extend,SESSION_EXTEND_TIME_END);
//            g_session_extend_flag = SESSION_EXTEND_TIME_END;
//        }
//    }

    /*post op_heat_end_soon measage*/
    if(heat_cnt  == (heat_all_time-BEFORE_FINISH_TIME)/HEAT_ONE_CNT_TIME)
    {
        post_msg_to_manager(op_heat_end_soon);

    }
    /*one session finish*/
    if(heat_cnt==heat_all_cnt)
    {
        heat_complete_state=1;
        post_msg_to_manager_with_arg(op_heat_finish,HEAT_FINISH_COMPLETELY);
        app_stop_heat();
    }

}
/*************************************************************************************************
  * @brief    : turn on heat session extend function
  * @param    : null
  * @return   : None
*************************************************************************************************/
void app_session_extend_function_on(void)
{
    uint32_t zone1_heat_all_time=0;
    uint32_t zone2_heat_all_time=0;

    g_session_extend_flag = SESSION_EXTEND_FUCTION_ON;
    //session extend to the last stage
    zone1_heat_all_time = (pzone1_prof.heat_para[pzone1_prof.count-1].time)*1000;
    zone2_heat_all_time= (pzone2_prof.heat_para[pzone2_prof.count-1].time)*1000;
    heat_all_time = zone1_heat_all_time >= zone2_heat_all_time? zone1_heat_all_time:zone2_heat_all_time;
    heat_all_cnt = heat_all_time/HEAT_ONE_CNT_TIME;;
}

/*************************************************************************************************
  * @brief    : get the g_session_extend_flag
  * @param    : null
  * @return   : g_session_extend_flag
*************************************************************************************************/
uint8_t app_get_session_extend_flag(void)
{
    return g_session_extend_flag;
}
/*************************************************************************************************
  * @brief    : Start heat
  * @param1   : 'n'/'b'
  * @return   : None
*************************************************************************************************/
void app_start_heat(char heat_type)
{
    data_change_frequent_t* pDataChangeFreq = NULL;
    //hwi_GPIO_WritePin(BOOST_EN_E,HWI_PIN_SET);
    //hwi_GPIO_WritePin(EN_7V6_SW_E,HWI_PIN_SET);
    app_get_duty();
    LOGD("heat start ...");
    //dev_pwm_set_duty(pwm_dac, pwm_dac_duty);
    //now_pwm= pwm_dac_duty;
    //LOGD("pwd dac pwm_dac_duty is %d\r\n", pwm_dac_duty);
    heat_cnt=0;
    //heat_frame=0;
    heat_up_count_down=0;
    session_count_down=0;
    heat_coil1_checkout_cnt=0;
    heat_coil2_checkout_cnt=0;
    coil1_start_state=0;
    coil2_start_state=0;
    heat_zone1_stage=0;
    heat_zone2_stage=0;
    coil1_need_heat=0;
    coil2_need_heat=0;
    total_joules=0.0;
    heat_chan=0;
    heat_complete_state=0;
    isense_damage_error_cnt=0;
//    tc_spike_error_cnt=0;
    pv1_sum=pv2_sum=0;
//    max_zone1_temp=0;
//    max_zone2_temp=0;
//    max_bat_temp=0;
//    max_cold_junc_temp=0;
    heat_enable_flag_1s=0;
    heat_times = 0;
    old_zone1_temp = dev_get_adc_result()->zone1_temp;
    old_zone2_temp = dev_get_adc_result()->zone2_temp;
    if(pre_heat_stop_time!=0)
    {
        next_heat_start_time=GetTick();
    }
    memset(pv1_array,0,sizeof(pv1_array));
    memset(pv2_array,0,sizeof(pv2_array));

    if(heat_state!=0)
    {
        LOGE("do not repeat heat!!");
        return;
    }
    pDataChangeFreq = get_data_change_frequent_from_ram();
    if(pre_session_errorCode_check()!=0)
    {
        pDataChangeFreq->lifeCycleData[sessions_aborted]++;
        return;
    }
    /*if now state is locked*/
    if(heat_lock_state!=0)
    {
        if(heat_lock_state==1)
        {
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_spike);
            pDataChangeFreq->lifeCycleData[sessions_aborted]++;
            LOGE("heat ERROR: FLT_DE_TC_SPIKE");
        }
        /*
        if(heat_lock_state==2)
        {
            post_msg_to_manager(op_heat_finish);
            post_msg_to_manager_with_arg(op_error_occur,flt_de_tc_high_cjr);
            pDataChangeFreq->lifeCycleData[sessions_aborted]++;
            LOGE("heat error stop: FLT_DE_TC_HIGH_CJR\n\r");
        }
        */
        LOGE("heat ERROR: device in lock 90s!");
        return;
    }
     if(next_heat_start_time - pre_heat_stop_time <30*1000)
    {
        pDataChangeFreq->session_data.b2b_sessions++;
    }
    heat_state=1;

    //TIMER_SAFE_RESET(heat_unit_timer, HEAT_ONE_CNT_TIME, TIMER_OPT_PERIOD, heat_one_cnt, NULL);
    heat_unit_timer = bat_timer_reset_ext(heat_unit_timer, "heat_unit_timer", HEAT_ONE_CNT_TIME, TIMER_OPT_PERIOD, heat_one_cnt);
    bat_timer_start(heat_unit_timer, portMAX_DELAY);
    pDataChangeFreq->lifeCycleData[session_starts]++;
}
void app_stop_heat(void)
{
    data_change_frequent_t* pDataChangeFreq = NULL;
    //now_pwm=0;
    //dev_pwm_set_duty(pwm_dac, now_pwm);
    //hwi_GPIO_WritePin(BOOST_EN_E,HWI_PIN_RESET);
    //TIMER_SAFE_DELETE(heat_unit_timer);
    if(heat_unit_timer){
        if(bat_timer_delete(heat_unit_timer, portMAX_DELAY)==pdPASS){
            heat_unit_timer = NULL;
        }
    }
#ifdef HTR_EN_1
    app_heat1_enable(0);
#endif
#ifdef HTR_EN_2
    app_heat2_enable(0);
#endif
    LOGD("heat stop ...");
    heat_state=0;
    /*do not post this message while heating because will heat again*/
    //post_msg_to_manager(op_heat_finish);
    pDataChangeFreq = get_data_change_frequent_from_ram();

    b2b.pre_heat_finish_tick = GetTick();//record the tick when the last heating was finished
    if(heat_complete_state==1)
    {
        pDataChangeFreq->lifeCycleData[sessions_finished_normally]++;
        pDataChangeFreq->session_data.completed_sessions++;
    }else{
        pDataChangeFreq->lifeCycleData[sessions_aborted]++;
        b2b.pre_heat_finish_tick -= 30*1000;//make next session not a b2b session
        pDataChangeFreq->session_data.aborted_sessions++;
    }
    pre_heat_stop_time=GetTick();
}
/*************************************************************************************************
  * @brief    : start boost heat
  * @return   : None
*************************************************************************************************/
void app_start_boost_heat(void)
{
    LOGD("heat app_start_boost_heat ... ");
    //pzone1_prof =(heat_profile_t*) malloc(sizeof(heat_profile_t));
    //pzone2_prof =(heat_profile_t*) malloc(sizeof(heat_profile_t));
    heat_profile_t zone = {0};
    heat_profile_t *pzone = &zone;
    uint32_t zone1_heat_all_time=0;
    uint32_t zone2_heat_all_time=0;
    //no session extend function for boost heating
    g_session_extend_flag = NONE_SESSION_EXTEND;
  
    /*get zone1 boost heat profile*/
    app_get_heat_profile('b', 1, pzone);
    //if(pzone!=NULL){
    if(pzone->count > 0 && pzone->count <= MAX_HEAT_PARA_LEN){
        //  LOGD("heat profile zone1: \r\n
        memset(&pzone1_prof,0,sizeof(heat_profile_t));
        memcpy(&pzone1_prof,pzone,sizeof(heat_profile_t));
        /*if datas are invalid*/
        for(uint8_t j=0; j<(pzone1_prof.count-1);j++)
        {
            if(pzone1_prof.heat_para[j].time>pzone1_prof.heat_para[j+1].time)
            {
                post_msg_to_manager_with_arg(op_heat_finish,HEAT_FINISH_PROFILE_ERROR);
                LOGE("heat start NOK:boost zone1 profile ERR");
                return;
            }
        }
        for(uint8_t i=0; i<pzone1_prof.count; i++){
            LOGD("zone1: %d time:%d temp:%d\r\n", i, pzone1_prof.heat_para[i].time, pzone1_prof.heat_para[i].temp);
        }
     
        zone1_heat_all_time = (pzone1_prof.heat_para[pzone1_prof.count-1].time)*1000;
    }else{
        /*use defaule heat profile*/
        app_set_default_heat_profile(&pzone1_prof,'b',1);
#if defined (HTR_EN_1) && defined (HTR_EN_2)
        zone1_heat_all_time=195*1000;
#else
        zone1_heat_all_time=200*1000;
#endif
        for(uint8_t i=0; i<pzone1_prof.count; i++){
            LOGD("default zone1: %d time:%d temp:%d\r\n", i, pzone1_prof.heat_para[i].time, pzone1_prof.heat_para[i].temp);
        }
    }
    /*get zone2 boost heat profile*/
      app_get_heat_profile('b', 2, pzone);
    //if(pzone!=NULL){
    if(pzone->count > 0 && pzone->count <= MAX_HEAT_PARA_LEN){
//      LOGD("heat profile zone2: \r\n");
        memset(&pzone2_prof,0,sizeof(heat_profile_t));
        memcpy(&pzone2_prof,pzone,sizeof(heat_profile_t));
        
        
        zone2_heat_all_time= (pzone2_prof.heat_para[pzone2_prof.count-1].time)*1000;
        /*if datas are invalid*/
        for(uint8_t j=0; j<(pzone2_prof.count-1);j++)
        {
            if(pzone2_prof.heat_para[j].time>pzone2_prof.heat_para[j+1].time)
            {
                post_msg_to_manager_with_arg(op_heat_finish,HEAT_FINISH_PROFILE_ERROR);
                LOGE("heat start NOK:boost zone2 profile ERR");
                return;
            }
        }
        for(uint8_t i=0; i<pzone2_prof.count; i++){
            LOGD("zone2: %d time:%d temp:%d\r\n", i, pzone2_prof.heat_para[i].time, pzone2_prof.heat_para[i].temp);
        }
    }else{
        /*use defaule heat profile*/
        app_set_default_heat_profile(&pzone2_prof,'b',2);
#if defined (HTR_EN_1) && defined (HTR_EN_2)
        zone2_heat_all_time=195*1000;
#else
        zone2_heat_all_time=200*1000;
#endif
        for(uint8_t i=0; i<pzone2_prof.count; i++){
            LOGD("default zone2: %d time:%d temp:%d\r\n", i, pzone2_prof.heat_para[i].time, pzone2_prof.heat_para[i].temp);
        }
    }
    heat_all_time = zone1_heat_all_time >= zone2_heat_all_time? zone1_heat_all_time:zone2_heat_all_time;
    heat_all_cnt =heat_all_time/HEAT_ONE_CNT_TIME;
    app_start_heat('b');
}
/*************************************************************************************************
  * @brief    : start normal heat
  * @return   : None
*************************************************************************************************/
void app_start_normal_heat(void)
{
    LOGD("heat app_start_normal_heat ... ");
    //pzone1_prof =(heat_profile_t*) malloc(sizeof(heat_profile_t));
    //pzone2_prof =(heat_profile_t*) malloc(sizeof(heat_profile_t));
    heat_profile_t zone = {0};
    heat_profile_t *pzone = &zone;
    uint32_t zone1_heat_all_time=0;
    uint32_t zone2_heat_all_time=0;
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    //set the session extend function for normal heating 
    if(pfrt->extend_mode == EXTEND_UI_ON)
    {
        g_session_extend_flag = HAVE_SESSION_EXTEND;
    }
    else
    {
        g_session_extend_flag = NONE_SESSION_EXTEND;
    }
        
    /*get zone1 normale heat profile*/
    app_get_heat_profile('n', 1, pzone);
    //if(pzone!=NULL){
    if(pzone->count > 0 && pzone->count <= MAX_HEAT_PARA_LEN){
//      LOGD("heat profile zone1: \r\n");
        memset(&pzone1_prof,0,sizeof(heat_profile_t));
        memcpy(&pzone1_prof,pzone,sizeof(heat_profile_t));
        

        
        if(g_session_extend_flag == HAVE_SESSION_EXTEND)
        {
            //Array index out of bounds check
            if(pzone1_prof.count<2)
            {
                post_msg_to_manager_with_arg(op_heat_finish,HEAT_FINISH_PROFILE_ERROR);
                LOGE("heat start NOK:normal zone1 profile ERR");
                return;
            }
            //as session_extend function, default heating all time should not include the last count
            zone1_heat_all_time = (pzone1_prof.heat_para[pzone1_prof.count-2].time)*1000;
        }
        else 
        {
            //as EXTEND_UI_OFF, heating all time include the last count
            zone1_heat_all_time = (pzone1_prof.heat_para[pzone1_prof.count-1].time)*1000;
        }
        
        /*if datas are invalid*/
        for(uint8_t j=0; j<(pzone1_prof.count-1);j++)
        {
            if(pzone1_prof.heat_para[j].time>pzone1_prof.heat_para[j+1].time)
            {
                post_msg_to_manager_with_arg(op_heat_finish,HEAT_FINISH_PROFILE_ERROR);
                LOGE("heat start NOK:normal zone1 profile ERR");
                return;
            }
        }
        for(uint8_t i=0; i<pzone1_prof.count; i++){
            LOGD("zone1: %d time:%d temp:%d", i, pzone1_prof.heat_para[i].time, pzone1_prof.heat_para[i].temp);
        }
    }else{
        /*use defaule heat profile*/
        app_set_default_heat_profile(&pzone1_prof,'n',1);
#if defined (HTR_EN_1) && defined (HTR_EN_2)
        zone1_heat_all_time=260*1000;
#else
        zone1_heat_all_time=270*1000;
#endif
        for(uint8_t i=0; i<pzone1_prof.count; i++){
            LOGD("default zone1: %d time:%d temp:%d", i, pzone1_prof.heat_para[i].time, pzone1_prof.heat_para[i].temp);
        }
    }
    /*get zone2 normale heat profile*/
      app_get_heat_profile('n', 2, pzone);
    //if(pzone!=NULL){
    if(pzone->count > 0 && pzone->count <= MAX_HEAT_PARA_LEN){
//      LOGD("heat profile zone2: \r\n");
        memset(&pzone2_prof,0,sizeof(heat_profile_t));
        memcpy(&pzone2_prof,pzone,sizeof(heat_profile_t));
       
        if(g_session_extend_flag == HAVE_SESSION_EXTEND)
        {
            //Array index out of bounds check
            if(pzone2_prof.count<2)
            {
                post_msg_to_manager_with_arg(op_heat_finish,HEAT_FINISH_PROFILE_ERROR);
                LOGE("heat start NOK:normal zone2 profile ERR");
                return;
            }
            //as session_extend function, default heating all time should not include the last count
            zone2_heat_all_time = (pzone2_prof.heat_para[pzone2_prof.count-2].time)*1000;
        }
        else 
        {
            //as EXTEND_UI_OFF, heating all time include the last count
            zone2_heat_all_time = (pzone2_prof.heat_para[pzone2_prof.count-1].time)*1000;
        }
  
        
       
        /*if datas are invalid*/
        for(uint8_t j=0; j<(pzone2_prof.count-1);j++)
        {
            if(pzone2_prof.heat_para[j].time>pzone2_prof.heat_para[j+1].time)
            {
                post_msg_to_manager_with_arg(op_heat_finish,HEAT_FINISH_PROFILE_ERROR);
                LOGE("heat start NOK:normal zone2 profile ERR");
                return;
            }
        }

        for(uint8_t i=0; i<pzone2_prof.count; i++){
            LOGD("zone2: %d time:%d temp:%d", i, pzone2_prof.heat_para[i].time, pzone2_prof.heat_para[i].temp);
        }
    }else{
        /*use defaule heat profile*/
        app_set_default_heat_profile(&pzone2_prof,'n',2);
#if defined (HTR_EN_1) && defined (HTR_EN_2)
        zone2_heat_all_time=260*1000;
#else
        zone2_heat_all_time=270*1000;
#endif

        for(uint8_t i=0; i<pzone2_prof.count; i++){
            LOGD("default zone2: %d time:%d temp:%d", i, pzone2_prof.heat_para[i].time, pzone2_prof.heat_para[i].temp);
        }
    }

    heat_all_time = zone1_heat_all_time >= zone2_heat_all_time? zone1_heat_all_time:zone2_heat_all_time;
    heat_all_cnt =heat_all_time/HEAT_ONE_CNT_TIME;
    app_start_heat('n');
}
/*************************************************************************************************
  * @brief    : start TC heat for CIT test
  * @param1   : 1/2
  * @param2   : need heat ms
  * @return   : None
*************************************************************************************************/
void app_start_tc_heat_test(uint8_t number,uint8_t time)
{
    if(number==1)
    {
        memset(&pzone1_prof,0,sizeof(heat_profile_t));
        pzone1_prof.count=1;
        pzone1_prof.heat_para[0].time=time;
        pzone1_prof.heat_para[0].temp=250;
        memset(&pzone2_prof,0,sizeof(heat_profile_t));
        pzone2_prof.count=1;
        pzone2_prof.heat_para[0].time=time;
        pzone2_prof.heat_para[0].temp=0;
    }else if(number==2)
    {
        memset(&pzone1_prof,0,sizeof(heat_profile_t));
        pzone1_prof.count=1;
        pzone1_prof.heat_para[0].time=time;
        pzone1_prof.heat_para[0].temp=0;
        memset(&pzone2_prof,0,sizeof(heat_profile_t));
        pzone2_prof.count=1;
        pzone2_prof.heat_para[0].time=time;
        pzone2_prof.heat_para[0].temp=250;
    }
    heat_all_time = time*1000;
    heat_all_cnt =heat_all_time/HEAT_ONE_CNT_TIME;
    app_start_heat('c');
}
/*************************************************************************************************
  * @brief    : start quick heat for CIT test
  * @return   : None
*************************************************************************************************/
void app_start_quick_heat_test()
{
    change_state(STATE_NORMAL_HEAT);
    memset(&pzone1_prof,0,sizeof(heat_profile_t));
    pzone1_prof.count=2;
    pzone1_prof.heat_para[0].time=60;
    pzone1_prof.heat_para[0].temp=254;
    pzone1_prof.heat_para[1].time=90;
    pzone1_prof.heat_para[1].temp=226;

    memset(&pzone2_prof,0,sizeof(heat_profile_t));
    pzone2_prof.count=3;
    pzone2_prof.heat_para[0].time=30;
    pzone2_prof.heat_para[0].temp=0;
    pzone2_prof.heat_para[1].time=50;
    pzone2_prof.heat_para[1].temp=155;
    pzone2_prof.heat_para[2].time=90;
    pzone2_prof.heat_para[2].temp=250;

    heat_all_time = 90*1000;
    heat_all_cnt =heat_all_time/HEAT_ONE_CNT_TIME;
    app_start_heat('n');
}

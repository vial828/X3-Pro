#include "HWI_Hal.h"
#include "dev_pwm_ll.h"
#include "kernel.h"
#include "manager.h"
#include "log.h"
#include "app_haptic.h"
#include "self_flash.h"
#include "dev_adc.h"
#include "HWI_gpio.h"
#include <string.h>
#include "batTimer.h"


static ptimer_t g_p_haptic_timer = NULL;
static ptimer_t long_ramp_haptic_timer = NULL;
static uint8_t g_hp_count;  /*buzz count*/
static uint16_t long_ramp_time;
static uint8_t long_ramp_count;
static char current_haptic_mode;
static haptic_mode_t current_haptic_mode_s = {0};
//static uint8_t data_diff_intensity[HP_MAX_INTENSITY_LEVEL]={0,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100};
//static uint16_t data_volt[HP_MAX_INTENSITY_LEVEL]={0,2000,2100,2300,2400,2500,2600,2700,2800,2900,3000,3100,3200,3300,3400,3500,3600};
static void call_back_haptic_start(const ptimer_t tm);
static void call_back_haptic_stop(const ptimer_t tm);

static uint8_t get_haptic_intensity(uint16_t haptic_volt)
{
    uint16_t haptic_intensity;
    uint16_t batv = dev_get_adc_result()->vbat*1000;

    if(batv == 0)
    {
        return 0;
    }
    haptic_intensity = (uint16_t)(haptic_volt*100/ batv);
    if(haptic_intensity > 100)
    {
        haptic_intensity = 100;
    }
    return haptic_intensity;
}

/*************************************************************************************************
  * @brief    : Haptic_On_flash
  * @param1   : void
  * @return   : void
  * @Instance : Haptic_On();
  * @Note     : 1. use for cit
*************************************************************************************************/
static void haptic_On_flash(void)
{
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    uint16_t haptic_intensity;
    if(pfrt->haptic_volt>3600)
    {
        LOGD("error haptic volt,set default");
        pfrt->haptic_volt = HAPTIC_VOLT_DEFAULT;
        update_data_flash(USR_DATA,INVALID);
    }

    haptic_intensity = get_haptic_intensity(pfrt->haptic_volt);

    dev_pwm_set_duty(pwm_haptic,haptic_intensity);
}

static void haptic_On_2v5(void)
{
    uint16_t haptic_intensity_2v5;

    haptic_intensity_2v5 = get_haptic_intensity(2500);

    dev_pwm_set_duty(pwm_haptic,haptic_intensity_2v5);
}

/*************************************************************************************************
  * @brief    : Haptic_Off
  * @param1   : void
  * @return   : void
  * @Instance : Haptic_Off();
  * @Note     : 1. use for cit
*************************************************************************************************/
static void haptic_Off(void)
{
    dev_pwm_set_duty(pwm_haptic,0);
}
/*************************************************************************************************
 * @brief    : start haptic_long_ramp_down callback
 * @return   : none
************************************************************************************************/
static void call_back_haptic_long_ramp_down(const ptimer_t tm)
{
    uint16_t haptic_intensity;
    uint16_t current_intensity;
    flash_record_t * pfrt = get_self_flash_record_from_ram();

    haptic_intensity = get_haptic_intensity(pfrt->haptic_volt);

    current_intensity = haptic_intensity-long_ramp_count;
    if(haptic_intensity<=long_ramp_count)
    {
        current_intensity=0;
    }
    dev_pwm_set_duty(pwm_haptic,current_intensity);
    long_ramp_count++;
}
/*************************************************************************************************
 * @brief    : for mode c start haptic_long_ramp_down
 * @return   : none
************************************************************************************************/
static void haptic_long_ramp_down(void)
{
    uint16_t haptic_intensity;
    uint16_t haptic_2v5;
    flash_record_t * pfrt = get_self_flash_record_from_ram();

    haptic_intensity = get_haptic_intensity(pfrt->haptic_volt);
    haptic_2v5 = get_haptic_intensity(2500);

    haptic_On_flash();
    if(haptic_intensity - haptic_2v5 <= 0)
    {
        return;
    }
    long_ramp_time = (uint16_t)(current_haptic_mode_s.one_cycle_buzz_s[0].on_time/(haptic_intensity - haptic_2v5));
    long_ramp_count = 1;
    long_ramp_haptic_timer = bat_timer_reset_ext(long_ramp_haptic_timer, "long_ramp_haptic_timer", long_ramp_time,
                                TIMER_OPT_PERIOD, call_back_haptic_long_ramp_down);
    bat_timer_start(long_ramp_haptic_timer, portMAX_DELAY);
}
/*************************************************************************************************
 * @brief    : start haptic_long_ramp_up callback
 * @return   : none
************************************************************************************************/
static void call_back_haptic_long_ramp_up(const ptimer_t tm)
{
    uint16_t current_intensity;
    uint16_t haptic_2v5;

    haptic_2v5 = get_haptic_intensity(2500);
    current_intensity = haptic_2v5 + long_ramp_count;
    if(current_intensity > 100)
    {
        current_intensity = 100;
    }
    dev_pwm_set_duty(pwm_haptic,current_intensity);
    long_ramp_count++;
}
/*************************************************************************************************
 * @brief    : for mode b start haptic_long_ramp_up
 * @return   : none
************************************************************************************************/
static void haptic_long_ramp_up(void)
{
    uint16_t haptic_intensity;
    uint16_t haptic_2v5;
    flash_record_t * pfrt = get_self_flash_record_from_ram();

    haptic_intensity = get_haptic_intensity(pfrt->haptic_volt);
    haptic_2v5 = get_haptic_intensity(2500);
    haptic_On_2v5();

    if(haptic_intensity - haptic_2v5 <= 0)
    {
        return;
    }
    long_ramp_count = 1;
    long_ramp_time = (uint16_t)(current_haptic_mode_s.one_cycle_buzz_s[0].on_time/(haptic_intensity - haptic_2v5));
    long_ramp_haptic_timer = bat_timer_reset_ext(long_ramp_haptic_timer, "long_ramp_haptic_timer", long_ramp_time,
                                TIMER_OPT_PERIOD, call_back_haptic_long_ramp_up);
    bat_timer_start(long_ramp_haptic_timer, portMAX_DELAY);
}

/*************************************************************************************************
 * @brief    : haptic enable
 * @parm1    : enable or disable
 * @return   : none
************************************************************************************************/
void app_haptic_enable(uint8_t en)
{
    uint16_t haptic_pwm_freq;
    if(en)
    {
        haptic_pwm_freq = get_self_flash_record_from_ram()->haptic_pwm_freq;
        if(haptic_pwm_freq>1200)
        {
            LOGD("error haptic freq,set default");
            get_self_flash_record_from_ram()->haptic_pwm_freq = PWM_FREQ_HAPTIC;
            haptic_pwm_freq = PWM_FREQ_HAPTIC;
            update_data_flash(USR_DATA,INVALID);
        }
        dev_set_pwm_htr_clock(pwm_haptic,haptic_pwm_freq);

        if(current_haptic_mode == 'b' )
        {
            haptic_long_ramp_up();
        }
        else if(current_haptic_mode == 'c')
        {
            haptic_long_ramp_down();
        }
        else{
            haptic_On_flash();
        }
    }
    else
    {
        haptic_Off();
    }
}

uint16_t app_get_haptic_intensity(void)
{
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    return pfrt->haptic_volt;
}

void app_set_haptic_intensity(uint16_t haptic_volt)
{
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    pfrt->haptic_volt = haptic_volt;
}

/*************************************************************************************************
 * @brief    : stop haptic
 * @return   : none
************************************************************************************************/
void app_haptic_shutdown(void)
{
    dev_pwm_set_duty(pwm_haptic,0);
}

/*************************************************************************************************
 * @brief    : start haptic timer callback 
 * @return   : none
************************************************************************************************/
static void call_back_haptic_start(const ptimer_t tm)
{
    if(current_haptic_mode_s.one_cycle_buzz_s[g_hp_count].on_time < HP_MIN_ON_TIME){
        LOGE("haptic on time is too short! Not allow to vibrate");
        return;
    }
    app_haptic_enable(1);
    /*start a stop haptic timer*/
    /*timer_reset(g_p_haptic_timer, current_haptic_mode_s.one_cycle_buzz_s[g_hp_count].on_time,
        TIMER_OPT_ONESHOT, call_back_haptic_stop, NULL);*/
    g_p_haptic_timer = bat_timer_reset_ext(g_p_haptic_timer, "g_p_haptic_timer", current_haptic_mode_s.one_cycle_buzz_s[g_hp_count].on_time,
        TIMER_OPT_ONESHOT, call_back_haptic_stop);
    bat_timer_start(g_p_haptic_timer, portMAX_DELAY);

}

/*************************************************************************************************
 * @brief    : stop haptic timer callback 
 * @return   : none
************************************************************************************************/
static void call_back_haptic_stop(const ptimer_t tm)
{
    app_haptic_enable(0);
    current_haptic_mode = NULL;
    g_hp_count++;
    if(g_hp_count >= current_haptic_mode_s.cycle_cnt){
//      dev_pwm_set_duty(pwm_haptic, 0);
//      app_haptic_enable(0);
        //TIMER_SAFE_DELETE(g_p_haptic_timer);
        if(g_p_haptic_timer){
            if(bat_timer_delete(g_p_haptic_timer, portMAX_DELAY)==pdPASS){
                g_p_haptic_timer = NULL;
            }
        }
        if(long_ramp_haptic_timer){
            if(bat_timer_delete(long_ramp_haptic_timer, portMAX_DELAY)==pdPASS){
                long_ramp_haptic_timer = NULL;
        }
    }
    }else{
        /*start a start haptic timer*/
        /*timer_reset(g_p_haptic_timer, current_haptic_mode_s.one_cycle_buzz_s[g_hp_count - 1].off_time,
            TIMER_OPT_ONESHOT, call_back_haptic_start, NULL);*/
        if(g_hp_count<1||(g_hp_count -1)>=HP_MAX_CYCLE_CNT){
            LOGE("buzz count value error!");
            return;
        }
        g_p_haptic_timer = bat_timer_reset_ext(g_p_haptic_timer, "g_p_haptic_timer", current_haptic_mode_s.one_cycle_buzz_s[g_hp_count - 1].off_time,
            TIMER_OPT_ONESHOT, call_back_haptic_start);
        bat_timer_start(g_p_haptic_timer, portMAX_DELAY);
    }
}

/*************************************************************************************************
 * @brief    : set buzz pattern
 * @parm1    : buzz duration
 * @parm1    : buzz gap
 * @parm1    : buzz count
 * @return   : none
************************************************************************************************/
//static void haptic_set_buzz_ms(uint32_t ms, uint32_t gap, uint8_t cnt)
//{
//    if(g_p_haptic_timer == NULL){
//        g_hp_count = cnt;
//        g_hp_ms = ms;
//        g_hp_gap_ms = gap;
//        app_haptic_enable(1);
//        /*use a soft timer to implemant buzz pattern*/
//        g_p_haptic_timer = timer_create(g_hp_ms, TIMER_OPT_ONESHOT, call_back_haptic_stop, NULL);
//    }
//}


/*************************************************************************************************
 * @brief    : set buzz gap duration
 * @parm1    : buzz duration
 * @return   : none
************************************************************************************************/
//void haptic_set_buzz_gap_ms(uint32_t ms)
//{
//    g_hp_gap_ms = ms;
//}

/*************************************************************************************************
 * @brief    : Haptic buzz in one mode
 * @parm1    : Buzz mode
 * @return   : none
************************************************************************************************/
void app_haptic_buzz(char mode)
{
    flash_record_t* pfrt = get_self_flash_record_from_ram();
    memset(&current_haptic_mode_s, 0, sizeof(haptic_mode_t));
    //TIMER_SAFE_DELETE(g_p_haptic_timer);
    if(g_p_haptic_timer){
        if(bat_timer_delete(g_p_haptic_timer, portMAX_DELAY)==pdPASS){
            g_p_haptic_timer = NULL;
        }
    }
    if(long_ramp_haptic_timer){
        if(bat_timer_delete(long_ramp_haptic_timer, portMAX_DELAY)==pdPASS){
            long_ramp_haptic_timer = NULL;
        }
    }
    g_hp_count = 0;
    if(mode == 'a')
    {
        memcpy(&current_haptic_mode_s, &(pfrt->haptic_a), sizeof(haptic_mode_t));
    }
    else if(mode == 'b')
    {
        memcpy(&current_haptic_mode_s, &(pfrt->haptic_b), sizeof(haptic_mode_t));
    }
    else if(mode == 'c')
    {
        memcpy(&current_haptic_mode_s, &(pfrt->haptic_c), sizeof(haptic_mode_t));
    }
    else if(mode == 'd')
    {
        memcpy(&current_haptic_mode_s, &(pfrt->haptic_d), sizeof(haptic_mode_t));
    }
    else
    {
        LOGE("wrong haptic mode!");
        return;
    }
    if(current_haptic_mode_s.one_cycle_buzz_s[g_hp_count].on_time < HP_MIN_ON_TIME){
        LOGE("haptic on time is too short! Not allow to vibrate");
        return;
    }
    current_haptic_mode = mode;
    app_haptic_enable(1);
    /*g_p_haptic_timer = timer_create(current_haptic_mode_s.one_cycle_buzz_s[g_hp_count].on_time,
            TIMER_OPT_ONESHOT, call_back_haptic_stop, NULL);*/
    g_p_haptic_timer = bat_timer_reset_ext(g_p_haptic_timer, "g_p_haptic_timer", current_haptic_mode_s.one_cycle_buzz_s[g_hp_count].on_time,
        TIMER_OPT_ONESHOT, call_back_haptic_stop);
    bat_timer_start(g_p_haptic_timer, portMAX_DELAY);
    //haptic_set_buzz_ms(buzz_a_ms, 250, 1);
}

/*************************************************************************************************
 * @brief    : buzz A pattern
 * @return   : none
************************************************************************************************/
//void haptic_buzz_A(void)
//{
//    /*for one count buzz the gap param is INVALID*/
//    haptic_set_buzz_ms(buzz_a_ms, 250, 1);
//}

/*************************************************************************************************
 * @brief    : buzz B pattern
 * @return   : none
************************************************************************************************/
//void haptic_buzz_B(void)
//{
//    haptic_set_buzz_ms(buzz_b_ms, buzz_b_gap_ms, 2);
//}

/*************************************************************************************************
 * @brief    : buzz C pattern
 * @return   : none
************************************************************************************************/
//void haptic_buzz_C(void)
//{
//    /*for one count buzz the gap param is INVALID*/
//    haptic_set_buzz_ms(buzz_c_ms, 250, 1);
//}

/*************************************************************************************************
 * @brief    : set buzz A para
 * @parm1    : buzz duration
 * @return   : none
************************************************************************************************/
//void haptic_set_A_para(uint16_t ms)
//{
//    /*read the current duration from flash*/
//    flash_record_t* pfrt = get_self_flash_record_from_ram();
//    buzz_a_ms = ms;
//    pfrt->buzz_a_ms = buzz_a_ms;
//    /*store the new duration to flash*/
//    update_data_flash(USR_DATA,INVALID);
//    LOGD("haptic A is:%d ms\r\n", buzz_a_ms);
//}

/*************************************************************************************************
 * @brief    : set buzz B para
 * @parm1    : buzz duration
 * @parm1    : buzz gap
 * @return   : none
************************************************************************************************/
//void haptic_set_B_para(uint16_t ms, uint16_t gap)
//{
//    /*read the current duration and gap from flash*/
//    flash_record_t* pfrt = get_self_flash_record_from_ram();
//    buzz_b_ms = ms;
//    buzz_b_gap_ms = gap;
//    pfrt->buzz_b_ms = buzz_b_ms;
//    pfrt->buzz_b_gap_ms = buzz_b_gap_ms;
//    /*store the new parm to flash*/
//    update_data_flash(USR_DATA,INVALID);

//    LOGD("haptic B is:%d gap is:%d\r\n", buzz_b_ms, buzz_b_gap_ms);
//}

/*************************************************************************************************
 * @brief    : set buzz C para
 * @parm1    : buzz duration
 * @return   : none
************************************************************************************************/
//void haptic_set_C_para(uint16_t ms)
//{
//    /*read the current duration from flash*/
//    flash_record_t* pfrt = get_self_flash_record_from_ram();
//    buzz_c_ms = ms;
//    pfrt->buzz_c_ms = buzz_c_ms;
//    /*store the new parm to flash*/
//    update_data_flash(USR_DATA,INVALID);
//    LOGD("haptic C is:%d ms\r\n", buzz_c_ms);
//}

void app_haptic_set_parameter(uint8_t mode, uint8_t* pdata, uint16_t data_len)
{
    if(!pdata){
        LOGE("pdata is null!");
        return;
    }
    flash_record_t* pfrt = get_self_flash_record_from_ram();
    switch(mode)
    {
        case 1:
            memcpy(&(pfrt->haptic_a), pdata, data_len);
            break;
        case 2:
            memcpy(&(pfrt->haptic_b), pdata, data_len);
            break;
        case 3:
            memcpy(&(pfrt->haptic_c), pdata, data_len);
            break;
        case 4:
            memcpy(&(pfrt->haptic_d), pdata, data_len);
            break;
        default:
            LOGE("wrong haptic mode!");
            break;
    }
    update_data_flash(USR_DATA,INVALID);
}

/*************************************************************************************************
 * @brief    : restore default buzz pattern
 * @parm1    : buzz type 
 * @return   : none
************************************************************************************************/
void app_haptic_restore_default(uint8_t ptype)
{
    flash_record_t* pfrt = get_self_flash_record_from_ram();
    LOGD("haptic_restore_default:ptype=%d\n\r",ptype);
    switch(ptype)
    {
        case 1:
            memset(&(pfrt->haptic_a), 0, sizeof(haptic_mode_t));
            pfrt->haptic_a.cycle_cnt = 1;
            pfrt->haptic_a.one_cycle_buzz_s[0].on_time = 250;
            break;
        case 2:
            memset(&(pfrt->haptic_b), 0, sizeof(haptic_mode_t));
            pfrt->haptic_b.cycle_cnt = 1;
            pfrt->haptic_b.one_cycle_buzz_s[0].on_time = 750;
            break;
        case 3:
            memset(&(pfrt->haptic_c), 0, sizeof(haptic_mode_t));
            pfrt->haptic_c.cycle_cnt = 1;
            pfrt->haptic_c.one_cycle_buzz_s[0].on_time = 750;
            break;
        case 4:
            memset(&(pfrt->haptic_d), 0, sizeof(haptic_mode_t));
            pfrt->haptic_d.cycle_cnt = 3;
            pfrt->haptic_d.one_cycle_buzz_s[0].on_time = 750;
            pfrt->haptic_d.one_cycle_buzz_s[0].off_time = 750;
            pfrt->haptic_d.one_cycle_buzz_s[1].on_time = 750;
            pfrt->haptic_d.one_cycle_buzz_s[1].off_time = 750;
            pfrt->haptic_d.one_cycle_buzz_s[2].on_time = 750;
            pfrt->haptic_d.one_cycle_buzz_s[2].off_time = 750;
            break;
        default:
            break;
    }
    update_data_flash(USR_DATA,INVALID);
}


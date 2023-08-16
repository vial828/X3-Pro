#include "HWI_Hal.h"
#include "stratos_defs.h"
#include "log.h"
#include "kernel.h"
#include "app_heat.h"
#include "usr_cmd.h"
#include "message.h"
#include "manager.h"
#include "comm.h"
#include "version.h"
#include "self_flash.h"
#include "dev_temperature.h"
#include "power.h"
#include "app_charge.h"
#include <string.h>
#include <stdio.h>
#include "app_haptic.h"
#include "error_code_led_output.h"
#include "dev_pwm_ll.h"
#include "self_flash.h"
#include "uart.h"
#include "dev_adc.h"
#include <stdlib.h>
#include "HWI_power.h"
#include "app_oled_UI.h"
#include "app_oled_display.h"
#include "wdg.h"
#include "dev_oled_driver.h"
#include "app_button.h"
#include "dev_bq25898x.h"
#include "cit.h"

uint8_t upload_error_flag = 1;
uint8_t cycle_log = 1;
uint8_t heat_log_16ms = 0;
uint8_t charge_IC_log = 0;
uint8_t no_shutdown_flag = 0;
/*************************************************************************************************
 * @brief   :get system parameter name
 * @param   :system parameter id
 * @return  :system parameter name
*************************************************************************************************/
char* get_system_parameter_name(system_parameter_n i)
{
    if(i < SYSTEM_MAX){
        return system_parameter_name[i];
    }else{
        return "SYSTEM_MAX";
    }
}

/*************************************************************************************************
 * @brief   :get system parameter value
 * @param   :flag bit,cmd data, cmd length
 * @return
*************************************************************************************************/
void set_system_parameter(uint8_t num,uint8_t *pdata, uint16_t data_len)
{
    flash_record_t* pfrt = get_self_flash_record_from_ram();
    switch(num)
    {
        case 1:
            memcpy(&(pfrt->error_parameter), pdata, data_len);
            update_data_flash(USR_DATA,INVALID);
            LOGD("updata system config success");
            break;
        default:
            LOGE("wrong system config num!");
            break;
    }
}

/*************************************************************************************************
  * @brief    : Get flag which controls whether errors should be uploaded to pctool automatically
  * @return   : upload error flag
*************************************************************************************************/
uint8_t get_upload_error_flag(void)
{
    return upload_error_flag;
}

/*************************************************************************************************
  * @brief    : Get flag which controls whether cycle log should be printed
  * @return   : cycle log flag
*************************************************************************************************/
uint8_t get_cycle_log_flag(void)
{
    return cycle_log;
}

/*************************************************************************************************
  * @brief    : Set flag which controls whether cycle log should be printed
  * @param1   : uint8_t(0: not print/1: print)
  * @return   : None
*************************************************************************************************/
void set_cycle_log_flag(uint8_t value)
{
    cycle_log = value;
}

/*************************************************************************************************
  * @brief    : Get flag which controls heating log should be printed every 16ms or 1s
  * @return   : heating 16ms flag
*************************************************************************************************/
uint8_t get_heat_log_16ms_flag(void)
{
    return heat_log_16ms;
}

/*************************************************************************************************
  * @brief    : Get flag which controls whether content of charge IC registers should be printed
  * @return   : charge IC flag
*************************************************************************************************/
uint8_t get_charge_IC_log_flag(void)
{
    return charge_IC_log;
}

/*************************************************************************************************
  * @brief    : Respond one command
  * @param1   : Command index
  * @param2   : Pointer to data
  * @param3   : Data length
  * @return   : None
*************************************************************************************************/
void respond_usr_cmd(uint8_t cmd, uint8_t *pdata, uint16_t len)
{
    comm_send(cmd, PC_ADDR, pdata, len);
}

static uint8_t hp_buffer[128];

/*************************************************************************************************
  * @brief    : Store heating profile in buffer
  * @param1   : Pointer to heating profile
  * @param2   : Pointer to buffer
  * @return   : The bytes occupied by the heating profile in the buffer
*************************************************************************************************/
static uint8_t fill_heat_para_buffer(heat_profile_t *ht, uint8_t *pbuff)
{
    uint16_t time, temp;
    uint16_t i,u;

    pbuff[0] = ht->count;
    for(i = 0; i< ht->count; i++){
        time = ht->heat_para[i].time;
        temp = ht->heat_para[i].temp;
        //LOGD("  %d time:%d temp:%d\r\n", i, time, temp);
        u = 4*i;
        pbuff[u+1] = time >> 8;
        pbuff[u+2] = time & 0x00ff;
        pbuff[u+3] = temp >> 8;
        pbuff[u+4] = temp & 0x00ff;
    }
    return (ht->count * 4 + 1);
}

/*************************************************************************************************
  * @brief    : Send heating profile to pc
  * @param1   : Heating type(normal/boost)
  * @return   : None
*************************************************************************************************/
static void read_heat_profile_to_pc(char type)
{
    //    uint8_t ht2_pos;
    heat_profile_t ht1 = {0};
    heat_profile_t ht2 = {0};
    heat_profile_t* pHt1 = &ht1;
    heat_profile_t* pHt2 = &ht2;
    app_get_heat_profile(type, 1, pHt1);
    app_get_heat_profile(type, 2, pHt2);
    hp_buffer[0] = type;
    hp_buffer[1] = 0x01;

    //LOGD("heat profile zone1: \r\n");
    if((type != 'n') && (type != 'b')){
        hp_buffer[2] = 0;
        //ht2_pos = 3;
    }else{
        //ht2_pos = 2 + fill_heat_para_buffer(pHt1, &hp_buffer[2]);
        fill_heat_para_buffer(pHt1, &hp_buffer[2]);
    }
    respond_usr_cmd(USR_CMD_READ_HP, hp_buffer, (hp_buffer[2])*4+3);
    //hp_buffer[ht2_pos]=0x02;
    //ht2_pos++;
    hp_buffer[0] = type;
    hp_buffer[1] = 0x02;
    //LOGD("heat profile zone2: \r\n");
    if((type != 'n') && (type != 'b')){
        hp_buffer[2] = 0;
    }else{
        fill_heat_para_buffer(pHt2, &hp_buffer[2]);
    }
    respond_usr_cmd(USR_CMD_READ_HP, hp_buffer, (hp_buffer[2])*4+3);
    //respond_usr_cmd(USR_CMD_READ_HP, hp_buffer, (hp_buffer[2]+hp_buffer[ht2_pos])*4+5);
}

/*
    This function is not used
*/
void start_upload_error(void)
{
    LOGD("start upload error! \r\n");
    upload_error_flag = 1;
}

/*************************************************************************************************
  * @brief    : Report current error to pc
  * @return   : None
*************************************************************************************************/
void send_current_error(void)
{
    uint32_t current_error = 0;
    uint64_t error_code_temp = get_error_code();
    LOGD("mc.error_code=0x%llx",error_code_temp);
    current_error = error_code_temp;
    hp_buffer[0] = (current_error >> 24);
    hp_buffer[1] = (current_error >> 16);
    hp_buffer[2] = (current_error >> 8);
    hp_buffer[3] = (current_error >> 0);
    respond_usr_cmd(USR_CMD_READ_CURRENT_ERROR, hp_buffer, 4);
}

/*************************************************************************************************
  * @brief    : Report the occurencese of all errors stored in flash
  * @return   : None
*************************************************************************************************/
void send_error_stastic(void)
{
    uint16_t num = 0;
    uint8_t i = 0;

    //get pointer to data_change_frequent area
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    LOGD("send_error_stastic:error_max=0x%x\r\n",error_max);

    //send the number of occurrences of each error
    while (i < error_max)
    {
        num = pDataChangeFreq->errorCodeData[i];
        hp_buffer[2*i+1] = num;
        hp_buffer[2*i] = (num >> 8);
        i++;
    }
    respond_usr_cmd(USR_CMD_READ_ERROR_STASTIC, hp_buffer, error_max*2);
}

void test_display_error(void)
{
    static uint32_t flag = 0;
    if (0 == flag)
    {
        start_upload_error();
    }
    if (1 == flag)
    {
        LOGD("-----------flt_de_tc_zone_imbalance occur!-----------\r\n");
        post_msg_to_manager_with_arg(op_error_occur, flt_de_tc_zone_imbalance);
    }

    if (2 == flag)
    {
        LOGD("----------------flt_de_bat_cold_heat occur!---------------\r\n");
        post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_cold_heat);
    }
    if (3 == flag)
    {
        send_current_error();
        send_error_stastic();
    }
    flag++;
}

/*************************************************************************************************
  * @brief    : Report lifecycle data to pc
  * @return   : None
*************************************************************************************************/
void send_lifecycle(void)
{
    uint8_t i = 0;
    int32_t num = 0;

    //get pointer to data_change_frequent area
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();

    //send life_cycle data
    while (i < life_cycle_log_max)
    {
        num = pDataChangeFreq->lifeCycleData[i];
        if ((i == in_WH) || (i == out_WH))
        {
            num = num/3600;
        }
        hp_buffer[i*4+3] = num;
        hp_buffer[i*4+2] = (num >> 8);
        hp_buffer[i*4+1] = (num >> 16);
        hp_buffer[i*4] = (num >> 24);
        i++;
    }
    respond_usr_cmd(USR_CMD_READ_LIFECYCLE, hp_buffer, life_cycle_log_max*4);
}

/*************************************************************************************************
  * @brief    : Report the last 100 sessions to pc
  * @return   : None
*************************************************************************************************/
void send_100_sessions(void)
{
    uint8_t i = 0;
    uint8_t session_num = 0;
    uint8_t hp_buffer_write_pos = 0;

    //get pointer to data_change_frequent area
    data_change_frequent_t* pDataChangeFrequent = get_data_change_frequent_from_ram();
    session_t* ptrSession100 = get_100sessions_from_ram();

    if (pDataChangeFrequent->g_sessionNum== 0)
    {
        respond_usr_cmd(USR_CMD_READ_100_SESSIONS, hp_buffer, 0);
        return;
    }
    hp_buffer[0] = pDataChangeFrequent->g_sessionNum;
    hp_buffer[1] = 0;
    hp_buffer_write_pos = 2;

    LOGD("g_sessionWritePos=%d,g_sessionNum=%d\r\n",pDataChangeFrequent->g_sessionWritePos,pDataChangeFrequent->g_sessionNum);
    i = (pDataChangeFrequent->g_sessionWritePos%100);
    while (i > 0)
    {
        //LOGD("i=%d\r\n",i);
        /*LOG_NOW("session:%f,%f,%f,%f,%f\r\n",ptrSession100[i -1].num[0],ptrSession100[i -1].num[1],
                                                                                    ptrSession100[i -1].num[2],ptrSession100[i -1].num[3],
                                                                                    ptrSession100[i -1].num[4]);*/
        memcpy((void*)&hp_buffer[hp_buffer_write_pos], (void *)(&ptrSession100[i -1].num[0]), sizeof(session_t));
        hp_buffer_write_pos += sizeof(session_t);
        session_num++;
        if (session_num == SESSIONS_NUM_IN_ONE_PACKET)
        {
            hp_buffer[1] = SESSIONS_NUM_IN_ONE_PACKET;
            respond_usr_cmd(USR_CMD_READ_100_SESSIONS, hp_buffer, hp_buffer_write_pos);
            hp_buffer[0] = pDataChangeFrequent->g_sessionNum;
            hp_buffer[1] = 0;
            hp_buffer_write_pos = 2;
            session_num = 0;
            //comm_task();
            while(UartStatusCheck() != SET)
            {
                comm_send_proc();
                while(UartStatusCheck() != SET);
            }
        }
        i--;

    }

    if (pDataChangeFrequent->g_sessionNum == 100)
    {
        i = 100;
        while (i > (pDataChangeFrequent->g_sessionWritePos%100))
        {
            //LOGD("i=%d\r\n",i);
            /*LOG_NOW("session:%f,%f,%f,%f,%f\r\n",ptrSession100[i -1].num[0],ptrSession100[i -1].num[1],
                                                                                    ptrSession100[i -1].num[2],ptrSession100[i -1].num[3],
                                                                                    ptrSession100[i -1].num[4]);*/
            memcpy((void*)&hp_buffer[hp_buffer_write_pos], (void *)(&ptrSession100[i-1].num[0]), sizeof(session_t));
            hp_buffer_write_pos += sizeof(session_t);
            session_num++;
            if (session_num == SESSIONS_NUM_IN_ONE_PACKET)
            {
                hp_buffer[1] = SESSIONS_NUM_IN_ONE_PACKET;
                respond_usr_cmd(USR_CMD_READ_100_SESSIONS, hp_buffer, hp_buffer_write_pos);
                hp_buffer[0] = pDataChangeFrequent->g_sessionNum;
                hp_buffer[1] = 0;
                hp_buffer_write_pos = 2;
                session_num = 0;
                //comm_task();
                while(UartStatusCheck() != SET)
                {
                    comm_send_proc();
                    while(UartStatusCheck() != SET);
                }
            }
            i--;
        }
    }

    if (session_num != 0)
    {
        hp_buffer[1] = session_num;
        respond_usr_cmd(USR_CMD_READ_100_SESSIONS, hp_buffer, hp_buffer_write_pos);
        //comm_task();
        while(UartStatusCheck() != SET)
        {
            comm_send_proc();
            while(UartStatusCheck() != SET);
        }
    }
}

void test_display_lifecycle_100_sessions(void)
{
    static uint32_t flag = 0;
    if (flag >= 200)
    {
        return;
    }
    /*if (0 == flag)
    {
        write_life_cycle_value_to_ram(max_bat_chg_vol, 0x55AA);
    }*/
    if (0 == flag%2)
    {
        write_session_to_ram(session_duration, flag);
        write_session_to_ram(max_susceptor_temp1_session, flag);
        write_session_to_ram(max_susceptor_temp2_session, flag);
        write_session_to_ram(max_bat_temp_session, flag);
        write_session_to_ram(max_cold_junc_temp_session, flag);
        update_data_flash(DATA_CHANGE_FREQUENT, SESSION_DATA);
    }
    /*if (2 == flag)
    {
        send_lifecycle();
    }
    if (1 == flag%2)
    {
        send_100_sessions();
    }*/
    flag++;

}

/*************************************************************************************************
  * @brief    : Get the flag of no shutdown
  * @param1   : void
  * @return   : uint8_t
*************************************************************************************************/
uint8_t get_no_shutdown_flag(void)
{
    return no_shutdown_flag;
}

/*************************************************************************************************
  * @brief    : get hardware level through  ADC  value
  * @param  	: void
  * @return   : hardware level
*************************************************************************************************/
uint8_t get_hwid_level(void)
{
    return read_hw_id();
}

/*************************************************************************************************
  * @brief    : restore system parameter as default
  * @param    : void
  * @return   : NONE
*************************************************************************************************/
void restore_default_system_parameter(void)
{
    flash_record_t *frt = get_self_flash_record_from_ram();

    app_set_default_heat_profile(&(frt->zone1_n),'n',1);
    app_set_default_heat_profile(&(frt->zone2_n),'n',2);

    app_set_default_heat_profile(&(frt->zone1_b),'b',1);
    app_set_default_heat_profile(&(frt->zone2_b),'b',2);
    memset(frt->ini_version,0, 32);
    memcpy(frt->ini_version,INI_VERSION, 17);
    frt->charge_temp_protect = BAT_HOT_PROTECT_THRESHOLD;
    frt->charge_temp_protect_relesae = BAT_HOT_PROTECT_RELEASE;
    frt->extend_mode = EXTEND_UI_OFF;
    frt->haptic_volt = HAPTIC_VOLT_DEFAULT;
    frt->haptic_pwm_freq = PWM_FREQ_HAPTIC;
    frt->eol_session = EOL_DEFAULT;
    frt->step1_session_nums = STEP1_NUMS_DEFAULT;
    frt->step2_session_nums = STEP2_NUMS_DEFAULT;
    frt->step3_session_nums = STEP3_NUMS_DEFAULT;
    frt->step4_session_nums = STEP4_NUMS_DEFAULT;
    frt->slow_chg_isense = SLOW_CHG_DEFAULT_MA;
    frt->slow_batv_h = SLOW_DEFAULT_H_BATV;
    frt->slow_batv_l = SLOW_DEFAULT_L_BATV;
    frt->wrong_chg_h_mv = WRONG_CHG_H_MV;
    frt->step1_chg_curr = FAST_CHARGE_CURR_STEP1_LIMIT;
    frt->step1_chg_volt= CHARGE_VOLT_LIMIT_STEP1;
    frt->step2_chg_curr = FAST_CHARGE_CURR_STEP2_LIMIT;
    frt->step2_chg_volt = CHARGE_VOLT_LIMIT_STEP2;
    frt->step3_chg_curr = FAST_CHARGE_CURR_STEP3_LIMIT;
    frt->step3_chg_volt= CHARGE_VOLT_LIMIT_STEP3;
    frt->step4_chg_curr = FAST_CHARGE_CURR_STEP4_LIMIT;
    frt->step4_chg_volt = CHARGE_VOLT_LIMIT_STEP4;
    frt->step2_chg_hot_limit = BAT_HOT_CHARGE_STEP2;
    frt->step2_chg_hot_protect = BAT_HOT_PROTECT_THRESHOLD_STEP2;
    frt->error_parameter.pre_session_temp_limit = PRE_SESSION_TEMP_LIMIT;
    frt->error_parameter.charge_temp_limit = BAT_HOT_CHARGING;
    frt->error_parameter.charge_temp_limit_clear = BAT_HOT_CHARGING_CLEAR;
    frt->error_parameter.bat_cold_charge_temp = BAT_COLD_CHARGE_TEMP;
    frt->error_parameter.bat_cold_charge_temp_clear = BAT_COLD_CHARGE_TEMP_CLEAR;
    frt->error_parameter.bat_hot_temp = BAT_HOT_TEMP;
    frt->error_parameter.bat_hot_temp_clear = BAT_HOT_TEMP_CLEAR;
    frt->error_parameter.bat_cold_heat_temp = BAT_COLD_HEAT_TEMP;
    frt->error_parameter.bat_cold_heat_temp_clear = BAT_COLD_HEAT_TEMP_CLEAR;
    frt->error_parameter.heat_empty_volt = DURING_SESSION_EMPTY_VOLT;
    frt->error_parameter.heat_cutoff_volt = PRE_HEAT_CUTOFF_VOLT;
    frt->error_parameter.heat_cutoff_volt_soc = PRE_HEAT_CUTOFF_VOLT_SOC;
    frt->error_parameter.bat_volt_damage_protect = BAT_VOLT_DAMAGE_PROTECT;
    frt->error_parameter.tc_zone1_hot = TC_ZONE1_HOT;
    frt->error_parameter.tc_zone1_hot_clear = TC_ZONE1_HOT_CLEAR;
    frt->error_parameter.tc_zone2_hot = TC_ZONE2_HOT;
    frt->error_parameter.tc_zone2_hot_clear = TC_ZONE2_HOT_CLEAR;
//    frt->error_parameter.tc_zone1_cold = TC_ZONE1_COLD;
//    frt->error_parameter.tc_zone1_cold_clear = TC_ZONE1_COLD_CLEAR;
//    frt->error_parameter.tc_zone2_cold = TC_ZONE2_COLD;
//    frt->error_parameter.tc_zone2_cold_clear = TC_ZONE2_COLD_CLEAR;
    frt->error_parameter.bat_volt_over = BAT_VOLTAGE_OVER;
    frt->error_parameter.bat_volt_over_clear = BAT_VOLTAGE_OVER_CLEAR;
    frt->error_parameter.discharge_current_over = BAT_DISCHARGE_CURR_OVER;
    frt->error_parameter.charge_current_over = BAT_CHARGE_CURR_OVER;
    frt->error_parameter.co_junc_hot = CO_JUNC_HOT;
    frt->error_parameter.co_junc_hot_clear = CO_JUNC_HOT_CLEAR;
//    frt->error_parameter.co_junc_cold = CO_JUNC_COLD;
//    frt->error_parameter.co_junc_cold_clear = CO_JUNC_COLD_CLEAR;
    frt->error_parameter.i_sense_damage = BAT_I_SENSE_DAMAGE;
    frt->error_parameter.charge_timeout = CHARGE_TIMEOUT;
    frt->error_parameter.coil_hot_temp = COIL_HOT_TEMP;
    frt->error_parameter.coil_hot_temp_clear = COIL_HOT_TEMP_CLEAR;
    frt->error_parameter.usb_hot_temp = USB_HOT_TEMP;
    frt->error_parameter.usb_hot_temp_clear = USB_HOT_TEMP_CLEAR;
//    frt->error_parameter.usb_cold_temp = USB_COLD_TEMP;
//    frt->error_parameter.usb_cold_temp_clear = USB_COLD_TEMP_CLEAR;
    update_data_flash(USR_DATA, INVALID);
    LOGD("system parameter have been restored to defaults ");
}
/*************************************************************************************************
  * @brief    : put system parameter to a pointer
  * @param1   : error_parameter_t struct
  * @return   : NONE
*************************************************************************************************/
void fill_system_para_buffer(void)
{
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    int16_t system_parameter_value[SYSTEM_MAX]={
        pfrt->charge_temp_protect,
        pfrt->charge_temp_protect_relesae,
        pfrt->haptic_volt,
        pfrt->haptic_pwm_freq,
        pfrt->eol_session,
        pfrt->step1_session_nums,
        pfrt->step2_session_nums,
        pfrt->step3_session_nums,
        pfrt->step4_session_nums,
        pfrt->slow_chg_isense,
        pfrt->slow_batv_h,
        pfrt->slow_batv_l,
        pfrt->wrong_chg_h_mv,
        pfrt->step1_chg_curr,
        pfrt->step1_chg_volt,
        pfrt->step2_chg_curr,
        pfrt->step2_chg_volt,
        pfrt->step3_chg_curr,
        pfrt->step3_chg_volt,
        pfrt->step4_chg_curr,
        pfrt->step4_chg_volt,
        pfrt->step2_chg_hot_limit,
        pfrt->step2_chg_hot_protect,
        pfrt->error_parameter.pre_session_temp_limit,
        pfrt->error_parameter.charge_temp_limit,
        pfrt->error_parameter.charge_temp_limit_clear,
        pfrt->error_parameter.bat_cold_charge_temp,
        pfrt->error_parameter.bat_cold_charge_temp_clear,
        pfrt->error_parameter.bat_hot_temp,
        pfrt->error_parameter.bat_hot_temp_clear,
        pfrt->error_parameter.bat_cold_heat_temp,
        pfrt->error_parameter.bat_cold_heat_temp_clear,
        pfrt->error_parameter.heat_empty_volt,
        pfrt->error_parameter.heat_cutoff_volt,
        pfrt->error_parameter.heat_cutoff_volt_soc,
        pfrt->error_parameter.bat_volt_damage_protect,
        pfrt->error_parameter.tc_zone1_hot,
        pfrt->error_parameter.tc_zone1_hot_clear,
        pfrt->error_parameter.tc_zone2_hot,
        pfrt->error_parameter.tc_zone2_hot_clear,
//        pfrt->tc_zone1_cold,
//        pfrt->tc_zone1_cold_clear,
//        pfrt->tc_zone2_cold,
//        pfrt->tc_zone2_cold_clear,
        pfrt->error_parameter.bat_volt_over,
        pfrt->error_parameter.bat_volt_over_clear,
        pfrt->error_parameter.discharge_current_over,
        pfrt->error_parameter.charge_current_over,
        pfrt->error_parameter.co_junc_hot,
        pfrt->error_parameter.co_junc_hot_clear,
//        pfrt->co_junc_cold,
//        pfrt->co_junc_cold_clear,
        pfrt->error_parameter.i_sense_damage,
        pfrt->error_parameter.charge_timeout,
        pfrt->error_parameter.coil_hot_temp,
        pfrt->error_parameter.coil_hot_temp_clear,
        pfrt->error_parameter.usb_hot_temp,
        pfrt->error_parameter.usb_hot_temp_clear,
//        pfrt->usb_cold_temp,
//        pfrt->usb_cold_temp_clear,
    };

    uint8_t count1 = 0;
    uint8_t command_num = 0;
    uint8_t flag = 0;
    char data[120];
    char systemconfig[SYSTEM_MAX][40];

    memset(data,0,120);
    command_num = SYSTEM_MAX/3;

    if(SYSTEM_MAX%3 == 0)
    {
       respond_usr_cmd(USR_CMD_GET_SYSTEM_PARAMETER, &command_num, 1);
    }
    else
    {
        command_num++;
        respond_usr_cmd(USR_CMD_GET_SYSTEM_PARAMETER, &command_num, 1);
    }
    for(count1 = 0; count1 < SYSTEM_MAX; count1++)
    {

        if(count1%3 == 0 || count1%3 == 1)
        {
            flag = 1;
            sprintf(systemconfig[count1],"%s:%d\n",system_parameter_name[count1],system_parameter_value[count1]);
            strncat(data,systemconfig[count1],strlen(systemconfig[count1]));
        }
        if(count1%3 == 2)
        {
            flag = 0;
            sprintf(systemconfig[count1],"%s:%d",system_parameter_name[count1],system_parameter_value[count1]);
            strncat(data,systemconfig[count1],strlen(systemconfig[count1]));
            respond_usr_cmd(USR_CMD_GET_SYSTEM_PARAMETER, (uint8_t *)data, strlen(data));
            memset(data,0,120);
        }
        if(count1 == SYSTEM_MAX-1)
        {
            if(flag == 1)
            {
                respond_usr_cmd(USR_CMD_GET_SYSTEM_PARAMETER, (uint8_t *)data, strlen(data));
            }
        }
    }
}


void get_sw_version(void)
{
    char data[114];
    char data2[100];
    char HWIDdata[20];
    char chargeIC[20];
    char ledIC[20];
    char hapticIC[20];

    boot_record_t * brt = get_boot_record_from_ram();
    ext_flash_record_t * ext_flash_brt = get_ext_flash_record_from_ram();

    memset(data, 0, 114);
    memset(data2, 0, 100);

    strncat(data, (char *)brt->bootloader_version, 30);
    if(!strncmp(data, BL_VERSION, strlen(BL_VERSION)))
    {
        LOGD("boot:%s\n\r", data);
        sprintf(data,"%s\r\n%s", DEVICE_APP_VER, (char *)brt->bootloader_version);
        //snprintf(data, 30, "%s\r\n", DEVICE_APP_VER);
        //strncat(data, (char *)brt->bootloader_version, 30);
    }
    else{
        LOGD("no bootloader ver\n\r");
        sprintf(data, "%s\r\n%s", DEVICE_APP_VER, "Device Bootloader unkown");
    }

    strcat(data,"\r\n");
    strncat(data, (char *)ext_flash_brt->ui_version,32);
    respond_usr_cmd(USR_CMD_VER, (uint8_t *)data, strlen(data));

    strcat(data2,"\r\nMCU ");
    strncat(data2, MCU_MODEL, strlen(MCU_MODEL));

    if(get_hwid_level()==0xff)
    {
        sprintf(HWIDdata,"\r\nHW ID unknown");
    }else{
        sprintf(HWIDdata,"\r\nHW ID %d",get_hwid_level());
    }
    strncat(data2, HWIDdata, strlen(HWIDdata));

    if(app_get_current_chrg_drv()==BQ25898E)
    {
        sprintf(chargeIC,"\r\nchgIC BQ25898E");
    }else if(app_get_current_chrg_drv()==MP2731)
    {
        sprintf(chargeIC,"\r\ncharge IC MP2731");
    }else{
        sprintf(chargeIC,"\r\ncharge IC unkown");
    }
    strncat(data2, chargeIC, strlen(chargeIC));

//    if(haptic_read_driver_id()==DRV2625)
//    {
//        sprintf(hapticIC,"\r\nhaptic IC DRV2625");
//    }else if(haptic_read_driver_id()==AW86224)
//    {
//        sprintf(hapticIC,"\r\nhaptic IC AW8622X");
//    }else{
//        sprintf(hapticIC,"\r\nhaptic IC unkown");
//    }
//    strncat(data2, hapticIC, strlen(hapticIC));
    LOGD("length %d\r\n", strlen(data));
    respond_usr_cmd(USR_CMD_VER, (uint8_t *)data2, strlen(data2));
}

/*************************************************************************************************
  * @brief    : Parse usr command received from pc
  * @param1   : Command ID
  * @param2   : Pointer to data part of command
  * @return   : None
*************************************************************************************************/
void parse_usr_cmd(uint8_t cmd, uint8_t *pdata, uint16_t len)
{
    switch(cmd){
        case USR_CMD_BUTTON:
            respond_usr_cmd(USR_CMD_BUTTON, NULL, 0);
           /* if(pdata[0] == 0x21){
                post_msg_to_manager(op_b_btn_down);
            }else if(pdata[0] == 0x31){
                post_msg_to_manager(op_b_btn_up);
            }else if(pdata[0] == 0x22){
                post_msg_to_manager(op_b_btn_2s_down);
            }else if(pdata[0] == 0x32){
                post_msg_to_manager(op_b_btn_2s_up);
            }else if(pdata[0] == 0x23){
                post_msg_to_manager(op_b_btn_3s_down);
            }else if(pdata[0] == 0x33){
                post_msg_to_manager(op_b_btn_3s_up);
            }else */
        if(pdata[0] == 0x41){
             post_msg_to_manager(op_btn_short_down);
        }else if(pdata[0] == 0x51){
             post_msg_to_manager(op_btn_short_up);
         }else if(pdata[0] == 0x42){
             post_msg_to_manager(op_btn_short_down);
             vTaskDelay(1);
             post_msg_to_manager(op_btn_1s5_down);
         }else if(pdata[0] == 0x43){
             //post_msg_to_manager(op_btn_oneshort_onelong);
             post_msg_to_manager(op_btn_short_down);
             vTaskDelay(1);
             post_msg_to_manager(op_btn_1s5_down);
             vTaskDelay(1);
             post_msg_to_manager(op_btn_3s_down);
         }else if(pdata[0] == 0x44){
             //post_msg_to_manager(op_btn_oneshort_onelong);
//             post_msg_to_manager(op_btn_short_down);
//             vTaskDelay(1);
//             post_msg_to_manager(op_btn_1s5_down);
//             vTaskDelay(1);
//             post_msg_to_manager(op_btn_3s_down);
//             vTaskDelay(1);
//             post_msg_to_manager(op_btn_5s_down);
         }else if(pdata[0] == 0x52){
             post_msg_to_manager(op_btn_1s5_up);
         }else if(pdata[0] == 0x53){
             post_msg_to_manager(op_btn_3short);
             //post_msg_to_manager(op_btn_3s_up);
         }
         break;
        case USR_CMD_WD_APP:
            comm_send_now(USR_CMD_WD_APP, PC_ADDR, NULL, 0);
            IWDG_reinit(pdata[1]<<8|pdata[0]);
            //hwi_HAL_Delay(pdata[1]<<8|pdata[0]);
            LOGD("watch dog time has been set to %d\r\n",pdata[1]<<8|pdata[0]);
            //LOGD("watch dog time cannot be accurately tested\r\n");
            //LOGD("please try a number closer to 2000 or greater than 2000\r\n");
        break;
        case USR_CMD_REBOOT:
            comm_send_now(USR_CMD_REBOOT, PC_ADDR, NULL, 0);
            power_soft_reset();
            break;
        case USR_CMD_HEAT:
            respond_usr_cmd(USR_CMD_HEAT, NULL, 0);
            switch(pdata[0]){
                case 0x01:
                    post_msg_to_manager(op_normal_heat);
                    break;
                case 0x02:
                    post_msg_to_manager(op_boost_heat);
                    break;
                case 0x03:
                    app_start_quick_heat_test();
                    break;
                case 0x00:
                    post_msg_to_manager(op_stop_heat);
                    break;
                default:
                    break;
            }
            break;
        case USR_CMD_RESET_PWM_PARAMETER:{
            respond_usr_cmd(USR_CMD_RESET_PWM_PARAMETER, NULL, 0);
            flash_record_t* pfrt =get_self_flash_record_from_ram();

            pfrt->pwm_htr_duty_coil1=PWM_DUTY_COIL1;
            pfrt->pwm_htr_duty_coil2=PWM_DUTY_COIL2;

            pfrt->pwm_htr_clock_coil1 = PWM_FREQ_COIL1;
            pfrt->pwm_htr_clock_coil2 = PWM_FREQ_COIL2;
            update_data_flash(USR_DATA,INVALID);
            LOGD("reset htr pwm parameters");
            }
            break;
        case USR_CMD_HTR_PWM_DUTY:{
            respond_usr_cmd(USR_CMD_HTR_PWM_DUTY, NULL, 0);
            flash_record_t* pfrt =get_self_flash_record_from_ram();
            if(pdata[0] == 1)/* coil1 */
            {
                if(pdata[1] <= 100){
                    pfrt->pwm_htr_duty_coil1 = pdata[1];
                    update_data_flash(USR_DATA,INVALID);
                    LOGD("pwm htr duty of coil1 update success :%d\r\n", pdata[1]);
                }else{
                    LOGD("pwm htr duty of coil1 is error :%d\r\n", pdata[1]);
                }
            }
            else if(pdata[0] == 2)/* coil2 */
            {
                if(pdata[1] <= 100){
                    pfrt->pwm_htr_duty_coil2 = pdata[1];
                    update_data_flash(USR_DATA,INVALID);
                    LOGD("pwm htr duty of coil2 update success :%d\r\n", pdata[1]);
                }else{
                    LOGD("pwm htr duty of coil2 is error :%d\r\n", pdata[1]);
                }
            }
            }
            break;
        case USR_CMD_HTR_PWM_CLOCK:{
            respond_usr_cmd(USR_CMD_HTR_PWM_CLOCK, NULL, 0);
            flash_record_t* pfrt =get_self_flash_record_from_ram();
            if(pdata[0] == 1)/* coil1 */
            {
                pfrt->pwm_htr_clock_coil1 = ((pdata[1]<<8)|pdata[2]);
                update_data_flash(USR_DATA,INVALID);
                LOGD("pwm htr clock of coil1 update success :%d\r\n", pfrt->pwm_htr_clock_coil1);
                dev_set_pwm_htr_clock(pwm_htren1,pfrt->pwm_htr_clock_coil1);
//                dev_set_pwm_htr_clock(pwm_htren3,pfrt->pwm_htr_clock_coil1);
            }
            else if(pdata[0] == 2)/* coil2 */
            {
                pfrt->pwm_htr_clock_coil2 = ((pdata[1]<<8)|pdata[2]);
                update_data_flash(USR_DATA,INVALID);
                LOGD("pwm htr clock of coil2 update success :%d\r\n", pfrt->pwm_htr_clock_coil2);
                dev_set_pwm_htr_clock(pwm_htren2,pfrt->pwm_htr_clock_coil2);
//                dev_set_pwm_htr_clock(pwm_htren4,pfrt->pwm_htr_clock_coil2);
            }
            }
            break;
        case USR_CMD_ADC:
            respond_usr_cmd(USR_CMD_ADC, NULL, 0);
            dev_print_all_adc();
            break;
        case USR_CMD_GET_CHARGE:
            //print_charge_context();
            app_SendChargeICParameter();
            break;
        case USR_CMD_READ_HP:
            read_heat_profile_to_pc(pdata[0]);
            break;
        case USR_CMD_WRITE_HP_NAME:{
                respond_usr_cmd(USR_CMD_WRITE_HP_NAME, NULL, 0);
                flash_record_t * pfrt = get_self_flash_record_from_ram();
                if(pdata[0] == 'n'){
                    memcpy(pfrt->n_heat_name, &pdata[1], 32);
                }else if(pdata[0] == 'b'){
                    memcpy(pfrt->b_heat_name, &pdata[1], 32);
                }
                //update_data_flash(USR_DATA,INVALID);
            }
            break;
        case USR_CMD_READ_HP_NAME:{
                flash_record_t * pfrt = get_self_flash_record_from_ram();
                if(pdata[0] == 'n'){
                    pfrt->n_heat_name[31] = '\0';
                    //LOGD("n_heat_name:%s\r\n", pfrt->n_heat_name);
                    respond_usr_cmd(USR_CMD_READ_HP_NAME, pfrt->n_heat_name, 32);
                }else if(pdata[0] == 'b'){
                    pfrt->b_heat_name[31] = '\0';
                    //LOGD("b_heat_name:%s\r\n", pfrt->b_heat_name);
                    respond_usr_cmd(USR_CMD_READ_HP_NAME, pfrt->b_heat_name, 32);
                }
            }
            break;
        case USR_CMD_SHUT_TIME:
            {
                respond_usr_cmd(USR_CMD_SHUT_TIME, NULL, 0);
                uint32_t ms;
                ms = (pdata[0]<<8 |pdata[1])*1000;
                set_auto_shut_down_ms(ms);
            }
            break;
        case USR_CMD_DISPLAY_LED:
            {
                //respond_usr_cmd(USR_CMD_DISPLAY_LED, NULL, 0);
            }
            break;
        case USR_CMD_SET_LED_PATTERN:
            {
                //respond_usr_cmd(USR_CMD_SET_LED_PATTERN, NULL, 0);
            }
            break;
        case USR_CMD_RESET_LED_PATTERN:
                //respond_usr_cmd(USR_CMD_RESET_LED_PATTERN, NULL, 0);
                break;
        case USR_CMD_SHUTDOWN:
            respond_usr_cmd(USR_CMD_SHUTDOWN, NULL, 0);
            LOG_NOW("system shut down ...\r\n");
            power_shutdown_mode();
            break;
        case USR_CMD_SET_HAPTIC_PATTERN:
            {
                respond_usr_cmd(USR_CMD_SET_HAPTIC_PATTERN, NULL, 0);
                app_haptic_set_parameter(pdata[0], &pdata[1], len - 1);
            }
            break;
        case USR_CMD_RESET_HAPTIC:
            {
                respond_usr_cmd(USR_CMD_RESET_HAPTIC, NULL, 0);
                if(0 == pdata[0])//reset intensity
                {
                    //update flash
                    flash_record_t * pfrt = get_self_flash_record_from_ram();
                    pfrt->haptic_volt = HAPTIC_VOLT_DEFAULT;
                    update_data_flash(USR_DATA,INVALID);

                    //update driver register
                    //app_set_haptic_intensity(HP_INTENSITY_3V_LEVEL);
                }
                else if(5 == pdata[0])
                {
                    flash_record_t * pfrt = get_self_flash_record_from_ram();
                    pfrt->haptic_pwm_freq = PWM_FREQ_HAPTIC;
                    update_data_flash(USR_DATA,INVALID);
                }
                else//1~4 correspond to haptic A~D
                {
                    app_haptic_restore_default(pdata[0]);
                }
                break;
            }
        case USR_CMD_DISPLAY_HAPTIC_PATTERN:
            respond_usr_cmd(USR_CMD_DISPLAY_HAPTIC_PATTERN, NULL, 0);
            switch(pdata[0]){
                    case 1:
                        app_haptic_buzz('a');
                        break;
                    case 2:
                        app_haptic_buzz('b');
                        break;
                    case 3:
                        app_haptic_buzz('c');
                        break;
                    case 4:
                        app_haptic_buzz('d');
                        break;
                    default:
                        break;
            }
            break;
        case USR_CMD_CLEAR_RETURN_ERR_FLAG:
        {
            respond_usr_cmd(USR_CMD_CLEAR_RETURN_ERR_FLAG, NULL, 0);
            clear_return_err_flag();
            break;
        }
        case USR_CMD_READ_CURRENT_ERROR:
        {
            uint64_t error_code = get_error_code();
            if(!error_code)
            {
                upload_error((errorCode_e)0xFF);
            }
            for(uint8_t error_pos = 0; error_pos < 64; error_pos++)
            {
                if(error_code & ((uint64_t)1 << error_pos)){
                    upload_error((errorCode_e)error_pos);
                }
            }
            break;
        }
        case USR_CMD_READ_ERROR_STASTIC:
        {
            send_error_stastic();
            break;
        }
        case USR_CMD_READ_LIFECYCLE:
        {
            send_lifecycle();
            break;
        }
        case USR_CMD_READ_100_SESSIONS:
        {
            send_100_sessions();
            break;
        }
        case USR_CMD_DISABLE_CHARGE:
            respond_usr_cmd(USR_CMD_DISABLE_CHARGE, NULL, 0);
            post_msg_to_manager(opt_cmd_disable_charge);
            break;
        case USR_CMD_ENABLE_CHARGE:
            respond_usr_cmd(USR_CMD_ENABLE_CHARGE, NULL, 0);
            post_msg_to_manager(opt_cmd_enable_charge);
            break;
        case USR_CMD_CYCLE_LOG:
            respond_usr_cmd(USR_CMD_CYCLE_LOG, NULL, 0);
            cycle_log = pdata[0];
            break;
        case USR_CMD_HEAT_LOG_16MS:
            respond_usr_cmd(USR_CMD_HEAT_LOG_16MS, NULL, 0);
            heat_log_16ms = pdata[0];
            break;
        case USR_CMD_HAPTIC_ON_OFF:
            respond_usr_cmd(USR_CMD_HAPTIC_ON_OFF, NULL, 0);
            if(pdata[0] == 0){
                app_haptic_enable(1);
                //dev_pwm_set_duty(pwm_haptic, 100);
            }else{
                app_haptic_enable(0);
                //dev_pwm_set_duty(pwm_haptic, 0);
            }
            break;
        case USR_CMD_CHG_IC_LOG:
            respond_usr_cmd(USR_CMD_CHG_IC_LOG, NULL, 0);
            charge_IC_log = pdata[0];
            break;
        case USR_CMD_MODIFY_VALUE:
        {
            respond_usr_cmd(USR_CMD_MODIFY_VALUE, NULL, 0);
            uint16_t value = pdata[2]<<8 | pdata[1];
            switch(pdata[0])
            {
                case BAT_TEMP_E:
                    phy_value.bat_temp = value;
                    phy_value.phy_val_get_pos |= 1<<BAT_TEMP_E;
                    LOGD("set bat_temp to %d\r\n", phy_value.bat_temp);
                    break;
                case BAT_V_E:
                    phy_value.bat_mv = value;
                    phy_value.phy_val_get_pos |= 1<<BAT_V_E;
                    LOGD("set bat_mv to %d\r\n", phy_value.bat_mv);
                    break;
                case ZONE1_TEMP_E:
                    phy_value.zone1_temp = value;
                    phy_value.phy_val_get_pos |= 1<<ZONE1_TEMP_E;
                    LOGD("set zone1_temp to %d\r\n", phy_value.zone1_temp);
                    break;
                case ZONE2_TEMP_E:
                    phy_value.zone2_temp = value;
                    phy_value.phy_val_get_pos |= 1<<ZONE2_TEMP_E;
                    LOGD("set zone2_temp to %d\r\n", phy_value.zone2_temp);
                    break;
                case USB_TEMP_E:
                    phy_value.usb_temp = value;
                    phy_value.phy_val_get_pos |= 1<<USB_TEMP_E;
                    LOGD("set usb_temp to %d\r\n", phy_value.usb_temp);
                    break;
                case I_SENSE_E:
                    phy_value.i_sense_ma = value;
                    phy_value.phy_val_get_pos |= 1<<I_SENSE_E;
                    LOGD("set i_sense_ma to %d\r\n", phy_value.i_sense_ma);
                    break;
                case COIL_TEMP_E:
                    phy_value.coil_temp = value;
                    phy_value.phy_val_get_pos |= 1<<COIL_TEMP_E;
                    LOGD("set coil_temp to %d\r\n", phy_value.coil_temp);
                    break;
                case COIL_JUNC_TEMP_E:
                    phy_value.coil_junc_temp = value;
                    phy_value.phy_val_get_pos |= 1<<COIL_JUNC_TEMP_E;
                    LOGD("set coil_junc_temp to %d\r\n", phy_value.coil_junc_temp);
                    break;
                case PWM_DAC_E:
                    phy_value.pwm_dac = value;
                    phy_value.phy_val_get_pos |= 1<<PWM_DAC_E;
                    LOGD("set pwm_dac to %d\r\n", phy_value.pwm_dac);
                    break;
                case TC1_TEMP_E:
                    phy_value.tc1_temp = value;
                    phy_value.phy_val_get_pos |= 1<<TC1_TEMP_E;
                    LOGD("set tc1_temp to %d\r\n", phy_value.tc1_temp);
                    break;
                case TC2_TEMP_E:
                    phy_value.tc2_temp = value;
                    phy_value.phy_val_get_pos |= 1<<TC2_TEMP_E;
                    LOGD("set tc2_temp to %d\r\n", phy_value.tc2_temp);
                    break;
                case BAT_ID_E:
                    phy_value.bat_id = value;
                    phy_value.phy_val_get_pos |= 1<<BAT_ID_E;
                    LOGD("set bat_id to %d\r\n", phy_value.bat_id);
                    break;
                case GAS_SOC_E:
                    phy_value.gas_soc = value;
                    phy_value.phy_val_get_pos |= 1<<GAS_SOC_E;
                    LOGD("set gas_soc to %d\r\n", phy_value.gas_soc);
                    break;
                case DEL_ALL:
                    memset(&phy_value, 0, sizeof(phy_value));
                    break;
                default:
                    break;
            }
            break;
        }
        case USR_CMD_RECALCULATE_BAT:
            respond_usr_cmd(USR_CMD_RECALCULATE_BAT, NULL, 0);
            app_RecalculateBat();
            break;
        case USR_CMD_SET_HEAT_BATV:
        {
            respond_usr_cmd(USR_CMD_SET_HEAT_BATV, NULL, 0);
            int16_t batv = ((pdata[1] << 8) | pdata[0]);
            if(batv > 4000 || batv <= 3000)
            {
                LOGE("heat threshold batv needs to be <= 4000mv and > 3000mv\r\n");
                break;
            }
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            pfrt->error_parameter.heat_cutoff_volt = batv;
            update_data_flash(USR_DATA,INVALID);
            LOGD("discharge_cutoff_volt is %dmv\r\n", batv);
            break;

        }
        case USR_CMD_ERASE_FREQUENT_DATA_FLASH:
            respond_usr_cmd(USR_CMD_ERASE_FREQUENT_DATA_FLASH, NULL, 0);
            erase_frquent_data_flash(FREQUENT_DATA_FLASH_ALL_PAGE);
            break;
        case USR_CMD_READ_DISCHARGE_CUTOFF_VOLT:
        {
            respond_usr_cmd(USR_CMD_READ_DISCHARGE_CUTOFF_VOLT, NULL, 0);
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            int16_t cutOff_v = pfrt->error_parameter.heat_cutoff_volt;
            LOGD("discharge cut-off volt is %dmv", cutOff_v);
            break;
        }
        case USR_CMD_READ_RECORD_WHEN_ERR:
        {
            respond_usr_cmd(USR_CMD_READ_RECORD_WHEN_ERR, NULL, 0);
            data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
            LOGD("Records when error ocurred:");
            LOGD("No., bat_v, coil_t, bat_t, cold_junc_t, usb_t, tc1_t, tc2_t, i_sense, chg_ic_reg_0B, chg_ic_reg_0C");
            for(uint8_t i = 0; i < RECORD_TIMES_WHEN_ERROR; i++)
            {
                LOGD("-%d- -%d-,-%d-,-%d-,-%d-,-%d-,-%d-,-%d-,-%d-,-0x%X-,-0x%X-",i+1,
                    pDataChangeFreq->records_when_err[i].vbat_adc,pDataChangeFreq->records_when_err[i].coil_t_adc,
                    pDataChangeFreq->records_when_err[i].bat_t_adc,pDataChangeFreq->records_when_err[i].cold_junc_t_adc,
                    pDataChangeFreq->records_when_err[i].usb_t_adc,pDataChangeFreq->records_when_err[i].tc1_adc,
                    pDataChangeFreq->records_when_err[i].tc2_adc,pDataChangeFreq->records_when_err[i].i_sense_adc,
                    pDataChangeFreq->records_when_err[i].CHG_IC_REG_0B,pDataChangeFreq->records_when_err[i].CHG_IC_REG_0C);
            }
            break;
        }
        case USR_CMD_SET_HAPTIC_INTENSITY:
        {
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            respond_usr_cmd(USR_CMD_SET_HAPTIC_INTENSITY, NULL, 0);
            int16_t haptic_volt = ((pdata[0] << 8) | pdata[1]);
            if(haptic_volt > HAPTIC_VOLT_MAX){
                LOGE("haptic volt out of range");
            }
            else
            {
                app_set_haptic_intensity(haptic_volt);
                pfrt->haptic_volt =haptic_volt;
                update_data_flash(USR_DATA,INVALID);
            }
            break;
        }

        case USR_CMD_SHOW_OLED_PATTERN:
        {
            dis_color_msg(pdata[0]);
            respond_usr_cmd(USR_CMD_SHOW_OLED_PATTERN, NULL, 0);
            break;
        }
        case USR_CMD_OLED_BRIGHTNESS:
        {
            dev_oled_reinit(pdata[0],pdata[1]);
            respond_usr_cmd(USR_CMD_OLED_BRIGHTNESS, NULL, 0);
            break;
        }
        case USR_CMD_SET_LED_INTENSITY:
        {
            //respond_usr_cmd(USR_CMD_SET_LED_INTENSITY, NULL, 0);
            break;
        }
        case USR_CMD_RESET_LED_INTENSITY:
        {
            //respond_usr_cmd(USR_CMD_RESET_LED_INTENSITY, NULL, 0);
            break;
        }
        case USR_CMD_GET_HAPTIC_INTENSITY:
        {
            uint16_t haptic_intensity = app_get_haptic_intensity();
            pdata[0] = (haptic_intensity>>8) & 0xff;
            pdata[1] = haptic_intensity & 0xff;
            respond_usr_cmd(USR_CMD_GET_HAPTIC_INTENSITY, pdata, 2);
            break;
        }
        case USR_CMD_GET_HAPTIC_PATTERN:
        {
            flash_record_t* pfrt = get_self_flash_record_from_ram();
            uint8_t buf_len = sizeof(haptic_mode_t) + 1;
            uint8_t * haptic_pattern = (uint8_t*)calloc(buf_len, 1);
            uint8_t pattern_data_len = 0;
            if(haptic_pattern == NULL){
                break;
            }
            //haptic a
            haptic_pattern[0] = 1;
            //cycle_cnt occupies 2 bytes, pattern data occupies (cycle_cnt * sizeof(one_cycle_buzz_t) - 2) bytes
            pattern_data_len = pfrt->haptic_a.cycle_cnt * sizeof(one_cycle_buzz_t);
            memcpy(&haptic_pattern[1], &(pfrt->haptic_a), pattern_data_len);
            respond_usr_cmd(USR_CMD_GET_HAPTIC_PATTERN, haptic_pattern, pattern_data_len + 1);

            //haptic b
            haptic_pattern[0] = 2;
            pattern_data_len = pfrt->haptic_b.cycle_cnt * sizeof(one_cycle_buzz_t);
            memcpy(&haptic_pattern[1], &(pfrt->haptic_b), pattern_data_len);
            respond_usr_cmd(USR_CMD_GET_HAPTIC_PATTERN, haptic_pattern, pattern_data_len + 1);

            //haptic c
            haptic_pattern[0] = 3;
            pattern_data_len = pfrt->haptic_c.cycle_cnt * sizeof(one_cycle_buzz_t);
            memcpy(&haptic_pattern[1], &(pfrt->haptic_c), pattern_data_len);
            respond_usr_cmd(USR_CMD_GET_HAPTIC_PATTERN, haptic_pattern, pattern_data_len + 1);

            //haptic d
            haptic_pattern[0] = 4;
            pattern_data_len = pfrt->haptic_d.cycle_cnt * sizeof(one_cycle_buzz_t);
            memcpy(&haptic_pattern[1], &(pfrt->haptic_d), pattern_data_len);
            respond_usr_cmd(USR_CMD_GET_HAPTIC_PATTERN, haptic_pattern, pattern_data_len + 1);
            free(haptic_pattern);
            break;
        }
        /*case USR_CMD_SET_B2B_BATT_TEMP_LIMIT:
        {
            flash_record_t *frt = get_self_flash_record_from_ram();
            frt->b2b_batt_temp_limit = pdata[0];
            update_data_flash(USR_DATA, INVALID);
            LOGD("b2b_batt_temp_limit is set to %d", frt->b2b_batt_temp_limit);
            respond_usr_cmd(USR_CMD_SET_B2B_BATT_TEMP_LIMIT, NULL, 0);
            break;
        }*/
        case USR_CMD_SET_PRE_SES_BATT_TEMP_LIMIT:
        {
            flash_record_t *frt = get_self_flash_record_from_ram();
            frt->error_parameter.pre_session_temp_limit = pdata[0];
            update_data_flash(USR_DATA, INVALID);
            LOGD("b2b_batt_temp_limit is set to %d", frt->error_parameter.pre_session_temp_limit);
            respond_usr_cmd(USR_CMD_SET_PRE_SES_BATT_TEMP_LIMIT, NULL, 0);
            break;
        }
        case USR_CMD_NO_SHUTDOWN:
        {
            respond_usr_cmd(USR_CMD_NO_SHUTDOWN, NULL, 0);
            switch (pdata[0])
            {
                case 0:
                    LOGD("Shutdown enabled");
                    no_shutdown_flag = 0;
                    break;
                case 1:
                    LOGD("Shutdown disabled");
                    no_shutdown_flag = 1;
                    break;
                default:
                    break;
            }
            break;
        }
        case USR_CMD_SET_SYSTEM_PARAMETER:
        {
            uint8_t data_length = SYSTEM_MAX*2+1;
            if(len == data_length)
            {
                flash_record_t* pfrt = get_self_flash_record_from_ram();
                pfrt->charge_temp_protect = (pdata[2] << 8) | pdata[1];
                pfrt->charge_temp_protect_relesae = (pdata[4] << 8) | pdata[3];
                pfrt->haptic_volt = (pdata[6] << 8) | pdata[5];
                pfrt->haptic_pwm_freq = (pdata[8] << 8) | pdata[7];
                pfrt->eol_session = (pdata[10] << 8) | pdata[9];
                pfrt->step1_session_nums = (pdata[12] << 8) | pdata[11];
                pfrt->step2_session_nums = (pdata[14] << 8) | pdata[13];
                pfrt->step3_session_nums = (pdata[16] << 8) | pdata[15];
                pfrt->step4_session_nums = (pdata[18] << 8) | pdata[17];
                pfrt->slow_chg_isense = (pdata[20] << 8) | pdata[19];
                pfrt->slow_batv_h = (pdata[22] << 8) | pdata[21];
                pfrt->slow_batv_l = (pdata[24] << 8) | pdata[23];
                pfrt->wrong_chg_h_mv = (pdata[26] << 8) | pdata[25];
                pfrt->step1_chg_curr = (pdata[28] << 8) | pdata[27];
                pfrt->step1_chg_volt = (pdata[30] << 8) | pdata[29];
                pfrt->step2_chg_curr = (pdata[32] << 8) | pdata[31];
                pfrt->step2_chg_volt = (pdata[34] << 8) | pdata[33];
                pfrt->step3_chg_curr = (pdata[36] << 8) | pdata[35];
                pfrt->step3_chg_volt = (pdata[38] << 8) | pdata[37];
                pfrt->step4_chg_curr = (pdata[40] << 8) | pdata[39];
                pfrt->step4_chg_volt = (pdata[42] << 8) | pdata[41];
                pfrt->step2_chg_hot_limit = (pdata[44] << 8) | pdata[43];
                pfrt->step2_chg_hot_protect = (pdata[46] << 8) | pdata[45];
                set_system_parameter(pdata[0], &pdata[47], len-47);
                respond_usr_cmd(USR_CMD_SET_SYSTEM_PARAMETER, NULL, 0);
            }
            else
            {
                LOGD("Data length mismatch,stop writing data");
            }
           break;
        }
        case USR_CMD_GET_SYSTEM_PARAMETER:
        {
            fill_system_para_buffer();
            break;
        }
        case USR_CMD_RETORE_DEFAULT_SYSTEM_PARAMETER:
        {
            restore_default_system_parameter();
            respond_usr_cmd(USR_CMD_RETORE_DEFAULT_SYSTEM_PARAMETER, NULL, 0);
            break;
        }
        case USR_CMD_ENTER_SHIPMODE:
        {
            power_ship_mode();
            break;
        }
#ifdef AUTO_TEST
        case USR_CMD_AUTO_TEST:
        {
            respond_usr_cmd(USR_CMD_AUTO_TEST, pdata, 1);
            switch (pdata[0])
            {
                case 1:
                    LOGD("Auto Test enabled");
                    set_auto_test_flag(1);
                    break;
                case 0:
                    LOGD("Auto Test disabled");
                    set_auto_test_flag(0);
                    break;
                default:
                    break;
            }
            break;
        }
#endif
        case USR_CMD_GET_HAPTIC_FREQ:
        {
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            uint16_t haptic_pwm_freq = pfrt->haptic_pwm_freq;
            //LOGD("haptic_pwm_freq = %d",haptic_pwm_freq);
            pdata[0] = (haptic_pwm_freq>>8) & 0xff;
            pdata[1] = haptic_pwm_freq & 0xff;
            respond_usr_cmd(USR_CMD_GET_HAPTIC_FREQ, pdata, 2);
            break;
        }
        case USR_CMD_SET_HAPTIC_FREQ:
        {
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            respond_usr_cmd(USR_CMD_SET_HAPTIC_FREQ, NULL, 0);

            pfrt->haptic_pwm_freq =((pdata[0] << 8) | pdata[1]);
            //LOGD("haptic_pwm_freq = %d",((pdata[0] << 8) | pdata[1]));
            update_data_flash(USR_DATA,INVALID);
            break;
        }
        case USR_CMD_SET_INI_VERSION:
        {
            respond_usr_cmd(USR_CMD_SET_INI_VERSION, NULL, 0);
            flash_record_t* pfrt = get_self_flash_record_from_ram();
            if(len > 32)
            {
                memcpy(&(pfrt->ini_version), pdata, 32);
            }
            else
            {
                memcpy(&(pfrt->ini_version), pdata, len);
            }
            break;
        }
        case USR_CMD_GET_INI_VERSION:
        {
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            respond_usr_cmd(USR_CMD_GET_INI_VERSION, pfrt->ini_version, 32);
            break;
        }
        case USR_CMD_UPDATE_HEAT_PROFILE:
        {
            respond_usr_cmd(USR_CMD_UPDATE_HEAT_PROFILE, NULL, 0);
            app_parse_heat_profile_cmd(pdata, len);
            break;
        }
        case USR_CMD_EXTEND_MODE_GET:
        {
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            pdata[0] = pfrt->extend_mode;
            respond_usr_cmd(USR_CMD_EXTEND_MODE_GET, pdata, 1);
            break;
        }
        case USR_CMD_EXTEND_MODE_SET:
        {
//            respond_usr_cmd(USR_CMD_EXTEND_MODE_SET, NULL, 0);
//            flash_record_t * pfrt = get_self_flash_record_from_ram();
//            pfrt->extend_mode = pdata[0];
//            update_data_flash(USR_DATA,INVALID);
            break;
        }
        case USR_CMD_GAUGE_UPDATE:
        {
            respond_usr_cmd(USR_CMD_GAUGE_UPDATE, NULL, 0);
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            pfrt->gauge_version = 0;
            update_data_flash(USR_DATA,INVALID);
            power_soft_reset();
            break;
        }
        case USR_CMD_READ_SN:
        {
            uint8_t smt_sn[SN_LEN] = {0};
            read_smt_sn(smt_sn);
            uint8_t count = 0;

           for(uint8_t i = 0;i < SN_LEN;i++){
                if(smt_sn[i] == 0xFF)
                {
                    count++;
                }
            }
           if(count == SN_LEN){
               LOGD("Device SN has lost");
           }else{
               LOGD("Device SN is %s", smt_sn);
           }
            respond_usr_cmd(USR_CMD_READ_SN, smt_sn, SN_LEN);
            break;
        }
        default:
            break;
    }

}

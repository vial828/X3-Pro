#include "log.h"
#include "manager.h"
#include "kernel.h"
#include "dev_temperature.h"
#include "error_code.h"
#include "dev_adc.h"
#include "app_charge.h"
#include "app_heat.h"
#include "math.h"
#include "HWI_gpio.h"
#include "dev_bq25898x.h"
#include "mp2731.h"

#define HEATING_OFF_PIN    GPIO_PIN_0
#define HEATING_OFF_PORT   GPIOF

error_check_e check_flag=enable_s;

/*
    variable error_store records which of all the continuous errors have occured
    to prevent repeated alarms
*/
static uint64_t error_store = 0;
int16_t pre_session_temp_limit;
int16_t charge_temp_limit;
int16_t charge_temp_limit_clear;
int16_t bat_cold_charge_temp;
int16_t bat_cold_charge_temp_clear;
int16_t bat_hot_temp;
int16_t bat_hot_temp_clear;
int16_t bat_cold_heat_temp;
int16_t bat_cold_heat_temp_clear;
float heat_cutoff_volt;
int16_t heat_cutoff_volt_soc;
float heat_empty_volt;
int16_t bat_volt_damage_protect;
int16_t tc_zone1_hot;
int16_t tc_zone1_hot_clear;
int16_t tc_zone2_hot;
int16_t tc_zone2_hot_clear;
//int16_t tc_zone1_cold;
//int16_t tc_zone1_cold_clear;
//int16_t tc_zone2_cold;
//int16_t tc_zone2_cold_clear;
float bat_volt_over;
float bat_volt_over_clear;
float discharge_current_over;
float charge_current_over;
int16_t co_junc_hot;
int16_t co_junc_hot_clear;
//int16_t co_junc_cold;
//int16_t co_junc_cold_clear;
float i_sense_damage;
int16_t charge_timeout;
int16_t coil_hot_temp;
int16_t coil_hot_temp_clear;
int16_t usb_hot_temp;
int16_t usb_hot_temp_clear;
//int16_t usb_cold_temp;
//int16_t usb_cold_temp_clear;



/*************************************************************************************************
  * @brief    : Before session errorCode check, read parameter from ini file
  * @return   : none
*************************************************************************************************/
void read_error_parameter(void)
{
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    int32_t all_heat_time = 0;
    int32_t step2_time = pfrt->step2_session_nums*290;
    if(all_heat_time<step2_time)
    {
        charge_temp_limit= pfrt->error_parameter.charge_temp_limit;
    }
    else{
        charge_temp_limit= pfrt->step2_chg_hot_limit;
    }
    pre_session_temp_limit = pfrt->error_parameter.pre_session_temp_limit;
    charge_temp_limit_clear= pfrt->error_parameter.charge_temp_limit_clear;
    bat_cold_charge_temp= pfrt->error_parameter.bat_cold_charge_temp;
    bat_cold_charge_temp_clear= pfrt->error_parameter.bat_cold_charge_temp_clear;
    bat_hot_temp= pfrt->error_parameter.bat_hot_temp;
    bat_hot_temp_clear= pfrt->error_parameter.bat_hot_temp_clear;
    bat_cold_heat_temp= pfrt->error_parameter.bat_cold_heat_temp;
    bat_cold_heat_temp_clear= pfrt->error_parameter.bat_cold_heat_temp_clear;
    heat_cutoff_volt = ((float)pfrt->error_parameter.heat_cutoff_volt)/1000;
    heat_cutoff_volt_soc = pfrt->error_parameter.heat_cutoff_volt_soc;
    heat_empty_volt = ((float)pfrt->error_parameter.heat_empty_volt)/1000;
    bat_volt_damage_protect = pfrt->error_parameter.bat_volt_damage_protect;
    tc_zone1_hot= pfrt->error_parameter.tc_zone1_hot;
    tc_zone1_hot_clear= pfrt->error_parameter.tc_zone1_hot_clear;
    tc_zone2_hot= pfrt->error_parameter.tc_zone2_hot;
    tc_zone2_hot_clear= pfrt->error_parameter.tc_zone2_hot_clear;
//    tc_zone1_cold= pfrt->error_parameter.tc_zone1_cold;
//    tc_zone1_cold_clear= pfrt->error_parameter.tc_zone1_cold_clear;
//    tc_zone2_cold= pfrt->error_parameter.tc_zone2_cold;
//    tc_zone2_cold_clear= pfrt->error_parameter.tc_zone2_cold_clear;
    bat_volt_over= ((float)pfrt->error_parameter.bat_volt_over)/1000;
    bat_volt_over_clear= ((float)pfrt->error_parameter.bat_volt_over_clear)/1000;
    discharge_current_over= ((float)pfrt->error_parameter.discharge_current_over)/1000;
    charge_current_over= ((float)pfrt->error_parameter.charge_current_over)/1000;
    co_junc_hot= pfrt->error_parameter.co_junc_hot;
    co_junc_hot_clear= pfrt->error_parameter.co_junc_hot_clear;
//    co_junc_cold= pfrt->error_parameter.co_junc_cold;
//    co_junc_cold_clear= pfrt->error_parameter.co_junc_cold_clear;
    i_sense_damage= ((float)pfrt->error_parameter.i_sense_damage)/1000;
    charge_timeout = pfrt->error_parameter.charge_timeout;
    coil_hot_temp = pfrt->error_parameter.coil_hot_temp;
    coil_hot_temp_clear = pfrt->error_parameter.coil_hot_temp_clear;
    usb_hot_temp = pfrt->error_parameter.usb_hot_temp;
    usb_hot_temp_clear = pfrt->error_parameter.usb_hot_temp_clear;
//    usb_cold_temp = pfrt->error_parameter.usb_cold_temp;
//    usb_cold_temp_clear = pfrt->error_parameter.usb_cold_temp_clear;
}

void check_paramemter(void)
{
    if(app_get_state() == STATE_CIT){
        return ;
    }
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    uint8_t flag = 0;
    if(pfrt->charge_temp_protect < pfrt->charge_temp_protect_relesae)
    {
        pfrt->charge_temp_protect = BAT_HOT_PROTECT_THRESHOLD;
        pfrt->charge_temp_protect_relesae = BAT_HOT_PROTECT_RELEASE;
        flag = 1;
    }
    if(pfrt->slow_chg_isense>=2500)
    {
        pfrt->slow_chg_isense = SLOW_CHG_DEFAULT_MA;
        flag = 1;
    }
    if(pfrt->slow_batv_h < pfrt->slow_batv_l)
    {
        pfrt->slow_batv_h = SLOW_DEFAULT_H_BATV;
        pfrt->slow_batv_l = SLOW_DEFAULT_L_BATV;
        flag = 1;
    }
    if(charge_temp_limit <= charge_temp_limit_clear)
    {
        pfrt->error_parameter.charge_temp_limit = BAT_HOT_CHARGING;
        pfrt->error_parameter.charge_temp_limit_clear = BAT_HOT_CHARGING_CLEAR;
        charge_temp_limit= pfrt->error_parameter.charge_temp_limit;
        charge_temp_limit_clear= pfrt->error_parameter.charge_temp_limit_clear;
        flag = 1;
    }
    if(bat_hot_temp <= bat_hot_temp_clear)
    {
        pfrt->error_parameter.bat_hot_temp = BAT_HOT_TEMP;
        pfrt->error_parameter.bat_hot_temp_clear = BAT_HOT_TEMP_CLEAR;
        bat_hot_temp= pfrt->error_parameter.bat_hot_temp;
        bat_hot_temp_clear= pfrt->error_parameter.bat_hot_temp_clear;
        flag = 1;
    }
     if(bat_cold_charge_temp >= bat_cold_charge_temp_clear)
    {
        pfrt->error_parameter.bat_cold_charge_temp = BAT_COLD_CHARGE_TEMP;
        pfrt->error_parameter.bat_cold_charge_temp_clear = BAT_COLD_CHARGE_TEMP_CLEAR;
        bat_cold_charge_temp= pfrt->error_parameter.bat_cold_charge_temp;
        bat_cold_charge_temp_clear= pfrt->error_parameter.bat_cold_charge_temp_clear;
        flag = 1;
    }
    if(bat_cold_heat_temp >= bat_cold_heat_temp_clear)
    {
        pfrt->error_parameter.bat_cold_heat_temp = BAT_COLD_HEAT_TEMP;
        pfrt->error_parameter.bat_cold_heat_temp_clear = BAT_COLD_HEAT_TEMP_CLEAR;
        bat_cold_heat_temp= pfrt->error_parameter.bat_cold_heat_temp;
        bat_cold_heat_temp_clear= pfrt->error_parameter.bat_cold_heat_temp_clear;
        flag = 1;
    }
    if(usb_hot_temp <= usb_hot_temp_clear)
    {
        pfrt->error_parameter.usb_hot_temp = USB_HOT_TEMP;
        pfrt->error_parameter.usb_hot_temp_clear = USB_HOT_TEMP_CLEAR;
        usb_hot_temp = pfrt->error_parameter.usb_hot_temp;
        usb_hot_temp_clear = pfrt->error_parameter.usb_hot_temp_clear;
        flag = 1;
    }
    if(co_junc_hot <= co_junc_hot_clear)
    {
        pfrt->error_parameter.co_junc_hot = CO_JUNC_HOT;
        pfrt->error_parameter.co_junc_hot_clear = CO_JUNC_HOT_CLEAR;
        co_junc_hot= pfrt->error_parameter.co_junc_hot;
        co_junc_hot_clear= pfrt->error_parameter.co_junc_hot_clear;
        flag = 1;
    }
//    if(co_junc_cold >= co_junc_cold_clear)
//    {
//        pfrt->error_parameter.co_junc_cold = CO_JUNC_COLD;
//        pfrt->error_parameter.co_junc_cold_clear = CO_JUNC_COLD_CLEAR;
//        co_junc_cold= pfrt->error_parameter.co_junc_cold;
//        co_junc_cold_clear= pfrt->error_parameter.co_junc_cold_clear;
//        flag = 1;
//    }
    if(tc_zone1_hot <= tc_zone1_hot_clear)
    {
        pfrt->error_parameter.tc_zone1_hot = TC_ZONE1_HOT;
        pfrt->error_parameter.tc_zone1_hot_clear = TC_ZONE1_HOT_CLEAR;
        tc_zone1_hot= pfrt->error_parameter.tc_zone1_hot;
        tc_zone1_hot_clear= pfrt->error_parameter.tc_zone1_hot_clear;
        flag = 1;
    }
    if(tc_zone2_hot <= tc_zone2_hot_clear)
    {
        pfrt->error_parameter.tc_zone2_hot = TC_ZONE2_HOT;
        pfrt->error_parameter.tc_zone2_hot_clear = TC_ZONE2_HOT_CLEAR;
        tc_zone2_hot= pfrt->error_parameter.tc_zone2_hot;
        tc_zone2_hot_clear= pfrt->error_parameter.tc_zone2_hot_clear;
        flag = 1;
    }
//    if(tc_zone1_cold >= tc_zone1_cold_clear)
//    {
//        pfrt->error_parameter.tc_zone1_cold = TC_ZONE1_COLD;
//        pfrt->error_parameter.tc_zone1_cold_clear = TC_ZONE1_COLD_CLEAR;
//        tc_zone1_cold= pfrt->error_parameter.tc_zone1_cold;
//        tc_zone1_cold_clear= pfrt->error_parameter.tc_zone1_cold_clear;
//        flag = 1;
//    }
//    if(tc_zone2_cold >= tc_zone2_cold_clear)
//    {
//        pfrt->error_parameter.tc_zone2_cold = TC_ZONE2_COLD;
//        pfrt->error_parameter.tc_zone2_cold_clear = TC_ZONE2_COLD_CLEAR;
//        tc_zone2_cold= pfrt->error_parameter.tc_zone2_cold;
//        tc_zone2_cold_clear= pfrt->error_parameter.tc_zone2_cold_clear;
//        flag = 1;
//    }
    if(coil_hot_temp <= coil_hot_temp_clear)
    {
        pfrt->error_parameter.coil_hot_temp = COIL_HOT_TEMP;
        pfrt->error_parameter.coil_hot_temp_clear = COIL_HOT_TEMP_CLEAR;
        coil_hot_temp= pfrt->error_parameter.coil_hot_temp;
        coil_hot_temp_clear= pfrt->error_parameter.coil_hot_temp_clear;
        flag = 1;
    }
//    if(usb_cold_temp >= usb_cold_temp_clear)
//    {
//        pfrt->error_parameter.usb_cold_temp = USB_COLD_TEMP;
//        pfrt->error_parameter.usb_cold_temp_clear = USB_COLD_TEMP_CLEAR;
//        usb_cold_temp= pfrt->error_parameter.usb_cold_temp;
//        usb_cold_temp_clear= pfrt->error_parameter.usb_cold_temp_clear;
//        flag = 1;
//    }
    if(flag == 1)
    {
        LOGD("ini para error ,restore");
        update_data_flash(USR_DATA,INVALID);
    }
}

/*************************************************************************************************
 * @brief    : check_cic_config_error
 * @return   : null
************************************************************************************************/
void check_cic_config_error(void)
{
    uint8_t chipID = 0;
    uint8_t errorPos = 0xFF;
    boot_record_t *brt = get_boot_record_from_ram();
    if(HWI_OK == Dev_BQ25898x_getDeviceID(&chipID)){//Reg 0x14

    }else if(HWI_OK == Dev_MP2731_getDeviceID(&chipID)){//Reg 0x4B

    }else{
        errorPos=flt_de_cic_config_error;
        write_error_occur_num_to_ram((errorCode_e)errorPos, 1);
        upload_error((errorCode_e)errorPos);

        if (0xFFFFFFFF == brt->error_pos){//no existing continous error
            brt->reset_err = ERR_EXIST;
            brt->error_pos = errorPos;
            update_data_flash(BOOT_RECORD, INVALID);
        }
    }
}

/*************************************************************************************************
  * @brief    : set error code check if need flag
  * @param    : disable_s/enable_s
  * @return   : None
*************************************************************************************************/
void set_error_check_status(error_check_e status)
{
	check_flag=status;
}
/*************************************************************************************************
  * @brief    : get error code check if need flag
  * @param    : None
  * @return   : disable_s/enable_s
*************************************************************************************************/
error_check_e get_error_check_status(void)
{
    return check_flag;
}

/*************************************************************************************************
  * @brief    : Heating off pin(PF0) initialization
  * @return   : void
*************************************************************************************************/
void heating_off_pin_init(void)
{
//    GPIO_InitTypeDef  GPIO_InitStruct;
//    //PF0 heating_off
//    GPIO_InitStruct.Pin       = HEATING_OFF_PIN;
//    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull      = GPIO_NOPULL;
//
//    hwi_GPIO_Init(HEATING_OFF_PORT, &GPIO_InitStruct);
//	hwi_GPIO_Init();

}
void error_eol_check(void)
{
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    //int32_t all_heat_time = 0;
    int32_t all_heat_time = pDataChangeFreq->lifeCycleData[total_heat_time];
    int32_t eol_time = pfrt->eol_session*290;
    if(all_heat_time >= eol_time)
    {
        if(!(error_store & FLT_DE_END_OF_LIFE))
        {
            post_msg_to_manager_with_arg(op_error_occur, flt_de_end_of_life);
            error_store |= FLT_DE_END_OF_LIFE;
        }
    }
}
/*************************************************************************************************
  * @brief    : during charging error check task
  * @return   : void
*************************************************************************************************/
void check_charge_task(void)
{
    uint8_t inputVolFault = 0;
    uint8_t tempOutRangeFault = 0;
    uint8_t chargTimeOutFault = 0;
    uint8_t outputVoltageFault = 0;
    uint8_t ntcFault = 0;
    //float ntcTemp = 0;
    //uint8_t voltageSelfCheckFault = 0;
    //uint8_t currentSelfCheckFault = 0;
    //RstChargeICWdgTimer();
    read_error_parameter();

    if(get_error_check_status()==disable_s){
        return;
    }

    app_CheckChrgFault(&inputVolFault, &tempOutRangeFault, &chargTimeOutFault);
//    if(inputVolFault==0)
//    {
//        if(error_store & FLT_DE_CIC_INPUT_VOLTAGE){
//            post_msg_to_manager_with_arg(op_error_clear, flt_de_cic_input_voltage);
//            error_store &= (~FLT_DE_CIC_INPUT_VOLTAGE);
//        }
//    }

    outputVoltageFault=app_CheckOutputVoltage();
    if(outputVoltageFault==0)
    {
        if(error_store & FLT_DE_CIC_OUTPUT_VOLTAGE){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_cic_output_voltage);
            error_store &= (~FLT_DE_CIC_OUTPUT_VOLTAGE);
        }
    }

    //check if device is in charging state
    if(app_get_state() != STATE_CHARGE){
        return ;
    }

    float bat_temp = dev_get_adc_result()->bat_temp;

    if (bat_temp >= charge_temp_limit)
    {
        if(!(error_store & FLT_DE_BAT_HOT_50_CHARGING)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_hot_50_charging);
            LOGD("batt: %.3f", bat_temp);
            error_store |= FLT_DE_BAT_HOT_50_CHARGING;
        }
    }
    if (bat_temp <= bat_cold_charge_temp)
    {
        if(!(error_store & FLT_DE_BAT_COLD_CHARGE)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_cold_charge);
            LOGD("batt: %.3f", bat_temp);
            error_store |= FLT_DE_BAT_COLD_CHARGE;
        }
    }
    if(dev_get_adc_result()->vbat>=bat_volt_over)
    {
        if(!(error_store & FLT_DE_BAT_VOLTAGE_OVER)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_voltage_over);
            LOGD("batv: %.3f", dev_get_adc_result()->vbat);
            error_store |= FLT_DE_BAT_VOLTAGE_OVER;
        }
    }

    /*ntcTemp = GetNTCTemp();
    LOGD("bat_temp: %.3f, ntcTemp: %.3f\r\n", bat_temp, ntcTemp);

    if (fabs(bat_temp - ntcTemp) > 20)
    {
        post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_temp_selfcheck);
        LOGD("bat_temp: %.3f, ntcTemp: %.3f", bat_temp, ntcTemp);
    }*/


//    if (inputVolFault)
//    {
//        //post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_input_voltage);
//        if(!(error_store & FLT_DE_CIC_INPUT_VOLTAGE)){
//            post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_input_voltage);
//            error_store |= FLT_DE_CIC_INPUT_VOLTAGE;
//        }
//    }

//    if (tempOutRangeFault)
//    {
//        //post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_temp_out_of_range);
//        if(!(error_store & FLT_DE_CIC_TEMP_OUT_OF_RANGE)){
//            post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_temp_out_of_range);
//            error_store |= FLT_DE_CIC_TEMP_OUT_OF_RANGE;
//        }
//    }else if(tempOutRangeFault==0)
//    {
//        if(error_store & FLT_DE_CIC_TEMP_OUT_OF_RANGE){
//            post_msg_to_manager_with_arg(op_error_clear, flt_de_cic_temp_out_of_range);
//            error_store &= (~FLT_DE_CIC_TEMP_OUT_OF_RANGE);
//        }
//    }
//    else if (chargTimeOutFault)
//    {
//        post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_charge_timeout);
//    }
//    else
//    {
        //do nothing
//    }
    /*if(app_CheckWatchdogFault())
    {
        post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_watchdog);
    }*/

    if(outputVoltageFault)
    {
        //post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_output_voltage);
        if(!(error_store & FLT_DE_CIC_OUTPUT_VOLTAGE)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_output_voltage);
            error_store |= FLT_DE_CIC_OUTPUT_VOLTAGE;
        }
    }

//    ntcFault = app_CheckNTCFault();
//    if (1 == ntcFault)
//    {
//        post_msg_to_manager_with_arg(op_error_occur, war_de_cic_cold);
//        LOGD("war_de_cic_cold\r\n");
//    }
//    else if (2 == ntcFault)
//    {
//        post_msg_to_manager_with_arg(op_error_occur, war_de_cic_hot);
//        LOGD("war_de_cic_hot\r\n");
//    }else if(0 == ntcFault)
//    {
//        post_msg_to_manager_with_arg(op_error_clear, war_de_cic_cold);
//        post_msg_to_manager_with_arg(op_error_clear, war_de_cic_hot);
//    }
    /*CheckVoltageSelfCheck(&voltageSelfCheckFault, &currentSelfCheckFault);
    if (voltageSelfCheckFault)
    {
        post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_voltage_selfcheck);
    }
    else if (currentSelfCheckFault)
    {
        post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_current_selfcheck);
    }
    else
    {
        //do nothing
    }*/
}

/*************************************************************************************************
  * @brief    : during heating error check task
  * @return   : void
*************************************************************************************************/
void check_heat_task(void)
{
    if(get_error_check_status()==disable_s){
        return;
    }
    if(!app_get_heat_state()){
        return ;
    }
    app_during_session_errorCode_check();
}

/*************************************************************************************************
  * @brief    : always error check task
  * @return   : void
*************************************************************************************************/
void error_check_task(void)
{
    float bat_v,i_sense,bat_temp,usb_temp,coil_temp,junc_temp,zone1_temp,zone2_temp;
    uint8_t usb_flag;
    adc_context_st * adc_c = dev_get_adc_result();
    read_error_parameter();
    check_paramemter();
    if(get_error_check_status()==disable_s){
        return;
    }

    usb_flag = app_check_usb_plug_status();

    if(adc_c->bat_temp <= bat_cold_heat_temp){
        if(!(error_store & FLT_DE_BAT_COLD_HEAT)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_cold_heat);
            LOGD("batt: %.3f", adc_c->bat_temp);
            error_store |= FLT_DE_BAT_COLD_HEAT;
        }
    }

    if(adc_c->bat_temp >= bat_cold_heat_temp_clear){
        if(error_store & FLT_DE_BAT_COLD_HEAT){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_cold_heat);
            error_store &= (~FLT_DE_BAT_COLD_HEAT);
        }
    }
    if(adc_c->bat_temp >= bat_hot_temp){
        if(app_get_state() != STATE_CHARGE){
            if(!(error_store & FLT_DE_BAT_HOT_55)){
                post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_hot_55);
                LOGD("batt: %.3f", adc_c->bat_temp);
                error_store |= FLT_DE_BAT_HOT_55;
            }
        }
    }
    if(adc_c->bat_temp <= bat_hot_temp_clear){
        if(error_store & FLT_DE_BAT_HOT_55){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_hot_55);
            error_store &= (~FLT_DE_BAT_HOT_55);
        }
    }

    if(adc_c->bat_temp <= charge_temp_limit_clear || usb_flag == NO_USB_PLUG){

        if(error_store & FLT_DE_BAT_HOT_50_CHARGING){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_hot_50_charging);
            error_store &= (~FLT_DE_BAT_HOT_50_CHARGING);
        }
    }
    if (adc_c->bat_temp >= bat_cold_charge_temp_clear || usb_flag == NO_USB_PLUG)
    {
        if(error_store & FLT_DE_BAT_COLD_CHARGE){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_cold_charge);
            LOGD("batt: %.3f", adc_c->bat_temp);
            error_store &= (~FLT_DE_BAT_COLD_CHARGE);
        }
    }

    if(adc_c->zone1_temp>=tc_zone1_hot){
        if(!(error_store & FLT_DE_TC_ZONE1_HOT)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_tc_zone1_hot);
            LOGD("zone1t: %.3f", adc_c->zone1_temp);
            error_store |= FLT_DE_TC_ZONE1_HOT;
        }
    }
    if(adc_c->zone1_temp <= tc_zone1_hot_clear){
        if(error_store & FLT_DE_TC_ZONE1_HOT){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_tc_zone1_hot);
            error_store &= (~FLT_DE_TC_ZONE1_HOT);
        }
    }

    if(adc_c->zone2_temp>=tc_zone2_hot){
        if(!(error_store & FLT_DE_TC_ZONE2_HOT)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_tc_zone2_hot);
            LOGD("zone2t: %.3f", adc_c->zone2_temp);
            error_store |= FLT_DE_TC_ZONE2_HOT;
        }
    }
    if(adc_c->zone2_temp <= tc_zone2_hot_clear){
        if(error_store & FLT_DE_TC_ZONE2_HOT){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_tc_zone2_hot);
            error_store &= (~FLT_DE_TC_ZONE2_HOT);
        }
    }

    if(adc_c->usb_temp>=usb_hot_temp){
        if(!(error_store & FLT_DE_USB_HOT)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_usb_hot);
            LOGD("usbt: %.3f", adc_c->usb_temp);
            error_store |= FLT_DE_USB_HOT;
        }
    }
    if(adc_c->usb_temp<=usb_hot_temp_clear){
        if(error_store & FLT_DE_USB_HOT){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_usb_hot);
            error_store &= (~FLT_DE_USB_HOT);
        }
    }
#ifdef SSCOM
//    if(adc_c->coil_temp<=0){
//        if(!(error_store & FLT_DE_CO_COLD)){
//            post_msg_to_manager_with_arg(op_error_occur, flt_de_co_cold);
//            LOGD("coilt: %.3f", adc_c->coil_temp);
//            error_store |= FLT_DE_CO_COLD;
//        }
//    }
//    if(adc_c->coil_temp > 2){
//        if(error_store & FLT_DE_CO_COLD){
//            post_msg_to_manager_with_arg(op_error_clear, flt_de_co_cold);
//            error_store &= (~FLT_DE_CO_COLD);
//        }
//    }
#endif
//    if(usb_temp<=usb_cold_temp){
//        if(!(error_store & FLT_DE_USB_COLD)){
//            post_msg_to_manager_with_arg(op_error_occur, flt_de_usb_cold);
//            LOGD("usbt: %.3f", usb_temp);
//            error_store |= FLT_DE_USB_COLD;
//        }
//    }
//    if(usb_temp >= usb_cold_temp_clear){
//        if(error_store & FLT_DE_USB_COLD){
//            post_msg_to_manager_with_arg(op_error_clear, flt_de_usb_cold);
//            error_store &= (~FLT_DE_USB_COLD);
//        }
//    }

//    if(zone1_temp<=tc_zone1_cold){
//        if(!(error_store & FLT_DE_TC_ZONE1_COLD)){
//            post_msg_to_manager_with_arg(op_error_occur, flt_de_tc_zone1_cold);
//            LOGD("zone1t: %.3f", zone1_temp);
//            error_store |= FLT_DE_TC_ZONE1_COLD;
//        }
//    }
//    if(zone1_temp >= tc_zone1_cold_clear){
//        if(error_store & FLT_DE_TC_ZONE1_COLD){
//            post_msg_to_manager_with_arg(op_error_clear, flt_de_tc_zone1_cold);
//            error_store &= (~FLT_DE_TC_ZONE1_COLD);
//        }
//    }

//    if(zone2_temp<=tc_zone2_cold){
//        if(!(error_store & FLT_DE_TC_ZONE2_COLD)){
//            post_msg_to_manager_with_arg(op_error_occur, flt_de_tc_zone2_cold);
//            LOGD("zone2t: %.3f", zone2_temp);
//            error_store |= FLT_DE_TC_ZONE2_COLD;
//        }
//    }
//    if(zone2_temp >= tc_zone2_cold_clear){
//        if(error_store & FLT_DE_TC_ZONE2_COLD){
//            post_msg_to_manager_with_arg(op_error_clear, flt_de_tc_zone2_cold);
//            error_store &= (~FLT_DE_TC_ZONE2_COLD);
//        }
//    }

    if(adc_c->coil_temp>=coil_hot_temp){
        if(!(error_store & FLT_DE_CO_HOT_SW)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_co_hot_sw);
            LOGD("coilt: %.3f", adc_c->coil_temp);
            error_store |= FLT_DE_CO_HOT_SW;
        }
    }
    if(adc_c->coil_temp <= coil_hot_temp_clear){
        if(error_store & FLT_DE_CO_HOT_SW){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_co_hot_sw);
            error_store &= (~FLT_DE_CO_HOT_SW);
        }
    }

    if(adc_c->vbat<=bat_volt_over_clear|| usb_flag == NO_USB_PLUG){
        if(error_store & FLT_DE_BAT_VOLTAGE_OVER){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_voltage_over);
            error_store &= (~FLT_DE_BAT_VOLTAGE_OVER);
        }
    }

    /*if(bat_v<2.3f){
        if(!(error_store & FLT_DE_BAT_DAMAGE)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_damage);
            LOGD("batv: %.3f", bat_v);
            error_store |= FLT_DE_BAT_DAMAGE;
        }
    }
    if(bat_v>2.9f){
        if(error_store & FLT_DE_BAT_DAMAGE){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_damage);
            error_store &= (~FLT_DE_BAT_DAMAGE);
        }
    }*/

    if(adc_c->i_sense>discharge_current_over){
        if(!(error_store & FLT_DE_BAT_DISCHARGE_CURRENT_OVER)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_discharge_current_over);
            LOGD("i_sense: %.3f", adc_c->i_sense);
            error_store |= FLT_DE_BAT_DISCHARGE_CURRENT_OVER;
        }
    }
    if(adc_c->i_sense<1.0f){
        if(error_store & FLT_DE_BAT_DISCHARGE_CURRENT_OVER){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_discharge_current_over);
            error_store &= (~FLT_DE_BAT_DISCHARGE_CURRENT_OVER);
        }
    }

    if(adc_c->i_sense<((-1)*charge_current_over)){
        if(!app_get_heat_state()){
            if(!(error_store & FLT_DE_BAT_CHARGE_CURRENT_OVER)){
                post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_charge_current_over);
                LOGD("i_sense: %.3f", adc_c->i_sense);
                error_store |= FLT_DE_BAT_CHARGE_CURRENT_OVER;
            }
        }
    }
    if(adc_c->i_sense>((-1)*(charge_current_over-1))){
        if(error_store & FLT_DE_BAT_CHARGE_CURRENT_OVER){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_bat_charge_current_over);
            error_store &= (~FLT_DE_BAT_CHARGE_CURRENT_OVER);
        }
    }

    if(adc_c->cold_junc_temp>=co_junc_hot){
        if(!(error_store & FLT_DE_CO_JUNC_HOT)){
            post_msg_to_manager_with_arg(op_error_occur, flt_de_co_junc_hot);
            LOGD("junct: %.3f", adc_c->cold_junc_temp);
            error_store |= FLT_DE_CO_JUNC_HOT;
        }
    }
    if(adc_c->cold_junc_temp<=co_junc_hot_clear){
        if(error_store & FLT_DE_CO_JUNC_HOT){
            post_msg_to_manager_with_arg(op_error_clear, flt_de_co_junc_hot);
            error_store &= (~FLT_DE_CO_JUNC_HOT);
        }
    }

//    if(junc_temp<=co_junc_cold){
//        if(!(error_store & WAR_DE_CO_JUNC_COLD)){
//            post_msg_to_manager_with_arg(op_error_occur, war_de_co_junc_cold);
//            LOGD("junct: %.3f", junc_temp);
//            error_store |= WAR_DE_CO_JUNC_COLD;
//        }
//    }
//    if(junc_temp >= co_junc_cold_clear){
//        if(error_store & WAR_DE_CO_JUNC_COLD){
//            post_msg_to_manager_with_arg(op_error_clear, war_de_co_junc_cold);
//            error_store &= (~WAR_DE_CO_JUNC_COLD);
//        }
//    }

//    if(HAL_GPIO_ReadPin(HEATING_OFF_PORT, HEATING_OFF_PIN) == GPIO_PIN_RESET)
//  if(hwi_GPIO_ReadPin(HEATING_OFF_E) == HWI_PIN_RESET)
//    {
//        if(!(error_store & FLT_DE_HW_CO_I_TC1_TC2_ERROR)){
//            post_msg_to_manager_with_arg(op_error_occur, flt_de_hw_co_i_tc1_tc2_error);
//            error_store |= FLT_DE_HW_CO_I_TC1_TC2_ERROR;
//            LOGD("coilt: %.3f, i_sense: %.3f, TC1: %d, TC2: %d", coil_temp, i_sense,
//                dev_get_tc_temp(1), dev_get_tc_temp(2));
//        }
//    }
//    else
//    {
//        if(error_store & FLT_DE_HW_CO_I_TC1_TC2_ERROR){
//            post_msg_to_manager_with_arg(op_error_clear, flt_de_hw_co_i_tc1_tc2_error);
//            error_store &= (~FLT_DE_HW_CO_I_TC1_TC2_ERROR);
//        }
//    }
    if(error_store!= 0 && app_get_heat_state() == 1)
    {
        app_stop_heat();
    }
    app_auto_clear_b2b_error();
}

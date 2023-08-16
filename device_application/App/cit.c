#include "kernel.h"
#include "log.h"
#include "comm.h"
#include "app_button.h"
#include "cit.h"
#include "manager.h"
#include "app_haptic.h"
#include "power.h"
#include "app_heat.h"
#include "string.h"
#include "self_flash.h"
#include "dev_adc.h"    
#include "dev_temperature.h"
#include "dev_pwm_ll.h"
#include "app_heat.h"
#include "version.h"
#include "m24c64.h"
#include "i2c.h"
#include "app_charge.h"
#include "app_charge.h"
#include "error_code.h"
#include <string.h>
#include <stdio.h>
#include "usr_cmd.h"
#include "dev_gauge_bq27z561r2.h"
#include "math.h"
#include "batTimer.h"
#include "dev_cap_aw9320x_application.h"
#include "app_oled_display.h"
#include "app_oled_UI.h"
#include "dev_oled_driver.h"

#define CIT_CMD_INOUT_CITMODE       0x70
#define CIT_CMD_SENSOR_TEST         0x71
#define CIT_CMD_FFRST_TEST          0x72
#define CIT_CMD_TC_TEMP_TEST        0x73
#define CIT_CMD_HAPITC_TEST         0x74
#define CIT_CMD_SLEEP_TEST          0x75
#define CIT_CMD_OLED_TEST           0x76
#define CIT_CMD_START_BUTTON_TEST   0x77
#define CIT_CMD_GET_VERSION_TEST    0x78
#define CIT_CMD_SMT_SN_TEST         0x79
#define CIT_CMD_MASTER_SN_TEST      0x7A
#define CIT_CMD_HEAT_TEST           0x7B
#define CIT_CMD_GET_VBAT_TEST       0x7C
#define CIT_CMD_GET_ISENSE_TEST     0x7D
#define CIT_CMD_RESET_TEST          0x7E
#define CIT_CMD_UPDATE_SESSION_NUM  0x7F
#define CIT_CMD_GET_VBUS_TEST       0x80
#define CIT_CMD_TC_HEAT_TEST        0x81
#define CIT_CMD_GET_LED_ID_TEST     0x82
#define CIT_CMD_GET_HAPTIC_ID_TEST  0x83
#define CIT_CMD_GET_TSPCT_TEST      0x84
#define CIT_CMD_RESTORE_FACTORY     0x85
//#define CIT_CMD_MODE_BUTTON_TEST    0x85
#define CIT_CMD_P_HEAT_TEST         0x86   //Finished product test
#define CIT_CMD_GET_HWID_TEST       0x87
#define CIT_CMD_CHARGE_TEST         0x88
//#define CIT_CMD_READ_SESSIONS       0x89
//#define CIT_CMD_RESET_SESSIONS      0X8A
#define CIT_CMD_GET_MCU             0x89  //get the MCU type
#define CIT_CMD_GET_BAT             0x8A  //get the battery type
#define CIT_CMD_GET_FULE_GAUGE_ID   0x8B  //get soc ID
#define CIT_CMD_GET_HEAT_COUNT      0x8C  //get heat completed count
#define CIT_CMD_GET_HALL_DOOR_TEST  0x8D
#define CIT_CMD_GET_HALL_MODE_TEST  0x8E
#define CIT_CMD_GET_CAP_ID          0x8F
#define CIT_CMD_SHIPMODE          0x90
#define CIT_CMD_GET_FLASH_VR_TEST   0x91
#define CIT_CMD_OLED_BRIGHT     0x92
#define CIT_CMD_GET_GAUGE_SOC   0x93
#define CIT_CMD_GET_OLED_ID     0x94

#ifndef SSCOM
#define CIT_CMD_FACTORY_CHARGE_TEST     0x9A
#endif

#define FF_RST_PIN    GPIO_PIN_13
#define FF_RST_PORT   GPIOB

#define MAX_HEAT  21

#define IC_OUT(x)  (x-1)

#define LED8   IC_OUT(1)
#define LED1   IC_OUT(8)

#define LED6   IC_OUT(6)
#define LED9   IC_OUT(9)
#define LED11  IC_OUT(11)
#define LED12  IC_OUT(12)
#define LED14  IC_OUT(14)
#define LED15  IC_OUT(15)
#define LED17  IC_OUT(17)

extern uint8_t flash_rx_buffer[76140];
static uint8_t g_cit_mode = 0;
//static timer_t * get_isense_htr1_timer;
//static timer_t * get_isense_htr2_timer;
static ptimer_t get_isense_htr1_timer;
static ptimer_t get_isense_htr2_timer;
//static timer_t * get_ff_rest1_timer;
//static timer_t * get_ff_rest2_timer;
//static timer_t * tc_test_record_timer;
//static timer_t * p_heat_test_timer;
static ptimer_t tc_test_record_timer;
static ptimer_t p_heat_test_timer;

static uint8_t status_value[10]={0};
static uint8_t heat_number=0;
static uint8_t heat_time = 0;
static uint8_t heat_count = 0;
static uint8_t heat_data[MAX_HEAT];
static uint8_t p_heat_time = 0;
#ifndef SSCOM
//static timer_t * p_charge_timer;
static ptimer_t p_charge_timer;
static uint8_t bat_v_limit=0;
#endif
void clear_cit_mode_flag(void)
{
    g_cit_mode = 0;
}
void set_cit_mode_flag(void)
{
    g_cit_mode = 1;
}

void writ_sn(uint8_t *pdata)
{
    flash_record_t * frt = get_self_flash_record_from_ram();
    memcpy(frt->sn, pdata, SN_LEN);
    update_data_flash(USR_DATA,INVALID);
}

void read_sn(uint8_t *pdata)
{
    flash_record_t * frt = get_self_flash_record_from_ram();
    memcpy(pdata, frt->sn, SN_LEN);
}

void writ_smt_sn(uint8_t *pdata)
{
    flash_record_t * frt = get_self_flash_record_from_ram();
    memcpy(frt->smt_sn, pdata, SN_LEN);
    update_data_flash(USR_DATA,INVALID);
}

void read_smt_sn(uint8_t *pdata)
{
    flash_record_t * frt = get_self_flash_record_from_ram();
    memcpy(pdata, frt->smt_sn, SN_LEN);
}

void cit_get_hwid_test()
{
    uint8_t return_data[3];
    uint16_t HWIDvolt;
    
    HWIDvolt = read_hw_id();
    return_data[2]=get_hwid_level();
    
    return_data[0]=(uint8_t) ((HWIDvolt&0xff00)>>8);
    return_data[1]= (uint8_t) (HWIDvolt&0x00ff);
    
    
    comm_send(CIT_CMD_GET_HWID_TEST, PC_ADDR, return_data, 3);
}

void cit_get_vbat_test()
{
    uint8_t adc_data[2] = {0};
    uint16_t adc_raw[1] = {0};
    
    adc_raw[0]=(uint16_t)(dev_get_vbat_volt()*1000);
    adc_data[0]=(uint8_t) ((adc_raw[0]&0xff00)>>8);
    adc_data[1]= (uint8_t) (adc_raw[0]&0x00ff);
    
    comm_send(CIT_CMD_GET_VBAT_TEST, PC_ADDR, adc_data, 2);
}

//static timer_t * check_unit_timer;
static ptimer_t check_unit_timer;
static int16_t check_cnt=0;
static uint8_t coil1_heat_state=0;
static uint8_t coil2_heat_state=0;
static uint8_t coil1_start_state=0;
static uint8_t coil2_start_state=0;

static void start_coil1_heat(void)
{
    if(coil1_start_state==0)
    {
        app_heat2_enable(0);
        app_heat1_enable(1);
        //start_hlh();
        coil1_start_state=1;
        coil2_start_state=0;
    }
}
static void start_coil2_heat(void)
{
    if(coil2_start_state==0)
    {
        app_heat1_enable(0);
        app_heat2_enable(1);
        //start_hlh();
        coil2_start_state=1;
        coil1_start_state=0;
    }
}

static void stop_coil1_heat(void)
{
    app_heat1_enable(0);
    coil1_start_state=0;
}
static void stop_coil2_heat(void)
{
    app_heat2_enable(0);
    coil2_start_state=0;
}

static void heat_stop()
{
    app_heat1_enable(0);
    app_heat2_enable(0);
    check_cnt=0;
    coil1_start_state=0;
    coil2_start_state=0;
    coil1_heat_state=0;
    coil2_heat_state=0;
    //TIMER_SAFE_DELETE(check_unit_timer);
    if(check_unit_timer){
        if(bat_timer_delete(check_unit_timer, portMAX_DELAY)==pdPASS){
            check_unit_timer = NULL;
        }
    }
}


static void check_one_cnt(const ptimer_t tm)
{
    uint8_t pdata[1] = {0}; 
    int16_t zone1_temp=(int16_t)(dev_get_cold_junc_temp() + dev_get_tc_temp(1));
    int16_t zone2_temp=(int16_t)(dev_get_cold_junc_temp() + dev_get_tc_temp(2));
    
    if(check_cnt%2==0)
    {
        if(zone1_temp<250)
        {
            start_coil1_heat();
        }else{
            stop_coil1_heat();
            coil1_heat_state=1;
        }
    
    }else{
        if(zone2_temp<250)
        {
            start_coil2_heat();
        }else{
            stop_coil2_heat();
            coil2_heat_state=1;
        }
    }
    
    check_cnt++;
    
    if(coil1_heat_state==1 && coil2_heat_state==1)
    {
        pdata[0]=0;
        comm_send(CIT_CMD_HEAT_TEST, PC_ADDR, pdata, 1);
        heat_stop();
    }
        
    if(check_cnt==20*1000/16)
    {
        heat_stop();
       
        pdata[0]=1;
        comm_send(CIT_CMD_HEAT_TEST, PC_ADDR, pdata, 1);
    }
}
void start_heat_test()
{
    //TIMER_SAFE_RESET(check_unit_timer, 16, TIMER_OPT_PERIOD, check_one_cnt, NULL);
    check_unit_timer = bat_timer_reset_ext(check_unit_timer, "check_unit_timer", 16, TIMER_OPT_PERIOD, check_one_cnt);
    bat_timer_start(check_unit_timer, portMAX_DELAY);
}
void cit_heat_test(uint8_t *pdata)
{
    if(pdata[0] == 0){
        //start heat
        //app_start_heat();
        start_heat_test();
    }else if(pdata[0] == 1){
        //stop heat
        app_stop_heat();
        comm_send(CIT_CMD_HEAT_TEST, PC_ADDR, pdata, 1);
    }
    
}

void cit_master_sn_test(uint16_t len,uint8_t *pdata)
{
    uint8_t data[14] = {0};
    if(len == 0){
        read_sn(data);
        comm_send(CIT_CMD_MASTER_SN_TEST, PC_ADDR, data, SN_LEN);
    }else{
        writ_sn(pdata);
        comm_send(CIT_CMD_MASTER_SN_TEST, PC_ADDR, data, 0);
    }
}
void save_sn_and_smtsn_data(uint8_t *data1,uint8_t *data2)
{
    read_sn(data1);
    read_smt_sn(data2);
}
void write_sn_and_smtsn_data(uint8_t *data1,uint8_t *data2)
{
    writ_sn(data1);
    writ_smt_sn(data2);
}
void cit_smt_sn_test(uint16_t len,uint8_t *pdata)
{
    uint8_t data[14] = {0};
    if(len == 0){
        read_smt_sn(data);
        comm_send(CIT_CMD_SMT_SN_TEST, PC_ADDR, data, SN_LEN);
    }else{
        writ_smt_sn(pdata);
        comm_send(CIT_CMD_SMT_SN_TEST, PC_ADDR, data, 0);
    }
}

void cit_get_version_test()
{
    char data[64] = {0};
    boot_record_t * brt = get_boot_record_from_ram();
    memset(data, 0, 64);   
    if(!strncmp((char *)brt->bootloader_version, BL_VERSION, strlen(BL_VERSION)))
    {
        //LOGD("boot:%s\n\r", (char *)frt->bootloader_version);
        snprintf(data,64, "%s %s", DEVICE_APP_VER, (char *)brt->bootloader_version);
    }else{
        //LOGD("no bootloader ver\n\r");
        snprintf(data,64, "%s %s", DEVICE_APP_VER, "Device bootloader unkown");
    }
        //LOGD("app:%s\r\n", data);
    comm_send(CIT_CMD_GET_VERSION_TEST, PC_ADDR, (uint8_t *)data, 64);

}

static void cit_get_flash_version_test(void)
{
    ext_flash_record_t * ext_flash_brt = get_ext_flash_record_from_ram();
    get_flash_version();
    comm_send(CIT_CMD_GET_FLASH_VR_TEST, PC_ADDR, (uint8_t *)ext_flash_brt,32);
}

void cit_sleep_test()
{
    power_shutdown_mode();
}

void cit_tc_temp_test(uint8_t *pdata)
{
    uint8_t adc_data[2] = {0};
    uint16_t adc_raw[1] = {0};
    if(pdata[0] == 0){
        adc_raw[0]=(uint16_t)dev_get_tc_temp(1);;

    }else if(pdata[0] == 1){
        adc_raw[0]=(uint16_t)dev_get_tc_temp(2);;
    }
    
    adc_data[0]=(uint8_t) ((adc_raw[0]&0xff00)>>8);
    adc_data[1]= (uint8_t) (adc_raw[0]&0x00ff);
    comm_send(CIT_CMD_TC_TEMP_TEST, PC_ADDR, adc_data, 2);
}

static void get_isense_htr2(const ptimer_t tm)
{
    float i_sense=0.0;
//    uint8_t adc_data[4];
    uint16_t adc_raw[2];
    
    i_sense=dev_get_i_sense();
    //LOGD("----i_sense[%f]-----\n\r",i_sense);
    
    if(i_sense>=0.6f)
    {
        status_value[5]=0x0;
    }else{
        status_value[5]=0x1;
    }
    
    adc_raw[0]=(uint16_t)(fabs(dev_get_i_sense())*1000);
    status_value[6]=(uint8_t) ((adc_raw[0]&0xff00)>>8);
    status_value[7]= (uint8_t) (adc_raw[0]&0x00ff);
    
    adc_raw[1]=0;//dev_get_adc(I_BAT);
    status_value[8]=(uint8_t) ((adc_raw[1]&0xff00)>>8);
    status_value[9]= (uint8_t) (adc_raw[1]&0x00ff);
    
    app_heat2_enable(0);
    
    comm_send(CIT_CMD_FFRST_TEST, PC_ADDR, status_value, 10);
    //TIMER_SAFE_DELETE(get_isense_htr2_timer);
    if(get_isense_htr2_timer){
        if(bat_timer_delete(get_isense_htr2_timer, portMAX_DELAY)==pdPASS){
            get_isense_htr2_timer = NULL;
        }
    }
}

static void get_isense_htr1(const ptimer_t tm)
{
    float i_sense=0.0;
//    uint8_t adc_data[4];
    uint16_t adc_raw[2];
    
    i_sense=dev_get_i_sense();
    //LOGD("----i_sense[%f]-----\n\r",i_sense);
    
    if(i_sense>=0.6f)
    {
        status_value[0]=0x0;
    }else{
        status_value[0]=0x1;
    }
    
    adc_raw[0]=(uint16_t)(fabs(dev_get_i_sense())*1000);
    status_value[1]=(uint8_t) ((adc_raw[0]&0xff00)>>8);
    status_value[2]= (uint8_t) (adc_raw[0]&0x00ff);
    
    adc_raw[1]=0;//dev_get_adc(I_BAT);
    status_value[3]=(uint8_t) ((adc_raw[1]&0xff00)>>8);
    status_value[4]= (uint8_t) (adc_raw[1]&0x00ff);
    
    app_heat1_enable(0);
    app_heat2_enable(1);
    //comm_send(CIT_CMD_FFRST_TEST, PC_ADDR, status_value, 1);
    //TIMER_SAFE_DELETE(get_ff_rest1_timer);
    //TIMER_SAFE_RESET(get_isense_htr2_timer, 16, TIMER_OPT_ONESHOT, get_isense_htr2, NULL);
    get_isense_htr2_timer = bat_timer_reset_ext(get_isense_htr2_timer, "get_isense_htr2_timer", 16, TIMER_OPT_ONESHOT, get_isense_htr2);
    bat_timer_start(get_isense_htr2_timer, portMAX_DELAY);

    //TIMER_SAFE_DELETE(get_isense_htr1_timer);
    if(get_isense_htr1_timer){
        if(bat_timer_delete(get_isense_htr1_timer, portMAX_DELAY)==pdPASS){
            get_isense_htr1_timer = NULL;
        }
    }
}

void cit_ff_rst_test()
{
    
    //dev_pwm_set_duty(pwm_dac, 30);
    app_heat1_enable(1); 
    //TIMER_SAFE_RESET(get_isense_htr1_timer, 50, TIMER_OPT_ONESHOT, get_isense_htr1, NULL);
    get_isense_htr1_timer = bat_timer_reset_ext(get_isense_htr1_timer, "get_isense_htr1_timer", 50, TIMER_OPT_ONESHOT, get_isense_htr1);
    bat_timer_start(get_isense_htr1_timer, portMAX_DELAY);
}

void cit_get_sensor_ad_test()
{
    
    uint8_t adc_data[18]={0};
    uint16_t adc_raw[9];
    uint8_t i=0;

    adc_raw[0]=(uint16_t)(dev_get_vbat_volt()*1000);
    adc_raw[1]=(uint16_t)dev_get_coil_temp();
    adc_raw[2]=(uint16_t)dev_get_bat_temp();
    adc_raw[3]=(uint16_t)dev_get_bat_temp_gauge();
    adc_raw[4]=(uint16_t)dev_get_cold_junc_temp();
    adc_raw[5]=(uint16_t)dev_get_usb_temp();
    adc_raw[6]=(uint16_t)(fabs(dev_get_i_sense())*1000);
    adc_raw[7]=(uint16_t)dev_get_tc_temp(1);
    adc_raw[8]=(uint16_t)dev_get_tc_temp(2);
    for(i=0;i<9;i++){
        //adc_raw[i]=dev_get_adc((adc_ch_e)i);

        adc_data[i*2]=(uint8_t) ((adc_raw[i]&0xff00)>>8);
        adc_data[i*2+1]= (uint8_t) (adc_raw[i]&0x00ff);
    }

    comm_send(CIT_CMD_SENSOR_TEST, PC_ADDR, adc_data, 18);
}

void cit_get_isense_test()
{
    uint16_t adc_raw;
    uint8_t adc_data[2]={0};

    adc_raw=(uint16_t)(fabs(dev_get_i_sense())*1000);
    adc_data[0]=(uint8_t) ((adc_raw&0xff00)>>8);
    adc_data[1]= (uint8_t) (adc_raw&0x00ff);
    
    comm_send(CIT_CMD_GET_ISENSE_TEST, PC_ADDR, adc_data, 2);
}

uint16_t temp_value=0;
void cit_reset_test(uint8_t *pdata)
{
    if(pdata[0] == 0){
        temp_value=pdata[1];

    }else if(pdata[0] == 1){
        pdata[1]=temp_value;
    }
    
    comm_send(CIT_CMD_RESET_TEST, PC_ADDR, pdata, 2);
}

static void cit_get_led_id_test()
{
    uint8_t pdata=0;
    //pdata=lp5521_read_r_current_for_cit();
    comm_send(CIT_CMD_GET_LED_ID_TEST, PC_ADDR, &pdata, 1);
}

#if 0
static void cit_get_haptic_id_test()
{
    uint8_t pdata=0;
    uint8_t chipID = 0;
    if(HWI_OK == DRV262x_getDeviceID(&chipID)){
        if(chipID == DRV2625_CHIPID)
        {
            LOGD("haptic driver is drv2625");
            pdata = 1;
        }
        else{
            LOGE("haptic driver is drv262x but not drv2625");
        }
    }else{
        //Reg 0x00
        if(aw86224_get_chipID(&chipID) == HWI_OK)
        {
            if(chipID == AW8622X_CHIP_ID){
                LOGD("haptic driver is aw8622x");
                pdata = 2;
            }else{
                LOGE("haptic driver is awinic ic but not aw8622x");
            }
        }
        else
        {
            LOGE("i2C communication error");
        }
    }
    comm_send(CIT_CMD_GET_HAPTIC_ID_TEST, PC_ADDR, &pdata, 1);
}
#endif

void cit_get_vbus_test()
{
    uint16_t adc_raw;
    uint8_t adc_data[2]={0};
    
    adc_raw=app_GetChargeICParameter();
    adc_data[0]=(uint8_t) ((adc_raw&0xff00)>>8);
    adc_data[1]= (uint8_t) (adc_raw&0x00ff);
    
    comm_send(CIT_CMD_GET_VBUS_TEST, PC_ADDR, adc_data, 2);
}

static void get_heatcnt_return(const ptimer_t tm)
{
    int16_t zoneTemp=0;

    if(heat_number == 1){
        zoneTemp=(int16_t)dev_get_zone_temp(1);
        }else{
            zoneTemp=(int16_t)dev_get_zone_temp(2);
        }
    heat_count++;
    heat_data[heat_count*2+1] = (uint8_t)(zoneTemp>>8);
    heat_data[heat_count*2+2] = (uint8_t)(zoneTemp&0x00ff);
    //LOG_NOW("zone temp=0x%04x\r\n", zoneTemp);
    if(heat_count >= heat_time){
        app_heat1_enable(0);
        app_heat2_enable(0);
        comm_send(CIT_CMD_TC_HEAT_TEST, PC_ADDR, heat_data, MAX_HEAT);
        //TIMER_SAFE_DELETE(tc_test_record_timer);
        if(tc_test_record_timer){
            if(bat_timer_delete(tc_test_record_timer, portMAX_DELAY)==pdPASS){
                tc_test_record_timer = NULL;
        }
    }

    }
}

static void get_p_heatcnt_return(const ptimer_t tm)
{
    uint16_t heatCnt=0;
    int16_t zone1Temp=0;
    int16_t zone2Temp=0;
    uint8_t adc_data[5]={0};

    //TIMER_SAFE_DELETE(p_heat_test_timer);
    if(p_heat_test_timer){
        if(bat_timer_delete(p_heat_test_timer, portMAX_DELAY)==pdPASS){
            p_heat_test_timer = NULL;
        }
    }
    
    heatCnt=app_get_heat_cnt();
    zone1Temp=(int16_t)dev_get_zone_temp(1);
    zone2Temp=(int16_t)dev_get_zone_temp(2);

    adc_data[1] = (uint8_t) ((zone1Temp&0xff00)>>8);
    adc_data[2] = (uint8_t) (zone1Temp&0x00ff);

    adc_data[3] = (uint8_t) ((zone2Temp&0xff00)>>8);
    adc_data[4] = (uint8_t) (zone2Temp&0x00ff);

    if((heatCnt*16)>=(heat_time-1)*1000){
        adc_data[0]=0;
    }else{
        adc_data[0]=1;
    }

    comm_send(CIT_CMD_P_HEAT_TEST, PC_ADDR, adc_data, 5);
}

void cit_tc_heat_test(uint8_t *pdata)
{
    //app_start_tc_heat_test(pdata[0],pdata[1]);
      int16_t zone1Temp=0;
      int16_t zone2Temp=0;
      uint8_t temp = pdata[1];
      heat_number=pdata[0];
      heat_time = pdata[2];

      memset(heat_data, 0, MAX_HEAT);
      heat_count = 0;
      zone1Temp = dev_get_zone_temp(1);
      zone2Temp = dev_get_zone_temp(2);

      if(heat_time > ((MAX_HEAT-1)/2-1)){ //check heat time
          heat_data[0] = 0x02; //fail
          comm_send(CIT_CMD_TC_HEAT_TEST, PC_ADDR, heat_data, MAX_HEAT);
          return;
      }
    if(heat_number==1)
    {
        if(zone1Temp>temp)
        {
            heat_data[0] = 0x01; //fail
            heat_data[1] = (uint8_t)(zone1Temp>>8);
            heat_data[2] = (uint8_t)(zone1Temp&0x00ff);
            heat_data[3] = (uint8_t)(zone2Temp>>8);
            heat_data[4] = (uint8_t)(zone2Temp&0x00ff);
            comm_send(CIT_CMD_TC_HEAT_TEST, PC_ADDR, heat_data, 5);
            return;
        }
    }else if(heat_number==2)
    {
        if(zone2Temp>temp)
        {
            heat_data[0] = 0x01; //fail
            heat_data[1] = (uint8_t)(zone1Temp>>8);
            heat_data[2] = (uint8_t)(zone1Temp&0x00ff);
            heat_data[3] = (uint8_t)(zone2Temp>>8);
            heat_data[4] = (uint8_t)(zone2Temp&0x00ff);
            comm_send(CIT_CMD_TC_HEAT_TEST, PC_ADDR, heat_data, 5);
            return;
        }
    }
    /*if(zone1Temp>temp || zone2Temp>temp)
    {
        heat_data[0] = 0x01; //fail
        heat_data[1] = (uint8_t)(zone1Temp>>8);
        heat_data[2] = (uint8_t)(zone1Temp&0x00ff);
        heat_data[3] = (uint8_t)(zone2Temp>>8);
        heat_data[4] = (uint8_t)(zone2Temp&0x00ff);
        comm_send(CIT_CMD_TC_HEAT_TEST, PC_ADDR, heat_data, MAX_HEAT);
        return;
    }*/
        app_get_duty(); /*set to default pwm frq and duty*/
    if(heat_number==1)
    {
        heat_data[0] = 0x00; //success
        heat_data[1] = (uint8_t)(zone1Temp>>8);
        heat_data[2] = (uint8_t)(zone1Temp&0x00ff);
        app_heat1_enable(1);
        app_heat2_enable(0);
    }else if(heat_number==2)
    {
        heat_data[0] = 0x00; //success
        heat_data[1] = (uint8_t)(zone2Temp>>8);
        heat_data[2] = (uint8_t)(zone2Temp&0x00ff);
        app_heat2_enable(1);
        app_heat1_enable(0);
    }
    //TIMER_SAFE_RESET(tc_test_record_timer, 1*1000, TIMER_OPT_PERIOD, get_heatcnt_return, NULL);
    tc_test_record_timer = bat_timer_reset_ext(tc_test_record_timer, "tc_test_record_timer", 1*1000, TIMER_OPT_PERIOD, get_heatcnt_return);
    bat_timer_start(tc_test_record_timer, portMAX_DELAY);
}

void cit_p_heat_test(uint8_t *pdata)
{
    app_start_tc_heat_test(pdata[0],pdata[1]);
    p_heat_time = pdata[1];
        //TIMER_SAFE_RESET(p_heat_test_timer, p_heat_time*1000, TIMER_OPT_ONESHOT, get_p_heatcnt_return, NULL);
    p_heat_test_timer = bat_timer_reset_ext(p_heat_test_timer, "p_heat_test_timer", p_heat_time*1000, TIMER_OPT_ONESHOT, get_p_heatcnt_return);
    bat_timer_start(p_heat_test_timer, portMAX_DELAY);
}

void cit_get_tspct_test()
{
    uint8_t pVal = 0;

    app_GetTSPCT(&pVal);
    comm_send(CIT_CMD_GET_TSPCT_TEST, PC_ADDR, &pVal, 1);
}

void cit_charge_test(uint8_t *pdata)
{
    if(pdata[0]==0)
    {
        //post_msg_to_manager(opt_cmd_disable_charge);
        app_charge_enable(0);
        
    }else if(pdata[0]==1)
    {
        //post_msg_to_manager(opt_cmd_enable_charge);
        app_charge_enable(1);
    }
    comm_send(CIT_CMD_CHARGE_TEST, PC_ADDR, pdata, 1);
}
#ifndef SSCOM
static void charge_enable_function(const ptimer_t tm)
{
    uint8_t bat_v =(uint8_t)(dev_get_vbat_volt()*10);
    //LOGD("--%d----%d-",bat_v,bat_v_limit);
    if(bat_v>=bat_v_limit)
    {
        app_charge_enable(0);
        //TIMER_SAFE_DELETE(p_charge_timer);
        if(p_charge_timer){
            if(bat_timer_delete(p_charge_timer, portMAX_DELAY)==pdPASS){
                p_charge_timer = NULL;
            }
        }
    }
}
void cit_factory_charge_test(uint8_t *pdata)
{
    float bat_v = dev_get_vbat_volt();
    uint8_t data[2];
    data[1]=(uint8_t)(bat_v*10);
    //LOGD("--%d----%d-",data[1],pdata[0]);
    if(pdata[0]>42 || pdata[0]<33)
    {
        data[0]=1;
    }else if(pdata[0]<=data[1])
    {
        data[0]=2;
    }else{
        data[0]=0;
        bat_v_limit=pdata[0];
        app_charge_enable(1);
        //TIMER_SAFE_RESET(p_charge_timer, 10000, TIMER_OPT_PERIOD, charge_enable_function, NULL);
        p_charge_timer = bat_timer_reset_ext(p_charge_timer, "p_charge_timer", 10000, TIMER_OPT_PERIOD, charge_enable_function);
        bat_timer_start(p_charge_timer, portMAX_DELAY);
    }
    comm_send(CIT_CMD_FACTORY_CHARGE_TEST, PC_ADDR, data, 2);
}
#endif
/*void cit_get_records_sessions_data()
{
    data_change_frequent_t* pDataChangeFreq = NULL;
    pDataChangeFreq = get_data_change_frequent_from_ram();

    char data[120];
    char COMPLETED[40];
    char ABORTED[40];
    char B2B[40];
    memset(data, 0, 120);
    sprintf(COMPLETED,"Number of completed sessions:%d\r\n",pDataChangeFreq->session_data.completed_sessions);
    strncat(data, COMPLETED, strlen(COMPLETED));
    sprintf(ABORTED,"Number of aborted sessions:%d\r\n",pDataChangeFreq->session_data.aborted_sessions);
    strncat(data, ABORTED, strlen(ABORTED));
    sprintf(B2B,"Number of back to back sessions:%d\r\n",pDataChangeFreq->session_data.b2b_sessions);
    strncat(data, B2B, strlen(B2B));
    comm_send(CIT_CMD_READ_SESSIONS, PC_ADDR, (uint8_t *)data ,strlen(data));
}

void cit_reset_records_session_data()
{
    data_change_frequent_t* pDataChangeFreq = NULL;
    pDataChangeFreq = get_data_change_frequent_from_ram();
    pDataChangeFreq->session_data.aborted_sessions = 0;
    pDataChangeFreq->session_data.completed_sessions = 0;
    pDataChangeFreq->session_data.b2b_sessions = 0;

    char data[120];
    char COMPLETED[40];
    char ABORTED[40];
    char B2B[40];
    memset(data, 0, 120);
    sprintf(COMPLETED,"Number of completed sessions:%d\r\n",pDataChangeFreq->session_data.completed_sessions);
    strncat(data, COMPLETED, strlen(COMPLETED));
    sprintf(ABORTED,"Number of aborted sessions:%d\r\n",pDataChangeFreq->session_data.aborted_sessions);
    strncat(data, ABORTED, strlen(ABORTED));
    sprintf(B2B,"Number of back to back sessions:%d\r\n",pDataChangeFreq->session_data.b2b_sessions);
    strncat(data, B2B, strlen(B2B));

    comm_send(CIT_CMD_RESET_SESSIONS, PC_ADDR ,(uint8_t *)data ,strlen(data));
}*/
typedef enum
{
    STM32Gxx = 0,
    GD32W515xx,
    CY8C6xx5,
}mcu_type_e;
/* This function is used for TOP test, to tell the tester the MCU_TYPE of this project. */
void cit_get_mcu_test(void)
{
    uint8_t u8McuType;
    
    u8McuType = (uint8_t)GD32W515xx; //pease modify this MCU_type following your project
    
    comm_send(CIT_CMD_GET_MCU, PC_ADDR, &u8McuType, 1);
}
/* This function is used for TOP test, to tell the tester the BATTERY_TYPE of this project. */
void cit_get_bat_test(void)
{
    uint8_t u8BatType;
    
    u8BatType = (uint8_t)app_read_bat_id();
    
    comm_send(CIT_CMD_GET_BAT, PC_ADDR, &u8BatType, 1);
}
/* This function is used for TOP test, to tell the tester the soc type of this project. */
void cit_get_fuel_gauge_id(void)
{
    uint8_t ret = 0;
    ret = Dev_BQ27Z561R2_ReadID();

    comm_send(CIT_CMD_GET_FULE_GAUGE_ID, PC_ADDR, &ret, 1);

}
/* This function is used for TOP test, to tell the tester the capacitance type of this project. */
void cit_cap_test(void)
{
    uint8_t ret = 0;
    ret = dev_aw93205_id_get();
    comm_send(CIT_CMD_GET_CAP_ID, PC_ADDR, &ret, 1);
}

/* This function is used for TOP test, to tell the tester the the heated completed count of this project. */
void cit_get_heat_completed_count_test(void)
{
    uint32_t *u32RetData = NULL;
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    u32RetData = &(pDataChangeFreq->session_data.completed_sessions);
    comm_send(CIT_CMD_GET_HEAT_COUNT, PC_ADDR, (uint8_t *)u32RetData, 4);
}

/* This function is used for TOP test, to tell the tester the soc from gauge. */
void cit_get_gauge_soc(void)
{
    uint8_t soc;
#ifdef ENABLE_FUEL_GAUGE
    soc = app_get_bat_left();
#else
    soc = 0
#endif
    comm_send(CIT_CMD_GET_GAUGE_SOC, PC_ADDR, &soc, 1);
}
void cit_get_oled_id(void)
{
    uint8_t OLED_ID;
    if (HWI_PIN_RESET == hwi_GPIO_ReadPin(OLED_ID_E))
    {
        OLED_ID = OLED_ID_1;
    }
    else
    {
        OLED_ID = OLED_ID_2;
    }
    comm_send(CIT_CMD_GET_OLED_ID, PC_ADDR, &OLED_ID, 1);
}
void ckeck_reset_state(uint8_t *state)
{
    uint8_t data1[14];
    uint8_t data2[14];
    uint8_t i;
    uint8_t count = 0;
    read_sn(data1);
    read_smt_sn(data2);
    for(i = 0;i < 14;i++){
        if(data1[i] == 0xFF && data2[i] == 0xFF)
        {
            count++;
        }
    }
    if(count == 14)
    {
        state[0] = 0;
    }
    else
    {
        state[0] = 1;
    }
}

void oled_control(uint8_t *pdata)
{
    uint8_t oled_data[1];

    if(pdata[0] == OLED_WHITE)//white
    {
        //dev_oled_reinit(pdata[1],pdata[2]);
        //vTaskDelay(10);
        dis_color_msg(OLED_WHITE);
    }
    else if(pdata[0] == OLED_BLACK)//black
    {
        //dev_oled_reinit(pdata[1],pdata[2]);
        //vTaskDelay(10);
        dis_color_msg(OLED_BLACK);
    }
    else if(pdata[0] == OLED_RED)//red
    {
        //dev_oled_reinit(pdata[1],pdata[2]);
        //vTaskDelay(10);
        dis_color_msg(OLED_RED);
    }
    else if(pdata[0] == OLED_GREEN)//green
    {
        //dev_oled_reinit(pdata[1],pdata[2]);
        //vTaskDelay(10);
        dis_color_msg(OLED_GREEN);
    }
    else if(pdata[0] == OLED_BLUE)//blue
    {
        //dev_oled_reinit(pdata[1],pdata[2]);
        //vTaskDelay(10);
        dis_color_msg(OLED_BLUE);
    }else
    {
        //dev_oled_reinit(pdata[1],pdata[2]);
        //vTaskDelay(10);
        dis_color_msg(OLED_WHITE);
    }
    oled_data[0]=(uint8_t)pdata[0];
    //oled_data[1]=(uint8_t)pdata[1];
    //oled_data[2]=(uint8_t)pdata[2];
    comm_send(CIT_CMD_OLED_TEST, PC_ADDR, oled_data, 1);
}

void cit_reset_to_factory(void)
{
    uint8_t state[1];
    uint8_t sn_data[14];
    uint8_t smt_sn_data[14];
    uint8_t pdata[29];
    save_sn_and_smtsn_data(sn_data,smt_sn_data);
    erase_data_flash();
    get_self_flash_record();
    ckeck_reset_state(state);
    write_sn_and_smtsn_data(sn_data,smt_sn_data);

    pdata[0] = state[0];
    for(uint8_t i=0; i<SN_LEN; i++)
    {
        pdata[i+1]= sn_data[i];
        pdata[i+15]= smt_sn_data[i];
    }
    comm_send(CIT_CMD_RESTORE_FACTORY, PC_ADDR, pdata, 29);
}

void parse_cit_cmd(uint8_t cmd, uint8_t *pdata, uint16_t len)
{
    uint8_t data[32];

    if(cmd == CIT_CMD_INOUT_CITMODE){
        comm_send(CIT_CMD_INOUT_CITMODE, PC_ADDR, pdata, 1);
        if(pdata[0]){
            post_msg_to_manager(op_entry_cit);
            set_error_check_status(disable_s);
        }else{
            post_msg_to_manager(op_exit_cit);
            set_error_check_status(enable_s);
        }
    }
    post_msg_to_manager(op_update_cit_timer);  //reset cit timer

    if(g_cit_mode == 1)
    {
        switch(cmd){
            case CIT_CMD_OLED_TEST: //oled control
                oled_control(pdata);
                break;
            case CIT_CMD_START_BUTTON_TEST:
                if(app_read_button() == btn_down)
                {
                    data[0] = 1;
                }else{
                    data[0] = 0;
                }
                comm_send(CIT_CMD_START_BUTTON_TEST, PC_ADDR, data, 1);
                break;
//           case CIT_CMD_MODE_BUTTON_TEST:
//              if(app_read_button(mode_btn) == btn_down)
//              {
//                  data[0] = 1;
//              }else{
//                  data[0] = 0;
//              }
//              comm_send(CIT_CMD_MODE_BUTTON_TEST, PC_ADDR, data, 1);
//                break;
            case CIT_CMD_HAPITC_TEST:
                if(pdata[0] == 0){
                    app_haptic_enable(1);
                    //dev_pwm_set_duty(pwm_haptic, 100);
                }else{
                    app_haptic_enable(0);
                    //dev_pwm_set_duty(pwm_haptic, 0);
                }
                comm_send(CIT_CMD_HAPITC_TEST, PC_ADDR, pdata, 1);
                break;

            case CIT_CMD_SENSOR_TEST:
                cit_get_sensor_ad_test();
                break;
            
            case CIT_CMD_FFRST_TEST:
                cit_ff_rst_test();
                break;
            
            case CIT_CMD_TC_TEMP_TEST:
                cit_tc_temp_test(pdata);
                break;
            
            case CIT_CMD_SLEEP_TEST:
                cit_sleep_test();
                break;
            
            case CIT_CMD_GET_VERSION_TEST:
                cit_get_version_test();
                break;
            
            case CIT_CMD_SMT_SN_TEST:
                cit_smt_sn_test(len,pdata);
                break;
            
            case CIT_CMD_MASTER_SN_TEST:
                cit_master_sn_test(len,pdata);
                break;
            
            case CIT_CMD_HEAT_TEST:
                cit_heat_test(pdata);
                break;
            
            case CIT_CMD_GET_VBAT_TEST:
                cit_get_vbat_test();
                break;
            
            case CIT_CMD_GET_ISENSE_TEST:
                cit_get_isense_test();
                break;
            
            case CIT_CMD_RESET_TEST:
                cit_reset_test(pdata);
                break;
            
            case CIT_CMD_UPDATE_SESSION_NUM:
                update_data_flash(DATA_CHANGE_FREQUENT,NONE_SESSION_DATA);
                comm_send(CIT_CMD_UPDATE_SESSION_NUM, PC_ADDR, pdata, 1);
                break;
            
            case CIT_CMD_GET_VBUS_TEST:
                cit_get_vbus_test();
                break;
            case CIT_CMD_TC_HEAT_TEST:
                cit_tc_heat_test(pdata);
                break;
            case CIT_CMD_P_HEAT_TEST:
                cit_p_heat_test(pdata);
                break;
            case CIT_CMD_GET_LED_ID_TEST:
                cit_get_led_id_test();
                break;
            case CIT_CMD_GET_HAPTIC_ID_TEST:
                //cit_get_haptic_id_test();
                break;
            case CIT_CMD_GET_TSPCT_TEST:
                cit_get_tspct_test();
                break;
            case CIT_CMD_GET_HWID_TEST:
                cit_get_hwid_test();
                break;
            case CIT_CMD_CHARGE_TEST:
                cit_charge_test(pdata);
                break;
            case CIT_CMD_GET_MCU:
                cit_get_mcu_test();
                break;
            case CIT_CMD_GET_BAT:
                cit_get_bat_test();
                break;
            #ifndef SSCOM
            case CIT_CMD_FACTORY_CHARGE_TEST:
                cit_factory_charge_test(pdata);
                break;
            #endif
            case CIT_CMD_GET_FULE_GAUGE_ID:
                cit_get_fuel_gauge_id();
                break;
            case CIT_CMD_GET_HEAT_COUNT:
                cit_get_heat_completed_count_test();
                break;
            case CIT_CMD_GET_HALL_DOOR_TEST:
                if(hwi_GPIO_ReadPin(HALL_INT_DOOR_E) == 0)
                {
                    data[0] = 1;
                }else{
                    data[0] = 0;
                }
                comm_send(CIT_CMD_GET_HALL_DOOR_TEST, PC_ADDR, data, 1);
                break;
            case CIT_CMD_GET_HALL_MODE_TEST:
                if(hwi_GPIO_ReadPin(HALL_INT_MODE_E) == 0)
                {
                    data[0] = 1;
                }else{
                    data[0] = 0;
                }
                comm_send(CIT_CMD_GET_HALL_MODE_TEST, PC_ADDR, data, 1);
                break;
             case CIT_CMD_GET_CAP_ID:
                cit_cap_test();
                break;
             case CIT_CMD_RESTORE_FACTORY:
                cit_reset_to_factory();
                break;
             case CIT_CMD_SHIPMODE:
                data[0] = 0;
                comm_send(CIT_CMD_SHIPMODE, PC_ADDR, data, 1);
                power_ship_mode();
                break;
             case CIT_CMD_GET_FLASH_VR_TEST:
                cit_get_flash_version_test();
                break;
            case CIT_CMD_OLED_BRIGHT:
                data[0] = 0;
                dev_oled_reinit(pdata[0],pdata[1]);
                comm_send(CIT_CMD_OLED_BRIGHT, PC_ADDR, data, 1);
                break;
            case CIT_CMD_GET_GAUGE_SOC:
                cit_get_gauge_soc();
                break;
             case CIT_CMD_GET_OLED_ID:
                cit_get_oled_id();
                break;
            //case CIT_CMD_READ_SESSIONS:
                //cit_get_records_sessions_data();
                //break;
            //case CIT_CMD_RESET_SESSIONS:
                //cit_reset_records_session_data();	
                //break;
            default:
                break;
        }
    }
}

#include <string.h>
#include "stratos_defs.h"
#include "log.h"
#include "message.h"
#include "manager.h"
#include "app_button.h"
#include "app_haptic.h"
#include "power.h"
#include "cit.h"
#include "app_charge.h"
#include "app_heat.h"
#include "dev_temperature.h"
#include "dev_adc.h"
#include "self_flash.h"
#include "error_code_led_output.h"
#include "kernel.h"
#include "usr_cmd.h"
#include "error_code.h"
#include "dev_bq25898x.h"
#include "mp2731.h"
#include "rtc.h"
#include "HWI_gpio.h"
#include "comm.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "batTimer.h"
#include "app_oled_UI.h"
#include "app_oled_display.h"

#define HEAT_TIME_OVER      15*60*1000    //todo: change the time
#define AUTO_SHUT_DOWM_MS   2*60*1000   // 2minutes
//#define AUTO_SHUT_DOWM_MS   10*1000    //todo change the time
#define EXIT_CIT_TIME      50*60*1000  //50 minutes
#define BATTARY_LV_DISP_DURATION    6*1000 //6s

typedef struct {
    ptimer_t pshutdown_timer;
    ptimer_t printf_timer;
    ptimer_t pexit_cit_timer;
    ptimer_t record_when_err_timer;
    ptimer_t comm_lock_timer;
//    ptimer_t enable_sw_33_timer;
//    ptimer_t batt_lv_disp_timer;
    ptimer_t pauto_test_timer;
//    ptimer_t poled_lock_timer;
    uint8_t bat_left;
    uint8_t log_mask;
    uint8_t hall_door_status;
    uint8_t heating_button_flag;
    uint32_t shut_down_ms;
    uint32_t charge_start_time;
    uint32_t charge_stop_time;
    uint32_t heat_start_time;
    uint8_t errorPos;
    uint8_t oled_lock_flag;
    uint64_t error_code;
    uint8_t error_exist_due_to_record;
    uint8_t auto_test_flag;
}manager_context_st;

typedef struct{
    uint8_t fuel;
    float coldt;
    float bat_temp;
    float usb_temp;
    float coil_temp;
    float coil_temp_b;
    float junc_temp;
    int16_t tc1_temp;
    int16_t tc2_temp;
    float vbus_v;
    float vbat_v;
    float i_sense;
    uint8_t bat_left;
}idle_log_t;

typedef struct{
    uint32_t ticks;
    uint8_t item;
    //heat
    float ses_time;
    int16_t sp1;
    int16_t pv1;
    int16_t sp2;
    int16_t pv2;
    uint8_t chan;
    float batv;
    float bata;
    uint16_t CJR;
    uint16_t session_J;
    float usbt;
    float batt;
    float coilt;
    float coldt;
 //   float coilbt;
    float zone1t;
    float zone2t;
    //charge
    uint8_t chg_state;
    uint8_t partab;
    float usbv;
    float bat_J;
    uint8_t fuel;
    //idle
    float tc1_temp;
    float tc2_temp;
}cycle_log_t;

static uint16_t g_state;
static const char g_state_name[STATE_MAX][32] = {
    {"STATE_BOOT"},
    {"STATE_IDLE"},
    {"STATE_NORMAL_HEAT"},
    {"STATE_BOOST_HEAT"},
    {"STATE_CHARGE"},
    {"STATE_CIT"},
    {"STATE_ERROR"}
};
static QueueHandle_t mngrQueueHandle = NULL;

static manager_context_st mc;
static idle_log_t ilog;
static cycle_log_t cycle_log_s;
static heat_session_log_t heat_session_log_s;
static uint8_t chg_timeout_flag ;
//uint8_t button_timeout_flag = 0;
extern uint8_t g_session_extend_flag;
//extern timer_t * g_error_timer;
//extern timer_t * err_exist_timer;
extern ptimer_t g_error_timer;
extern ptimer_t err_exist_timer;
/*************************************************************************************************
  * @brief    : create a message queue for manager
  * @param1   : msg id
  * @return   : none
*************************************************************************************************/
uint8_t create_mngr_msg_queue(void)
{
   mngrQueueHandle = xQueueCreate(Max_QueueMsg_Size, sizeof(msg_st));
   if(mngrQueueHandle){
      return 1;
   }else{
      return 0;
   }
}
/*************************************************************************************************
  * @brief    : post a message to manager msg queue
  * @param1   : msg id
  * @return   : none
*************************************************************************************************/
void post_msg_to_manager(opcode_e opcode)
{
    msg_st msg;
    data_change_frequent_t* pDataChangeFreq = NULL;
    msg.opcode = opcode;
    if(mngrQueueHandle == 0)
    {
        LOGD("No mngrQueueHandle");
        return;
    }
    LOGD("post2mngr %s", get_opcode_name(opcode));
    xQueueSend(mngrQueueHandle, (void *)&msg, portMAX_DELAY);

    /*record button event*/
    if (op_btn_short_down == opcode)
    {
        pDataChangeFreq = get_data_change_frequent_from_ram();
        pDataChangeFreq->lifeCycleData[button_presses]++;
    }
}

/*************************************************************************************************
  * @brief    : post a message to manager msg queue
  * @param1   : msg id
  * @param1   : argument
  * @return   : none
*************************************************************************************************/
void post_msg_to_manager_with_arg(opcode_e opcode, uint32_t arg)
{
    msg_st msg;
    msg.opcode = opcode;
    msg.value = arg;
    data_change_frequent_t* pDataChangeFreq = NULL;

    if(mngrQueueHandle == 0)
    {
        LOGD("No mngrQueueHandle");
        return;
    }
    xQueueSend(mngrQueueHandle, (void *)&msg, portMAX_DELAY);
    LOGD("post2mngr %s:%d", get_opcode_name(opcode), arg);

    /*record hall toggle*/
    if (op_hall_door_mode == opcode)
    {
        pDataChangeFreq = get_data_change_frequent_from_ram();
        pDataChangeFreq->lifeCycleData[hall_toggle]++;
        //LOGD("hall_toggle = %d \r\n", pDataChangeFreq->lifeCycleData[hall_toggle]);
    }
}

/*common function*/
/*************************************************************************************************
  * @brief    : clear the oled out of lock state
  * @param1   : lock_reason
  * @return   : none
*************************************************************************************************/
uint8_t get_oled_lock_flag(uint8_t lock_reason)
{
    return (mc.oled_lock_flag & lock_reason);
}

/*************************************************************************************************
  * @brief    : set the oled into lock state
  * @param1   : lock_reason
  * @return   : none
*************************************************************************************************/
void set_oled_lock_flag(uint8_t lock_reason)
{
    if(lock_reason & OLED_CHG_LOCK)
    {
        app_chg_entry_suspend(CHG_DISPLAY_SUSPEND);
    }
    else  //no CHG_LOCK now
    {     //but still CHG_LOCK before this UI starting
        if(get_oled_lock_flag(OLED_CHG_LOCK)) 
        {
            app_chg_try_to_exit_suspend(CHG_DISPLAY_SUSPEND);
        }
    }
    mc.oled_lock_flag = lock_reason;
}

/*************************************************************************************************
  * @brief    : clear_all_oled_lock
  * @return   : none
*************************************************************************************************/
void clear_all_oled_lock(void)
{
    if(get_oled_lock_flag(OLED_CHG_LOCK))
    {
        app_chg_try_to_exit_suspend(CHG_DISPLAY_SUSPEND);
    }
    mc.oled_lock_flag = 0;
    LOGD("clear oled lock");
}

/*************************************************************************************************
  * @brief    : clear the oled out of lock state
  * @param1   : lock_reason
  * @return   : none
*************************************************************************************************/
void clear_oled_lock_flag(uint8_t lock_reason)
{
    if(lock_reason & OLED_CHG_LOCK)
    {
        app_chg_try_to_exit_suspend(CHG_DISPLAY_SUSPEND);
    }
    mc.oled_lock_flag =  mc.oled_lock_flag & (~lock_reason);
}

/*************************************************************************************************
  * @brief    : timer callback
  * @return   : none
*************************************************************************************************/
static void call_back_auto_shut_down(const ptimer_t tm)
{
    //todo: shutdown some devices
    //TIMER_SAFE_DELETE(mc.pshutdown_timer);
    stop_auto_shutdown_timer();
    LOG_NOW("system shut down ...\r\n");
    power_shutdown_mode();
}


///*************************************************************************************************
//  * @brief    : timer callback
//  * @return   : none
//*************************************************************************************************/
//static void call_back_exit_oled_lock(const ptimer_t tm)
//{
//    if(get_oled_lock_flag(OLED_CHG_LOCK))
//    {
//        app_chg_try_to_exit_suspend(CHG_DISPLAY_SUSPEND);
//    }
//    mc.oled_lock_flag = 0;
//    LOGD("exit oled LOCK ...\r\n");
//}

///*************************************************************************************************
//  * @brief    : timer callback
//  * @return   : none
//*************************************************************************************************/
//static void call_back_exit_heating_lock(const ptimer_t tm)
//{
//    mc.oled_lock_flag = 0;
//    app_chg_try_to_exit_suspend(CHG_HEAT_SUSPEND);
//    LOGD("exit heating LOCK ...\r\n");
//}

#ifdef AUTO_TEST
/*************************************************************************************************
  * @brief    : timer callback
  * @return   : none
*************************************************************************************************/
static void call_back_auto_test(const ptimer_t tm)
{
    //todo: start the Heating for the AutoTest 
    //as the voltage > 3.68 and batt < 46
    if(cycle_log_s.batv*1000 > PRE_HEAT_CUTOFF_VOLT)
    {
       if(cycle_log_s.batt < PRE_SESSION_TEMP_LIMIT)
       {
           post_msg_to_manager(op_normal_heat);
           if(mc.pauto_test_timer){
               if(bat_timer_delete(mc.pauto_test_timer, portMAX_DELAY)==pdPASS){
                   mc.pauto_test_timer = NULL;
               }
           }
           LOGD("Start auto test success ...\r\n");
       }
       else
       {
            LOGD("Start heating after BatT cool ...\r\n");
       }
    }
    else
    {
        LOGD("Start charging after BatT cool ...\r\n");
    }

}
#endif

/*************************************************************************************************
  * @brief    : send idle log byte
  * @param1   : log byte Pointer
  * @param2   : log byte lenght
  * @return   : none
*************************************************************************************************/
static void send_idle_log_bytes(uint8_t *pdata, uint16_t len)
{
    comm_send(IDLE_LOG, PC_ADDR, pdata, len);
}

/*************************************************************************************************
  * @brief    : Send cycle log to pc
  * @param1   : Pointer to log contents
  * @param2   : Log size
  * @return   : None
*************************************************************************************************/
void send_cycle_log_bytes(uint8_t *pdata, uint16_t len)
{
    comm_send(LOG_POA1, PC_ADDR, pdata, len);
}

/*************************************************************************************************
  * @brief    : Timer call back function to send heating log
  * @param1   : Pointer to timer structure
  * @param2   : Pointer to parameter
  * @return   : None
*************************************************************************************************/
static void call_back_printf_heat_logs(const ptimer_t tm)
{
    heat_chan_t* chan_data = app_get_chan_value();
    cycle_log_s.ticks = GetTick();
    cycle_log_s.item = HEAT_LOG;
    cycle_log_s.ses_time = app_get_heat_cnt()*HEAT_ONE_CNT_TIME/1000.0;
    cycle_log_s.sp1 = app_get_sp_value(1);
    cycle_log_s.pv1 = app_get_pv_value(1);
    cycle_log_s.sp2 = app_get_sp_value(2);
    cycle_log_s.pv2 = app_get_pv_value(2);
    cycle_log_s.chan = chan_data->g_chan;
    cycle_log_s.batv = dev_get_adc_result()->vbat;
    cycle_log_s.bata = chan_data->log_i;
    cycle_log_s.CJR = (uint16_t)(app_get_heat_CJR()*1000);
    cycle_log_s.session_J = app_get_totalJ_value();
    cycle_log_s.usbt = dev_get_adc_result()->usb_temp;
    cycle_log_s.batt = dev_get_adc_result()->bat_temp;
    cycle_log_s.coilt = dev_get_adc_result()->coil_temp;
    cycle_log_s.coldt = dev_get_adc_result()->cold_junc_temp;
//    cycle_log_s.coilbt = get_coil_board_temp();
    cycle_log_s.zone1t = dev_get_adc_result()->zone1_temp;
    cycle_log_s.zone2t = dev_get_adc_result()->zone2_temp;
    cycle_log_s.chg_state = app_GetChargerState();
    cycle_log_s.partab = app_GetChargerPartab();
    cycle_log_s.usbv = app_GetVbusVolt()/1000.0;
    cycle_log_s.bat_J = app_GetBatJoules();
    cycle_log_s.fuel = app_get_bat_left();
    cycle_log_s.tc1_temp = dev_get_tc_temp(1);
    cycle_log_s.tc2_temp = dev_get_tc_temp(2);
    if(get_cycle_log_flag() == 1 && get_comm_lock() == 0)
    {
        if(get_heat_log_16ms_flag() == 0)
        {
            if(app_get_heat_cnt()%62 == 0){
                send_cycle_log_bytes((uint8_t*)&cycle_log_s, sizeof(cycle_log_t));
            }
        }
        else{
            send_cycle_log_bytes((uint8_t*)&cycle_log_s, sizeof(cycle_log_t));
        }
    }
    heat_session_log_s.max_susceptor_temp1 = (heat_session_log_s.max_susceptor_temp1 > cycle_log_s.zone1t) ? \
        heat_session_log_s.max_susceptor_temp1 : cycle_log_s.zone1t;
    heat_session_log_s.max_susceptor_temp2 = (heat_session_log_s.max_susceptor_temp2 > cycle_log_s.zone2t) ? \
        heat_session_log_s.max_susceptor_temp2 : cycle_log_s.zone2t;
    heat_session_log_s.max_bat_temp_session = (heat_session_log_s.max_bat_temp_session > cycle_log_s.batt) ? \
        heat_session_log_s.max_bat_temp_session : cycle_log_s.batt;
    heat_session_log_s.max_cold_junc_temp = (heat_session_log_s.max_cold_junc_temp > cycle_log_s.coldt) ? \
        heat_session_log_s.max_cold_junc_temp : cycle_log_s.coldt;
}

/*************************************************************************************************
  * @brief    : Timer call back function to send charging log
  * @param1   : Pointer to timer structure
  * @param2   : Pointer to parameter
  * @return   : None
*************************************************************************************************/
static void call_back_printf_chg_logs(const ptimer_t tm)
{
    cycle_log_s.ticks = GetTick();
    cycle_log_s.item = CHARGE_LOG;
    cycle_log_s.ses_time = 0;
    cycle_log_s.sp1 = 0;
    cycle_log_s.pv1 = dev_get_adc_result()->zone1_temp;
    cycle_log_s.sp2 = 0;
    cycle_log_s.pv2 = dev_get_adc_result()->zone2_temp;
    cycle_log_s.chan = 0;
    cycle_log_s.batv = dev_get_adc_result()->vbat;
    cycle_log_s.bata = dev_get_adc_result()->i_sense;
    cycle_log_s.CJR = 0;
    cycle_log_s.session_J = 0;
    cycle_log_s.usbt = dev_get_adc_result()->usb_temp;
    cycle_log_s.batt = dev_get_adc_result()->bat_temp;
    cycle_log_s.coilt = dev_get_adc_result()->coil_temp;
    cycle_log_s.coldt = dev_get_adc_result()->cold_junc_temp;
//    cycle_log_s.coilbt = get_coil_board_temp();
    cycle_log_s.zone1t = dev_get_adc_result()->zone1_temp;
    cycle_log_s.zone2t = dev_get_adc_result()->zone2_temp;
    cycle_log_s.chg_state = app_GetChargerState();
    cycle_log_s.partab = app_GetChargerPartab();
    cycle_log_s.usbv = app_GetVbusVolt()/1000.0;
    cycle_log_s.bat_J = app_GetBatJoules();
    cycle_log_s.fuel = app_get_bat_left();
    cycle_log_s.tc1_temp = dev_get_tc_temp(1);
    cycle_log_s.tc2_temp = dev_get_tc_temp(2);
    if (1 == get_cycle_log_flag() && get_comm_lock() == 0)
    {
        send_cycle_log_bytes((uint8_t*)&cycle_log_s, sizeof(cycle_log_t));
        if(get_charge_IC_log_flag() == 1)
        {
            app_send_charge_IC_reg();
        }
    }
}

/*************************************************************************************************
  * @brief    : Timer call back function to send idle log
  * @param1   : Pointer to timer structure
  * @param2   : Pointer to parameter
  * @return   : None
*************************************************************************************************/
static void call_back_printf_idle_logs(const ptimer_t tm)
{
    cycle_log_s.ticks = GetTick();
    cycle_log_s.item = IDLE_LOG;
    cycle_log_s.ses_time = 0;
    cycle_log_s.sp1 = 0;
    cycle_log_s.pv1 = dev_get_adc_result()->zone1_temp;
    cycle_log_s.sp2 = 0;
    cycle_log_s.pv2 = dev_get_adc_result()->zone2_temp;
    cycle_log_s.chan = 0;
    cycle_log_s.batv = dev_get_adc_result()->vbat;
    cycle_log_s.bata = dev_get_adc_result()->i_sense;
    cycle_log_s.CJR = 0;
    cycle_log_s.session_J = 0;
    cycle_log_s.usbt = dev_get_adc_result()->usb_temp;
    cycle_log_s.batt = dev_get_adc_result()->bat_temp;
    cycle_log_s.coilt = dev_get_adc_result()->coil_temp;
    cycle_log_s.coldt = dev_get_adc_result()->cold_junc_temp;
//    cycle_log_s.coilbt = get_coil_board_temp();
    cycle_log_s.zone1t = dev_get_adc_result()->zone1_temp;
    cycle_log_s.zone2t = dev_get_adc_result()->zone2_temp;
    cycle_log_s.chg_state = app_GetChargerState();
    cycle_log_s.partab = app_GetChargerPartab();
    cycle_log_s.usbv = app_GetVbusVolt()/1000.0;
    cycle_log_s.bat_J = app_GetBatJoules();
    cycle_log_s.fuel = app_get_bat_left();
    cycle_log_s.tc1_temp = dev_get_tc_temp(1);
    cycle_log_s.tc2_temp = dev_get_tc_temp(2);
    if (1 == get_cycle_log_flag() && get_comm_lock() == 0)
    {
        send_cycle_log_bytes((uint8_t*)&cycle_log_s, sizeof(cycle_log_t));
    }
//    LOGD("idle_printf: %d 0x%x %d %d %d %d %d %0.3f %0.3f %d %d %0.3f %0.3f",
//    cycle_log_s.ticks, cycle_log_s.item, cycle_log_s.sp1, cycle_log_s.pv1, cycle_log_s.sp2,
//    cycle_log_s.pv2, cycle_log_s.chan, cycle_log_s.batv, cycle_log_s.bata, cycle_log_s.CJR,
//    cycle_log_s.session_J, cycle_log_s.usbt, cycle_log_s.batt);
//    LOGD("idle_printf: %0.3f %0.3f %0.3f %d %d %d %d %0.3f %0.3f %d %d %d",
//    cycle_log_s.coilt, cycle_log_s.coldt, cycle_log_s.coilbt, cycle_log_s.zone1t, cycle_log_s.zone2t,
//    cycle_log_s.chg_state, cycle_log_s.partab, cycle_log_s.usbv, cycle_log_s.bat_J, cycle_log_s.fuel,
//    cycle_log_s.tc1_temp, cycle_log_s.tc2_temp);
}

/*************************************************************************************************
  * @brief    : Timer call back function to send error log
  * @param1   : Pointer to timer structure
  * @param2   : Pointer to parameter
  * @return   : None
*************************************************************************************************/
static void call_back_printf_error_logs(const ptimer_t tm)
{
    cycle_log_s.ticks = GetTick();
    cycle_log_s.item = ERROR_LOG;
    cycle_log_s.ses_time = 0;
    cycle_log_s.sp1 = 0;
    cycle_log_s.pv1 = dev_get_adc_result()->zone1_temp;
    cycle_log_s.sp2 = 0;
    cycle_log_s.pv2 = dev_get_adc_result()->zone2_temp;
    cycle_log_s.chan = 0;
    cycle_log_s.batv = dev_get_adc_result()->vbat;
    cycle_log_s.bata = dev_get_adc_result()->i_sense;
    cycle_log_s.CJR = 0;
    cycle_log_s.session_J = 0;
    cycle_log_s.usbt = dev_get_adc_result()->usb_temp;
    cycle_log_s.batt = dev_get_adc_result()->bat_temp;
    cycle_log_s.coilt = dev_get_adc_result()->coil_temp;
    cycle_log_s.coldt = dev_get_adc_result()->cold_junc_temp;
//    cycle_log_s.coilbt = get_coil_board_temp();
    cycle_log_s.zone1t = dev_get_adc_result()->zone1_temp;
    cycle_log_s.zone2t = dev_get_adc_result()->zone2_temp;
    cycle_log_s.chg_state = app_GetChargerState();
    cycle_log_s.partab = app_GetChargerPartab();
    cycle_log_s.usbv = app_GetVbusVolt()/1000.0;
    cycle_log_s.bat_J = app_GetBatJoules();
    cycle_log_s.fuel = app_get_bat_left();
    cycle_log_s.tc1_temp = dev_get_tc_temp(1);
    cycle_log_s.tc2_temp = dev_get_tc_temp(2);
    if (1 == get_cycle_log_flag() && get_comm_lock() == 0)
    {
        send_cycle_log_bytes((uint8_t*)&cycle_log_s, sizeof(cycle_log_t));
    }
}

/*************************************************************************************************
  * @brief    : creat or start a timer to auto shutdown 
  * @return   : None
*************************************************************************************************/
void start_auto_shutdown_timer(void)
{
    //TIMER_SAFE_RESET(mc.pshutdown_timer, mc.shut_down_ms, TIMER_OPT_ONESHOT, call_back_auto_shut_down, NULL);
    mc.pshutdown_timer = bat_timer_reset_ext(mc.pshutdown_timer, "pshutdown_timer", mc.shut_down_ms, TIMER_OPT_ONESHOT, call_back_auto_shut_down);
    bat_timer_start(mc.pshutdown_timer, portMAX_DELAY);

}

///*************************************************************************************************
//  * @brief    : creat or start a timer for OLED display lock
//  * @return   : None
//*************************************************************************************************/
//void start_oled_lock_timer(uint16_t lock_period) 
//{
//    //TIMER_SAFE_RESET(mc.poled_lock_timer, lock_period, TIMER_OPT_ONESHOT, call_back_exit_oled_lock, NULL);
//    mc.poled_lock_timer = bat_timer_reset_ext(mc.poled_lock_timer, "poled_lock_timer", lock_period, TIMER_OPT_ONESHOT, call_back_exit_oled_lock);
//    bat_timer_start(mc.poled_lock_timer, portMAX_DELAY);
//}


///*************************************************************************************************
//  * @brief    : creat or start a timer for OLED display lock
//  * @return   : None
//*************************************************************************************************/
//void start_exit_heating_lock_timer(uint16_t lock_period) 
//{
//    //TIMER_SAFE_RESET(mc.poled_lock_timer, lock_period, TIMER_OPT_ONESHOT, call_back_exit_oled_lock, NULL);
//    mc.poled_lock_timer = bat_timer_reset_ext(mc.poled_lock_timer, "poled_lock_timer", lock_period, TIMER_OPT_ONESHOT, call_back_exit_heating_lock);
//    bat_timer_start(mc.poled_lock_timer, portMAX_DELAY);
//}

static void change_stick_sensor_status(void)
{
    uint16_t stick_sensor_status_old;
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    stick_sensor_status_old = pfrt->stick_sensor_status;
    if(stick_sensor_status_old == STICK_SENSOR_OFF)
    {
        pfrt->stick_sensor_status = STICK_SENSOR_ON; 
    }
    else if(stick_sensor_status_old == STICK_SENSOR_ON)
    {
        pfrt->stick_sensor_status = STICK_SENSOR_OFF;
    }
    update_data_flash(USR_DATA, INVALID);
    change_stick_sensor_ui(pfrt->stick_sensor_status);
    LOGD("change stick_sensor_status to %d",pfrt->stick_sensor_status);
}

/*************************************************************************************************
  * @brief    : update timer 
  * @return   : None
*************************************************************************************************/
static void update_auto_shutdown_timer(void)
{
    start_auto_shutdown_timer();
}

/*************************************************************************************************
  * @brief    : delete shutdown timer 
  * @return   : None
*************************************************************************************************/
void stop_auto_shutdown_timer(void)
{
    //TIMER_SAFE_DELETE(mc.pshutdown_timer);
    if(mc.pshutdown_timer){
        if(bat_timer_delete(mc.pshutdown_timer, portMAX_DELAY)==pdPASS){
            mc.pshutdown_timer = NULL;
        }
    }
}

/*************************************************************************************************
  * @brief    : set auto shutdown timer duration
  * @param1   : duration
  * @return   : None
*************************************************************************************************/
void set_auto_shut_down_ms(uint32_t ms)
{
    if(ms>1000){
        mc.shut_down_ms = ms;
        if(mc.pshutdown_timer){
            update_auto_shutdown_timer();
        }
        LOGD("auto shut down ms is %d ms\r\n", ms);
    }else{
        LOGE("auto shut down ms must bigger than 1000ms\r\n");
    }
}

#ifdef AUTO_TEST
/*************************************************************************************************
  * @brief    : set auto test flag
  * @param1   : flag: 1 enable / 0 disable
  * @return   : None
*************************************************************************************************/
void set_auto_test_flag(uint8_t flag)
{
    if(flag == 1)
    {
        mc.auto_test_flag = 1;
        //autoTest, donot shutdown anymore
        stop_auto_shutdown_timer();
        //start autoTest timer
        mc.pauto_test_timer = bat_timer_reset_ext(mc.pauto_test_timer, "pauto_test_timer",60*1000, TIMER_OPT_PERIOD, call_back_auto_test);
        bat_timer_start(mc.pauto_test_timer, portMAX_DELAY);      
    }
    else if(flag == 0)
    {
        mc.auto_test_flag = 0;
        //stop autoTest immediately
        post_msg_to_manager(op_stop_heat);
        //TIMER_SAFE_DELETE(mc.pauto_test_timer);  
        if(mc.pauto_test_timer){
            if(bat_timer_delete(mc.pauto_test_timer, portMAX_DELAY)==pdPASS){
                mc.pauto_test_timer = NULL;
            }
        }        
    }
   
}
#endif
/*************************************************************************************************
  * @brief    : Start timer for sending cycle log
  * @param1   : Cycle log type
  * @return   : None
*************************************************************************************************/
static void start_printf_parameters_timer(uint8_t cycle_log_type)
{
    switch(cycle_log_type)
    {
        case IDLE_LOG:
            //TIMER_SAFE_RESET(mc.printf_timer, 1000, TIMER_OPT_PERIOD, call_back_printf_idle_logs, NULL);
            mc.printf_timer = bat_timer_reset_ext(mc.printf_timer, "printf_timer", 1000, TIMER_OPT_PERIOD, call_back_printf_idle_logs);
            bat_timer_start(mc.printf_timer, portMAX_DELAY);
            break;
        case CHARGE_LOG:
            //TIMER_SAFE_RESET(mc.printf_timer, 1000, TIMER_OPT_PERIOD, call_back_printf_chg_logs, NULL);
            mc.printf_timer = bat_timer_reset_ext(mc.printf_timer, "printf_timer", 1000, TIMER_OPT_PERIOD, call_back_printf_chg_logs);
            bat_timer_start(mc.printf_timer, portMAX_DELAY);
            break;
        case HEAT_LOG:
            memset(&heat_session_log_s, 0, sizeof(heat_session_log_s));
            //TIMER_SAFE_RESET(mc.printf_timer, 16, TIMER_OPT_PERIOD, call_back_printf_heat_logs, NULL);
            mc.printf_timer = bat_timer_reset_ext(mc.printf_timer, "printf_timer", 16, TIMER_OPT_PERIOD, call_back_printf_heat_logs);
            bat_timer_start(mc.printf_timer, portMAX_DELAY);
            break;
        case ERROR_LOG:
            //TIMER_SAFE_RESET(mc.printf_timer, 1000, TIMER_OPT_PERIOD, call_back_printf_error_logs, NULL);
            mc.printf_timer = bat_timer_reset_ext(mc.printf_timer, "printf_timer", 1000, TIMER_OPT_PERIOD, call_back_printf_error_logs);
            bat_timer_start(mc.printf_timer, portMAX_DELAY);
            break;
        default:
            LOGE("cycle_log_type error\r\n");
            break;
    }

}

/*************************************************************************************************
  * @brief    : Stop timer for sending cycle log
  * @return   : None
*************************************************************************************************/
static void stop_printf_parameters_timer(void)
{
    //TIMER_SAFE_DELETE(mc.printf_timer);
    if(mc.printf_timer){
        if(bat_timer_delete(mc.printf_timer, portMAX_DELAY)==pdPASS){
            mc.printf_timer = NULL;
        }
    }
}

#ifdef SSCOM
static void call_back_send_comm_lock_state(const ptimer_t tm)
{
    uint8_t comm_lock_state = get_comm_lock();
    comm_send(CMD_COMM_LOCK, PC_ADDR, &comm_lock_state, 1);
}
#endif
/*************************************************************************************************
  * @brief    : get manager state
  * @return   : state
*************************************************************************************************/
uint16_t app_get_state(void)
{
    return g_state;
}

/*************************************************************************************************
  * @brief    : record some environment when error occur
  * @return   : none
*************************************************************************************************/
void record_when_error_occur(const ptimer_t tm)
{
    static uint8_t record_times = 0;
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    pDataChangeFreq->records_when_err[record_times].vbat_adc = dev_get_adc(VBAT_VOLT);
    pDataChangeFreq->records_when_err[record_times].coil_t_adc = dev_get_adc(COIL_TEMP);
    pDataChangeFreq->records_when_err[record_times].bat_t_adc = dev_get_adc(BAT_TEMP);
    pDataChangeFreq->records_when_err[record_times].cold_junc_t_adc = dev_get_adc(COLD_JUNC);
    pDataChangeFreq->records_when_err[record_times].usb_t_adc = dev_get_adc(USB_TEMP);
    pDataChangeFreq->records_when_err[record_times].i_sense_adc = dev_get_adc(I_SENSE);
    pDataChangeFreq->records_when_err[record_times].tc1_adc = dev_get_adc(TC1);
    pDataChangeFreq->records_when_err[record_times].tc2_adc = dev_get_adc(TC2);
    uint8_t chg_ic_reg = 0;
    if(app_get_current_chrg_drv() == BQ25898E)
    {
            Dev_BQ25898X_ReadRegBits(BQ25898X_REG_0B,0xff,0,&chg_ic_reg);
            pDataChangeFreq->records_when_err[record_times].CHG_IC_REG_0B = chg_ic_reg;
            Dev_BQ25898X_ReadRegBits(BQ25898X_REG_0C,0xff,0,&chg_ic_reg);
            pDataChangeFreq->records_when_err[record_times].CHG_IC_REG_0C = chg_ic_reg;
    }
    else if(app_get_current_chrg_drv() == MP2731)
    {
        Dev_MP2731_ReadRegBits(MP2731_REG_0C,0xff,0,&chg_ic_reg);
        pDataChangeFreq->records_when_err[record_times].CHG_IC_REG_0B = chg_ic_reg;
        Dev_MP2731_ReadRegBits(MP2731_REG_0D,0xff,0,&chg_ic_reg);
        pDataChangeFreq->records_when_err[record_times].CHG_IC_REG_0C = chg_ic_reg;
    }
//    LOGD("record_times: %d, usb_temp_adc: %d, reg_0B: 0x%X, reg_0C: 0x%X", record_times,
//        pDataChangeFreq->records_when_err[record_times].usb_t_adc,
//        pDataChangeFreq->records_when_err[record_times].CHG_IC_REG_0B,
//        pDataChangeFreq->records_when_err[record_times].CHG_IC_REG_0C);
    record_times++;
    if(RECORD_TIMES_WHEN_ERROR <= record_times){
        record_times = 0;
        //TIMER_SAFE_DELETE(mc.record_when_err_timer);
        if(mc.record_when_err_timer){
            if(bat_timer_delete(mc.record_when_err_timer, portMAX_DELAY)==pdPASS){
                mc.record_when_err_timer = NULL;
            }
         }
    }
}

/*************************************************************************************************
  * @brief    : change manager state
  * @param    : new state
  * @return   : none
*************************************************************************************************/
void change_state(system_state_e newState)
{
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    static uint8_t cycle_log_flag = 0;
    /*auto shutdown timer for STATE IDLE*/
    if(newState == STATE_IDLE)
    {
        if (NO_USB_PLUG == app_check_usb_plug_status())  /*???????*/
        {
#ifdef AUTO_TEST
            if(mc.auto_test_flag == 0)
#endif
                start_auto_shutdown_timer();
        }
#ifdef AUTO_TEST       
        if(mc.auto_test_flag)
        {    //start autoTest timer
             mc.pauto_test_timer = bat_timer_reset_ext(mc.pauto_test_timer, "pauto_test_timer",60*1000, TIMER_OPT_PERIOD, call_back_auto_test);
             bat_timer_start(mc.pauto_test_timer, portMAX_DELAY);
        }
#endif
        start_printf_parameters_timer(IDLE_LOG);
    }
    else if(newState == STATE_CHARGE)
    {
        start_printf_parameters_timer(CHARGE_LOG);
#ifdef AUTO_TEST          
        if(mc.pauto_test_timer)
        {
            if(bat_timer_delete(mc.pauto_test_timer, portMAX_DELAY)==pdPASS){
               mc.pauto_test_timer = NULL;
            }
        }
#endif
    }
    else if(newState == STATE_BOOST_HEAT || newState == STATE_NORMAL_HEAT)
    {
        app_chg_entry_suspend(CHG_HEAT_SUSPEND);
        start_printf_parameters_timer(HEAT_LOG);
        mc.heat_start_time = GetTick();
        //LOGD("heat_start = %d", mc.heat_start_time);
    }
    else if(newState == STATE_ERROR)
    {
        app_chg_entry_error();
        start_printf_parameters_timer(ERROR_LOG);
        //stop_printf_parameters_timer();
        //TIMER_SAFE_RESET(mc.record_when_err_timer, 20, TIMER_OPT_PERIOD, record_when_error_occur, NULL);
        mc.record_when_err_timer = bat_timer_reset_ext(mc.record_when_err_timer, "record_when_err_timer",  20, TIMER_OPT_PERIOD, record_when_error_occur);
        bat_timer_start(mc.record_when_err_timer, portMAX_DELAY);
    }
    else if(newState == STATE_CIT)
    {
        stop_printf_parameters_timer();
    }

    if(g_state == STATE_IDLE && newState != STATE_IDLE)
    {
        stop_auto_shutdown_timer();
    }else if(g_state == STATE_BOOST_HEAT || g_state == STATE_NORMAL_HEAT){
        app_chg_try_to_exit_suspend(CHG_HEAT_SUSPEND);
        uint32_t heat_stop_time = GetTick();
        //LOGD("heat_stop = %d,heat_start =%d", heat_stop_time,mc.heat_start_time);
        if (heat_stop_time > mc.heat_start_time)
        {
            int32_t temp = (heat_stop_time - mc.heat_start_time)/1000;
            pDataChangeFreq->lifeCycleData[total_heat_time] += temp;
            //LOGD("heat_temp = %d",temp);
            //LOGD("heat_total = %d", pDataChangeFreq->lifeCycleData[total_heat_time]);
        }
    }

    if(newState == STATE_CHARGE && g_state != STATE_CHARGE)
    {
        pDataChangeFreq->lifeCycleData[chag_times]++;
        if (mc.bat_left >=95)
        {
            pDataChangeFreq->lifeCycleData[chag_sessions_begin_dark]++;
        }
        mc.charge_start_time = GetTick();
    }
    else if(g_state == STATE_CHARGE &&newState != STATE_CHARGE)
    {
        mc.charge_stop_time = GetTick();
        if (mc.charge_stop_time > mc.charge_start_time)
        {
            int32_t temp = (mc.charge_stop_time - mc.charge_start_time)/1000;
            if (temp > pDataChangeFreq->lifeCycleData[max_chg_time])
            {
                pDataChangeFreq->lifeCycleData[max_chg_time] = temp;
            }
            pDataChangeFreq->lifeCycleData[total_chg_time] += temp;
        }
    }

    if(g_state == STATE_CIT)//exit from cit
    {
        log_mask = mc.log_mask;
        set_cycle_log_flag(cycle_log_flag);
    }
    else if(newState == STATE_CIT)//entry cit
    {
        mc.log_mask = log_mask;
        log_mask = LOG_NULL;
        cycle_log_flag = get_cycle_log_flag();
        set_cycle_log_flag(0 );
    }

    LOGD("from %s to %s ...\r\n", g_state_name[g_state], g_state_name[newState]);
    g_state = newState;
}

/*************************************************************************************************
  * @brief    : exit cit state
  * @param    : new state
  * @return   : none
*************************************************************************************************/
static void exit_cit(void)
{
    clear_cit_mode_flag();
    /*turn off buzz*/
    app_haptic_enable(0);

    app_chg_exit_cit();
    change_state(STATE_IDLE);

    //TIMER_SAFE_DELETE(mc.pexit_cit_timer);
    if(mc.pexit_cit_timer){
        if(bat_timer_delete(mc.pexit_cit_timer, portMAX_DELAY)==pdPASS){
            mc.pexit_cit_timer = NULL;
        }
    }
}

/*************************************************************************************************
  * @brief    : timer callback for exit cit
  * @return   : none
*************************************************************************************************/
static void cb_exit_cit(const ptimer_t tm)
{
    LOGD("cit timeout\r\n");
    exit_cit();
}

/*************************************************************************************************
  * @brief    : get session result params
  * @param    : heat_session_log_t pointer
  * @return   : none
*************************************************************************************************/
void app_get_session_result_params(heat_session_log_t * ptr)
{
    ptr->max_susceptor_temp1=heat_session_log_s.max_susceptor_temp1;
    ptr->max_susceptor_temp2=heat_session_log_s.max_susceptor_temp2;
    ptr->max_bat_temp_session=heat_session_log_s.max_bat_temp_session;
    ptr->max_cold_junc_temp=heat_session_log_s.max_cold_junc_temp;
    ptr->session_duration=app_get_heat_cnt()*HEAT_ONE_CNT_TIME;
}
/*************************************************************************************************
  * @brief    : every_20th_session_check
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void every_20th_session_check(void){
    //every_20th_session_check
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    uint32_t session_num = pDataChangeFreq->lifeCycleData[session_starts];

    if(session_num - pDataChangeFreq->last_cleaning_num >= 20){

        start_oled_display_battery_check(app_get_bat_left());
        pDataChangeFreq->last_cleaning_num = session_num;
        update_data_flash(DATA_CHANGE_FREQUENT, SESSION_DATA);
    }else{
        // post_msg_to_oled_with_arg(op_oled_battery_check_timer,app_get_bat_left());
        start_oled_display_battery_check(app_get_bat_left());
    }
    if(app_get_bat_left()<6)
    {
        app_haptic_buzz('a');
    }
//    start_oled_lock_timer(4000);    //4000ms
    set_oled_lock_flag(OLED_BTN_LOCK);


}

/*************************************************************************************************
  * @brief    : heat finish handler_ in idle, heat not start as heating profile error
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_heat_start_fail(msg_st* pmsg)
{
    uint8_t heat_error_state = pmsg->value;
    app_haptic_buzz('c');
    LOGD("hapticC heat_start_fail");
    if(heat_error_state != HEAT_FINISH_PROFILE_ERROR){
         return;
    }
    start_error_ui(ERROR_RETURN);  //show return error once
}

/*************************************************************************************************
  * @brief    : heat finish handler 
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_heat_finish_c(msg_st* pmsg)
{
    uint8_t usb_plug;
    uint8_t heat_error_state = pmsg->value;
    heat_session_log_t session = {0};
    usb_plug = app_check_usb_plug_status();
    if (usb_plug == WRONG_USB_PLUG){
        LOGD("heat_finish wrong charger");
        show_wrong_charger();
    }else{
        LOGD("haptic C finish heat");
        app_haptic_buzz('c');
        if (heat_error_state == HEAT_FINISH_COMPLETELY){
            start_oled_transition_session_to_remove_stickout();
            set_oled_lock_flag(OLED_BTN_LOCK|OLED_CHG_LOCK);
         }
        else{
            //post_msg_to_oled(op_stop_heat);
            start_oled_heat_cancel();
        }
    }
     change_state(STATE_IDLE); //exit heating state, already lock the oled
     error_eol_check();
     /*record some heat parms to flash*/
     app_get_session_result_params(&session);
     if (session.session_duration != 0)
     {
        write_session_to_ram(session_duration, session.session_duration);
        write_session_to_ram(max_susceptor_temp1_session, session.max_susceptor_temp1);
        write_session_to_ram(max_susceptor_temp2_session, session.max_susceptor_temp2);
        write_session_to_ram(max_bat_temp_session, session.max_bat_temp_session);
        write_session_to_ram(max_cold_junc_temp_session, session.max_cold_junc_temp);
        update_data_flash(DATA_CHANGE_FREQUENT, SESSION_DATA);
     }
}

/*************************************************************************************************
  * @brief    : entry cit handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_entry_cit_c(msg_st* pmsg)
{
    if(app_get_state()==STATE_NORMAL_HEAT || app_get_state()==STATE_BOOST_HEAT){
        app_stop_heat();
    }
    start_oled_clear_black();
    set_cit_mode_flag();
    app_haptic_enable(0);
    app_chg_entry_cit();
    change_state(STATE_CIT);
    /*start a timer for auto exit fromt cit*/
    //TIMER_SAFE_RESET(mc.pexit_cit_timer, EXIT_CIT_TIME, TIMER_OPT_ONESHOT, cb_exit_cit, NULL);
    mc.pexit_cit_timer = bat_timer_reset_ext(mc.pexit_cit_timer, "pexit_cit_timer", EXIT_CIT_TIME, TIMER_OPT_ONESHOT, cb_exit_cit);
    bat_timer_start(mc.pexit_cit_timer, portMAX_DELAY);
}

/*************************************************************************************************
  * @brief    : exit cit handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_exit_cit_c(msg_st* pmsg)
{
    exit_cit();

}

/*************************************************************************************************
  * @brief    : timer callback for heat timer over
  * @return   : none
*************************************************************************************************/
//static void call_back_stop_heat(const timer_t *tm, void* param)
//{
//    LOGE("E: heat %ds timeover ...\r\n", HEAT_TIME_OVER/1000);
//    app_led_fade_off_all();
//    //TIMER_SAFE_DELETE(mc.pheat_timer);
//    change_state(STATE_IDLE);
//    app_stop_heat();
//}

/*************************************************************************************************
  * @brief    : 3short handler for common situation to show certification_craphics
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_3short_c(msg_st* pmsg)
{
    if(mc.hall_door_status == door_close)
    {
        if(get_oled_lock_flag(OLED_LONG_BTN_LOCK))
        {
            return;
        }
        if(WRONG_USB_PLUG == app_check_usb_plug_status())
        {
            show_wrong_charger();
            return;
        }
        clear_all_oled_lock();
        start_oled_certfct_graph();
        //LOGD("show_certifct_graph");
    }
}

/*************************************************************************************************
  * @brief    : button short up msg handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_short_up_idle(msg_st* pmsg)
{
#ifdef ENABLE_STICK_SENSOR
    if(mc.hall_door_status == door_close && get_oled_lock_flag(OLED_STICK_SENSOR_LOCK))
    {
        change_stick_sensor_status();
        return;
    }
#endif

    if(get_oled_lock_flag(OLED_BTN_LOCK))
    {
        return;
    }
    if(WRONG_USB_PLUG == app_check_usb_plug_status())
    {
        show_wrong_charger();
        return;
    }
    if(chg_timeout_flag == chg_timeout_show)
    {
        chg_timeout_flag = chg_timeout_noshow;
        start_oled_charge_over_time_ui(app_get_bat_left());
        set_oled_lock_flag(OLED_BTN_LOCK);
        return;
    }
//    post_msg_to_oled_with_arg(op_oled_battery_check_timer,app_get_bat_left());
    start_oled_display_battery_check(app_get_bat_left());
    // start_oled_lock_timer(4000);    //4000ms
    if(app_get_bat_left()<6)
    {
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK);
        app_haptic_buzz('a');
        LOGD("hapticA for lowSOC:%d", app_get_bat_left());
    }
    else
    {
        set_oled_lock_flag(OLED_BTN_LOCK);
        LOGD("bat_check SOC:%d", app_get_bat_left());
    }
}

/*************************************************************************************************
  * @brief    : button short down msg handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_short_down_idle(msg_st* pmsg)
{
    /*recount shutdown timer*/
    if (NO_USB_PLUG == app_check_usb_plug_status())
    {
        update_auto_shutdown_timer();
    }
}

/*************************************************************************************************
  * @brief    : button 1s5 down msg handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_1s5_down_idle(msg_st* pmsg)
{
    if(get_oled_lock_flag(OLED_LONG_BTN_LOCK))
    {
        return;
    }
    stop_auto_shutdown_timer();
    if(mc.hall_door_status == door_base)
    {
        app_start_normal_heat();
        if(app_get_heat_state() == 1)
        {
            app_haptic_buzz('a');
            LOGD("hapticA for BASE btn_1s5_down");
            start_oled_heat(BASE_WARM_UP);
            change_state(STATE_NORMAL_HEAT);
            mc.heating_button_flag = 1;
        }
    }
    else if(mc.hall_door_status == door_boost)
    {
        app_start_boost_heat();
        if(app_get_heat_state() == 1)
        {
            app_haptic_buzz('a');
            LOGD("hapticA for BOOST btn_1s5_down");
            start_oled_heat(BOOST_WARM_UP);
            change_state(STATE_BOOST_HEAT);
            mc.heating_button_flag = 1;
        }
    }else if(mc.hall_door_status == door_close)
    {
#ifdef ENABLE_STICK_SENSOR
        if(!get_oled_lock_flag(OLED_STICK_SENSOR_LOCK))
        {
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            start_stick_sensor_ui(pfrt->stick_sensor_status);
            set_oled_lock_flag(OLED_STICK_SENSOR_LOCK);
        }
        else
        {
            start_oled_clear_black();
            LOGD("exit_stick_sensor_idle");
        }
#endif
    }

}
/*************************************************************************************************
  * @brief    : button 1s5 up msg handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_1s5_up_idle(msg_st* pmsg)
{
    if(mc.hall_door_status == door_close)
    {
        start_auto_shutdown_timer();
        LOGD("btn_1s5_up_idle");
    }
}

/*************************************************************************************************
  * @brief    : hall door mode handler for common situation
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_hall_door_mode_c(msg_st* pmsg)
{
    uint8_t door_status = (uint8_t)pmsg->value;
    if(door_status == close2base)
    {
        mc.hall_door_status = door_base;
    }
    else if(door_status == base2boost)
    {
        mc.hall_door_status = door_boost;
    }
    else if(door_status == boost2base)
    {
        mc.hall_door_status = door_base;
    }
    else if(door_status == base2close)
    {
        mc.hall_door_status = door_close;
    }
    else if(door_status == boost2close)
    {
        mc.hall_door_status = door_close;
    }
    else if(door_status == close2boost)
    {
        mc.hall_door_status = door_boost;
    }
    else
    {
        mc.hall_door_status = door_error;
    }
}
/*************************************************************************************************
  * @brief    : hall door mode handler for boot state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_hall_door_mode_boot(msg_st* pmsg)
{
    uint8_t door_status = (uint8_t)pmsg->value;

    handle_hall_door_mode_c(pmsg);
    if(app_get_bat_left() < 6)
    {
        return;
    }

    if(door_status == base2close)
    {
        start_oled_display_mode_select(door_status);
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
    }
    else if(door_status == close2base)
    {
        start_oled_welcome_brand();
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
    }
    
//    start_oled_lock_timer(2200);    //2200ms for welcome animation
}
/*************************************************************************************************
  * @brief    : hall door mode handler for HEAT
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_hall_door_mode_heat(msg_st* pmsg)
{
    uint8_t door_status = (uint8_t)pmsg->value;

    handle_hall_door_mode_c(pmsg);
    if(door_status == base2close)
    {
        //todo: stop heat
        app_haptic_buzz('c');
        //TIMER_SAFE_DELETE(mc.pheat_timer);
        LOGD("hapticC for door close heat");
        app_stop_heat();

        start_no_mode_closing_animation();
        set_oled_lock_flag(OLED_CHG_LOCK|OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
        change_state(STATE_IDLE);
    }

}
/*************************************************************************************************
  * @brief    : hall door mode handler for IDLE
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_hall_door_mode_idle(msg_st* pmsg)
{
    uint8_t door_status = (uint8_t)pmsg->value;
    
    handle_hall_door_mode_c(pmsg);
    /*recount shutdown timer*/
    if (NO_USB_PLUG == app_check_usb_plug_status())
    {
        update_auto_shutdown_timer();
    }
    
    if(WRONG_USB_PLUG == app_check_usb_plug_status())
    {
        if(get_oled_lock_flag(OLED_HALL_LOCK))
        {
            return;
        }
        show_wrong_charger();
        return;
    }
    if(door_status == base2close)   // base2close
    {
        start_oled_display_mode_select(door_status);
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
//        start_oled_lock_timer(2000);    //2000ms for bye animation
    }
    else if(door_status == close2base)  // close2base
    {
        if(app_get_bat_left()>=6)
        {
            start_oled_welcome_brand();
            if(app_get_bat_left() == 100)
            {
                set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
            }
            else
            {
                set_oled_lock_flag(OLED_CHG_LOCK|OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
            }
//            start_oled_lock_timer(2200);    //2200ms for welcome animation
        }
    }
    if(get_oled_lock_flag(OLED_HALL_LOCK))
    {
        return;
    }
    //door_status: base2boost & boost2base
    if(app_get_bat_left()<6)
    {
        start_oled_display_battery_check(app_get_bat_left());
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK);
        app_haptic_buzz('a');
        LOGD("hapticA for lowSOC");
//        start_oled_lock_timer(4000);
    }
    else{
        start_oled_display_mode_select(door_status);
        clear_all_oled_lock();
    }

}



/*************************************************************************************************
  * @brief    : cli normal heat msg handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_normal_heat(msg_st* pmsg)
{
    stop_auto_shutdown_timer();


    app_start_normal_heat();
    if(app_get_heat_state() == 1)
    {
        app_haptic_buzz('a');
        LOGD("hapticA for NORMAL");
        start_oled_heat(BASE_WARM_UP);
        change_state(STATE_NORMAL_HEAT);
    }

}

/*************************************************************************************************
  * @brief    : cli boost heat msg handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_boost_heat(msg_st* pmsg)
{
    stop_auto_shutdown_timer();


    app_start_boost_heat();
    if(app_get_heat_state() == 1)
    {
        app_haptic_buzz('a');
        LOGD("hapticA for BOOST");
        start_oled_heat(BOOST_WARM_UP);
        change_state(STATE_BOOST_HEAT);
    }

}

/*************************************************************************************************
  * @brief    : soc msg handler in idle state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_bat_left_idle(msg_st* pmsg)
{
    if(pmsg->value == 100){//plug usb when full battary level
        if (WELL_USB_PLUG == app_check_usb_plug_status())
        {
            stop_auto_shutdown_timer();
        }
        /*Haptic A*/
        app_haptic_buzz('a');
        LOGD("hapticA bat_soc:%d", app_get_bat_left());
       
    }
}
static void handle_charge_timeout_idle(msg_st* pmsg)
{
    uint8_t flag = pmsg->value;

    if(flag == chg_timeout_clear)
    {
        if(chg_timeout_flag == chg_timeout_show)
        {
            chg_timeout_flag = chg_timeout_noshow;
            start_oled_charge_over_time_ui(app_get_bat_left());
            set_oled_lock_flag(OLED_BTN_LOCK);
        }
        else if(chg_timeout_flag == chg_timeout_noshow)
        {
//            every_20th_session_check();
            if(get_oled_lock_flag(OLED_HALL_LOCK))
            {
                return;
            }
            start_oled_display_battery_check(app_get_bat_left());
            if(app_get_bat_left()<6)
            {
                app_haptic_buzz('a');
            }
            set_oled_lock_flag(OLED_BTN_LOCK);
        }
        app_chg_try_to_exit_suspend(CHG_TIMEOUT_SUSPEND);
    }
}

/*************************************************************************************************
  * @brief    : charge msg handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_charge_idle(msg_st* pmsg)
{
    uint8_t usb_status = pmsg->value;
    uint8_t bat_soc = app_get_bat_left();
    if(usb_status == charge_begin)
    {
        change_state(STATE_CHARGE);
        //post_msg_to_oled_with_arg(op_oled_charging_check_timer,app_get_bat_left());
        start_oled_display_charging_check(bat_soc);
        set_oled_lock_flag(OLED_BTN_LOCK);
        LOGD("idle chg begin soc:%d",bat_soc);
        /*Haptic A*/
        app_haptic_buzz('a');
    }
    else if(usb_status == charge_full){//charge full
        /*usb unplug must start auto shutdown timer*/
        if (WELL_USB_PLUG == app_check_usb_plug_status())
        {
            stop_auto_shutdown_timer();
        }
        //post_msg_to_oled_with_arg(op_oled_battery_check_timer,app_get_bat_left());
        start_oled_display_battery_check(bat_soc);
        set_oled_lock_flag(OLED_BTN_LOCK);
        LOGD("idle chg full soc:%d",bat_soc);
        /*Haptic A*/
        app_haptic_buzz('a');
     }
    else if(usb_status == cable_disconnected){ //Disconnect charger
        LOGD("idle unplug charger");
        start_auto_shutdown_timer();
//        every_20th_session_check();
        if(get_oled_lock_flag(OLED_HALL_LOCK))
        {
            return;
        }
        start_oled_display_battery_check(app_get_bat_left());
        if(app_get_bat_left()<6)
        {
            app_haptic_buzz('a');
        }
        set_oled_lock_flag(OLED_BTN_LOCK);
    }else if(usb_status == wrong_charge){ //wrong charger
        LOGD("idle wrong charger");
        show_wrong_charger();
    }
}
/*************************************************************************************************
  * @brief    : button short up handler in normal heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_short_up_nh(msg_st* pmsg)
{
    //todo: start extend session if SESSION_EXTEND_TIME_START
    if(SESSION_EXTEND_TIME_START == app_get_session_extend_flag())
    {
        //set the heating time extend another 30s
        app_session_extend_function_on();
        start_enter_session_extended_ui();
        //start_oled_session_extend_function_on();
    }
  
}

/*************************************************************************************************
  * @brief    : button up 1.5s - 3s handler in both normal&boost heat
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_1s5_up_c(msg_st* pmsg)
{
    if(mc.heating_button_flag)
    {
        mc.heating_button_flag = 0;
    }
}
/*************************************************************************************************
  * @brief    : stop heating function in manager
  * @param    : none
  * @return   : none
*************************************************************************************************/
void manager_stop_heating(void)
{
    uint8_t usb_plug;

    //todo: stop heat
    app_stop_heat();
    usb_plug = app_check_usb_plug_status();
    if (usb_plug == WRONG_USB_PLUG){
        LOGD("stop heat wrong charge");
        show_wrong_charger();
    }else{
        LOGD("hapticC for stop heat");
        app_haptic_buzz('c');
        //post_msg_to_oled(op_stop_heat);
        if(mc.hall_door_status != door_close){
            start_oled_heat_cancel();
            set_oled_lock_flag(OLED_CHG_LOCK|OLED_HALL_LOCK|OLED_BTN_LOCK);
//            start_oled_lock_timer(1200);
        }else{
            start_no_mode_closing_animation();
            set_oled_lock_flag(OLED_BTN_LOCK);
        }
    }
    change_state(STATE_IDLE);
    error_eol_check();
}
/*************************************************************************************************
  * @brief    : button 5s down handler in normal heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_5s_down_nh(msg_st* pmsg)
{
    if(mc.heating_button_flag)
    {
        manager_stop_heating();
    }
}

/*************************************************************************************************
  * @brief    : button 3s down handler in normal heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_3s_down_nh(msg_st* pmsg)
{
    if(mc.heating_button_flag)
    {
        return;
    }
    manager_stop_heating();
}

/*************************************************************************************************
  * @brief    : cli stop heat handler
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_stop_heat(msg_st* pmsg)
{
     manager_stop_heating();
}

/*************************************************************************************************
  * @brief    : warmup finish handler in normal heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_heat_warmup_end_nh(msg_st* pmsg)
{
    LOGD("warmup finish");
    app_haptic_buzz('b');
    start_oled_heat(BASE_SESSION);
}

/*************************************************************************************************
  * @brief    : warmup finish handler in boost heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_heat_warmup_end_bh(msg_st* pmsg)
{
    LOGD("warmup finish");
    app_haptic_buzz('b');
    start_oled_heat(BOOST_SESSION);
}

/*************************************************************************************************
  * @brief    : heat left 20s  handler in normal heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_heat_session_extend_nh(msg_st* pmsg)
{
    if(SESSION_EXTEND_TIME_START == pmsg->value)
    {
        app_haptic_buzz('a');
        LOGD("hapticA for extend");
        //start_oled_session_extend_time_start();
        start_dis_session_extended_ui(1);
    }
}
static void handle_warm_up_ui_begin(msg_st* pmsg)
{
    start_oled_warm_up_ui();
}
static void handle_session_ui_begin(msg_st* pmsg)
{
    start_oled_session_ui();
}
static void handle_heat_up_remove_ui_begin(msg_st* pmsg)
{
    if(mc.heating_button_flag)
    {
        start_oled_clear_black();
    }
    else
    {
        start_oled_transition_heatup_to_remove_stickout();
        set_oled_lock_flag(OLED_BTN_LOCK|OLED_CHG_LOCK);
    }
}
static void handle_session_remove_ui_begin(msg_st* pmsg)
{
    start_oled_transition_session_to_remove_stickout();
    set_oled_lock_flag(OLED_BTN_LOCK|OLED_CHG_LOCK);
}
static void handle_remove_stick_ui_begin(msg_st* pmsg)
{
    start_op_oled_remove_stick_ui();
    if(app_check_usb_plug_status()!=WELL_USB_PLUG||app_get_suspend_state(CHG_CLI_SUSPEND))
    {
        clear_all_oled_lock();
    }else{
        set_oled_lock_flag(OLED_BTN_LOCK|OLED_CHG_LOCK);//when charger is connect
    }
}
static void handle_remove_mode_ui_begin(msg_st* pmsg)
{
    clear_all_oled_lock();
    if(app_check_usb_plug_status()!=WELL_USB_PLUG||app_get_suspend_state(CHG_CLI_SUSPEND))
    {
        if(app_get_bat_left()<6)
        {
            start_oled_display_battery_check(app_get_bat_left());
            set_oled_lock_flag(OLED_BTN_LOCK);
            app_haptic_buzz('a');
        }else{
            start_op_oled_remove_mode_ui();
        }
    }else{
        post_msg_to_oled(op_oled_clear_black);
    }
}
static void handle_battery_ui_begin(msg_st* pmsg)
{
    start_oled_display_battery_check(app_get_bat_left());
    if(app_get_bat_left()<6)
    {
        app_haptic_buzz('a');
    }
}
static void handle_cancel_session_ui_begin(msg_st* pmsg)
{
    g_session_extend_flag = NONE_SESSION_EXTEND;
    start_oled_heat_cancel();
}
static void handle_cancel_heat_up_ui_begin(msg_st* pmsg)
{
    start_oled_heat_cancel();
}
/*************************************************************************************************
  * @brief    : heat left 20s  handler in normal heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_heat_left10_nh(msg_st* pmsg)
{
    app_haptic_buzz('a');
//    start_dis_session_extended_ui(0);
//    start_oled_session_count_down();
}

/*************************************************************************************************
  * @brief    : button 5s down msg handler in boost heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_5s_down_bh(msg_st* pmsg)
{
    //heating start flag is still ON, will stop heating in button_5s_down
    if(mc.heating_button_flag)
    {
        manager_stop_heating();
    }
}
/*************************************************************************************************
  * @brief    : button 3s down msg handler in boost heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_3s_down_bh(msg_st* pmsg)
{
    //heating start flag is still ON, will not stop heating in button_3s_down
    if(mc.heating_button_flag)
    {
        return;
    }
    manager_stop_heating();
}


/*************************************************************************************************
  * @brief    : heat left20 msg handler in boost heat state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_heat_left10_bh(msg_st* pmsg)
{
    app_haptic_buzz('a');
//    start_oled_session_count_down();
}

/*************************************************************************************************
  * @brief    : show_wrong_charger
  * @param    : null
  * @return   : None
*************************************************************************************************/
void show_wrong_charger(void)
{
    start_oled_wrong_charger();
    set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK);
}
/*************************************************************************************************
  * @brief    : charge msg handler in charge state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_charge_charge(msg_st* pmsg)
{
    data_change_frequent_t* pDataChangeFreq = NULL;
    uint8_t bat_soc = app_get_bat_left();
    if(pmsg->value == charge_full){   //charge full
        LOGD("chg full SOC: %d",bat_soc);
        //if(get_oled_lock_flag(OLED_BTN_LOCK))
        //{
        //    return;
        //}
        //start_oled_display_battery_check(bat_soc);
        //set_oled_lock_flag(OLED_BTN_LOCK);
//        start_oled_lock_timer(4000);    //4000ms

        pDataChangeFreq = get_data_change_frequent_from_ram();
        pDataChangeFreq->lifeCycleData[chag_sessions_got_full_battery]++;
        change_state(STATE_IDLE);
    }else if(pmsg->value == cable_disconnected){ //Disconnect charger
        LOGD("chg unplug usb");
        change_state(STATE_IDLE);
//        every_20th_session_check();
        if(get_oled_lock_flag(OLED_HALL_LOCK))
        {
            return;
        }
        start_oled_display_battery_check(app_get_bat_left());
        if(app_get_bat_left()<6)
        {
            app_haptic_buzz('a');
        }
        set_oled_lock_flag(OLED_BTN_LOCK);
    }else if(pmsg->value == wrong_charge){ //wrong charger
        LOGD("chg wrong charger");
        change_state(STATE_IDLE);
        show_wrong_charger();
    }
}
/*************************************************************************************************
  * @brief    : charge msg handler in booting state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_charge_boot(msg_st* pmsg)
{
    data_change_frequent_t* pDataChangeFreq = NULL;

    if(pmsg->value == wrong_charge){ //wrong charger
        LOGD("boot wrong charger");
        show_wrong_charger();
    }
}
/*************************************************************************************************
  * @brief    : soc msg handler in charge state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_bat_left_charge(msg_st* pmsg)
{
    uint8_t bat_left = pmsg->value;
    data_change_frequent_t* pDataChangeFreq = NULL;
    mc.bat_left = bat_left;
    LOGD("bat_soc:%d", bat_left);
    if (bat_left==100)
    {
        if(get_oled_lock_flag(OLED_BTN_LOCK))
        {
            return;
        }
//        post_msg_to_oled_with_arg(op_oled_battery_check_timer,app_get_bat_left());
        start_oled_display_battery_check(bat_left);
        set_oled_lock_flag(OLED_BTN_LOCK);
//        start_oled_lock_timer(4000);    //4000ms

        pDataChangeFreq = get_data_change_frequent_from_ram();
        pDataChangeFreq->lifeCycleData[chag_sessions_got_full_battery]++;
        change_state(STATE_IDLE);
//      app_setChargerState(CHG_INVALID);
//      setChargerPartab(CHG_PARTAB_INVALID);

        return;
    }
}
static void handle_charge_timeout_charge(msg_st* pmsg)
{
    uint8_t flag = pmsg->value;
    if(flag == chg_timeout_occur)
    {
        chg_timeout_flag = chg_timeout_show;
        change_state(STATE_IDLE);
        app_chg_entry_suspend(CHG_TIMEOUT_SUSPEND);
    }
}
///*************************************************************************************************
//  * @brief    : button short down msg handler in charge state
//  * @param    : message pointer
//  * @return   : none
//*************************************************************************************************/
//static void handle_button_short_down_charge(msg_st* pmsg)
//{
//}
/*************************************************************************************************
  * @brief    : handle_button_short_up_charge in charge state
  * @return   : None
*************************************************************************************************/
static void handle_button_short_up_charge(msg_st* pmsg)
{
#ifdef ENABLE_STICK_SENSOR
    if(mc.hall_door_status == door_close && get_oled_lock_flag(OLED_STICK_SENSOR_LOCK))
    {
        change_stick_sensor_status();
        return;
    }
#endif

    if(get_oled_lock_flag(OLED_BTN_LOCK))
    {
        return;
    }

    //post_msg_to_oled_with_arg(op_oled_charging_check_timer,app_get_bat_left());
    start_oled_display_charging_check(app_get_bat_left());
    if(app_get_bat_left()<6)
    {
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK);
    }
    else
    {
        set_oled_lock_flag(OLED_BTN_LOCK);
    }
//    start_oled_lock_timer(4000);    //4000ms
    LOGD("btn_short_up_chg");
}

/*************************************************************************************************
  * @brief    : hold button 1s5 down msg handler in charge state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_1s5_down_charge(msg_st* pmsg)
{
    if(get_oled_lock_flag(OLED_LONG_BTN_LOCK))
    {
        return;
    }
    if(mc.hall_door_status == door_base)
    {
        app_start_normal_heat();
        if(app_get_heat_state() == 1)
        {
            app_haptic_buzz('a');
            LOGD("hapticA for NORMAL");
            start_oled_heat(BASE_WARM_UP);
            change_state(STATE_NORMAL_HEAT);
            mc.heating_button_flag = 1;
        }
    }
    else if(mc.hall_door_status == door_boost)
    {
        app_start_boost_heat();
        if(app_get_heat_state() == 1)
        {
            app_haptic_buzz('a');
            LOGD("hapticA for BOOST");
            start_oled_heat(BOOST_WARM_UP);
            change_state(STATE_BOOST_HEAT);
            mc.heating_button_flag = 1;
         }
    }
    else if(mc.hall_door_status == door_close)
    {
#ifdef ENABLE_STICK_SENSOR
        if(!get_oled_lock_flag(OLED_STICK_SENSOR_LOCK))
        {
            flash_record_t * pfrt = get_self_flash_record_from_ram();
            start_stick_sensor_ui(pfrt->stick_sensor_status);
            set_oled_lock_flag(OLED_STICK_SENSOR_LOCK);
        }
        else
        {
            start_oled_clear_black();
            LOGD("exit_stick_sensor_chg");
        }
#endif
    }
}
/*************************************************************************************************
  * @brief    : button 1s5 up msg handler in charge state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_button_1s5_up_charge(msg_st* pmsg)
{
    if(mc.hall_door_status == door_close)
    {
        LOGD("door_close_btn_1s5_up");
    }
}

/*************************************************************************************************
  * @brief    : hall door mode handler for CHARGE
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_hall_door_mode_charge(msg_st* pmsg)
{
    uint8_t door_status = (uint8_t)pmsg->value;

    handle_hall_door_mode_c(pmsg);
    /*recount shutdown timer*/
    if (NO_USB_PLUG == app_check_usb_plug_status())
    {
        update_auto_shutdown_timer();
    }

    if(door_status == base2close)
    {
        start_oled_display_mode_select(door_status);
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
//        start_oled_lock_timer(2000);    //2000ms for bye animation
    }
    else if(door_status == close2base)
    {
        if(app_get_bat_left()>=6)
        {
            start_oled_welcome_brand();
            set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK|OLED_LONG_BTN_LOCK);
//            start_oled_lock_timer(2200);    //2200ms for brand animation
        }
    }

    if(get_oled_lock_flag(OLED_HALL_LOCK))
    {
        return;
    }
    //door_status: base2boost & boost2base
    if(app_get_bat_left()<6)
    {
        start_oled_display_charging_check(app_get_bat_left());
        set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK);
//        start_oled_lock_timer(4000);
    }
    else{
        start_oled_display_mode_select(door_status);
        clear_all_oled_lock();
    }
}

/*************************************************************************************************
  * @brief    : update cit timer msg handler 
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_update_cit_timer(msg_st* pmsg)
{
    //TIMER_SAFE_RESET(mc.pexit_cit_timer, EXIT_CIT_TIME, TIMER_OPT_ONESHOT, cb_exit_cit, NULL);
    mc.pexit_cit_timer = bat_timer_reset_ext(mc.pexit_cit_timer, "pexit_cit_timer", EXIT_CIT_TIME, TIMER_OPT_ONESHOT, cb_exit_cit);
    bat_timer_start(mc.pexit_cit_timer, portMAX_DELAY);
}
/*************************************************************************************************
  * @brief    : display_mode_ui_after_clear_batvover_error
  * @param    : none
  * @return   : none
*************************************************************************************************/
static void display_mode_after_clear_batvover(void)
{
    if(app_get_hall_door_status()==door_base)
    {
        start_oled_display_mode_select(base_open);
    }
    else if(app_get_hall_door_status()==door_boost)
    {
        start_oled_display_mode_select(boost_open);
    }
}
/*************************************************************************************************
  * @brief    : Get the value that represents the currently existing errors
  * @return   : Global variable g_error_code
*************************************************************************************************/
uint64_t get_error_code(void)
{
    return mc.error_code;
}

static void error_oled_output_ui(uint8_t errorPosTemp)
{
    clear_all_oled_lock();
    if(flt_de_bat_cold_charge == errorPosTemp||flt_de_bat_cold_heat == errorPosTemp)
    {
        start_error_ui(ERROR_TOO_COLD);
    }else if(errorPosTemp == flt_de_bat_hot_40_pre_ses || errorPosTemp == flt_de_bat_hot_55
               || errorPosTemp == flt_de_bat_hot_50_charging|| errorPosTemp == flt_de_usb_hot|| errorPosTemp ==flt_de_co_hot_sw
               || errorPosTemp ==flt_de_co_junc_hot|| errorPosTemp == flt_de_tc_zone2_hot|| errorPosTemp == flt_de_tc_zone1_hot)
    {
        start_error_ui(ERROR_TOO_HOT);
    }else if(errorPosTemp == flt_de_bat_discharge_current_over ||errorPosTemp == flt_de_bat_charge_current_over|| errorPosTemp == flt_de_cic_config_error){
        start_oled_error_reset_oneshot();
    }else if(errorPosTemp == flt_de_bat_damage || errorPosTemp == flt_de_cic_output_voltage ){
        start_error_ui(ERROR_RETURN);
    }else if(errorPosTemp ==  flt_de_end_of_life){
        start_error_ui(ERROR_EOL);
//    }else if(errorPosTemp == flt_de_cic_charge_timeout)
//    {
//        start_oled_charge_over_time_ui(app_get_bat_left());
    }else if(errorPosTemp == flt_de_bat_voltage_over)
    {
        start_oled_over_voltage_ui();
    }

}
/*************************************************************************************************
  * @brief    : Deal with the new error message
  * @param1   : Pointer to message
  * @return   : 1(error is one-shot type or error is continuous type and newly occurred)
                0(error is continuous type and have existed)
*************************************************************************************************/
static uint8_t pre_press_error_code(msg_st* pmsg)
{
    uint8_t errorPosTemp = 0xFF;
    uint8_t retVal = 1;
    //errorPosTemp = get_error_pos(pmsg->value);
    errorPosTemp = pmsg->value;
    uint64_t errorCode = get_error_code();
    LOGD("errorPosTemp:%d,errorCode:0x%llx",errorPosTemp, errorCode);
    if (0xFF == errorPosTemp)
    {
        return 0;
    }
    if ((op_error_occur == pmsg->opcode) && (errorCode&((uint64_t)1 << errorPosTemp)))
    {
         return 0;
    }
    if ((op_error_occur == pmsg->opcode) && (errorPosTemp >= flt_de_bat_hot_55) && (errorPosTemp <= flt_de_bat_hot_50_charging))
    {
        LOGD("continue error: %d ",errorPosTemp);
        if (errorPosTemp == flt_de_bat_cold_charge ||errorPosTemp == flt_de_bat_cold_heat || errorPosTemp == flt_de_bat_hot_50_charging
            || errorPosTemp == flt_de_usb_hot || errorPosTemp == flt_de_bat_voltage_over
            //|| errorPosTemp == flt_de_bat_discharge_current_over || errorPosTemp == flt_de_bat_charge_current_over
            || errorPosTemp == flt_de_co_junc_hot)
        {
            if (g_state == STATE_CHARGE)
            {
                LOGD("abort charge");

                if (flt_de_usb_hot == errorPosTemp)
                {
                    app_setChargerState(CHG_USBHOT);
                }
                if (flt_de_bat_cold_charge == errorPosTemp)
                {
//                    setChargerPartab(CHG_PARTAB_SUSPEND_BAT_COLD);
                }
                if (flt_de_bat_hot_50_charging == errorPosTemp)
                {
//                    setChargerPartab(CHG_PARTAB_SUSPEND_BAT_HOT);
                }
            }
        }
        if(errorPosTemp == flt_de_bat_hot_55 || errorPosTemp == flt_de_bat_cold_heat
            || errorPosTemp == flt_de_tc_zone1_hot || errorPosTemp == flt_de_tc_zone2_hot
            || errorPosTemp == flt_de_usb_hot || errorPosTemp == flt_de_co_hot_sw
            || errorPosTemp == flt_de_bat_cold_charge
            //|| errorPosTemp == flt_de_bat_damage
            //|| errorPosTemp == flt_de_bat_discharge_current_over
            || errorPosTemp == flt_de_co_junc_hot)
        {
            if ((g_state == STATE_BOOST_HEAT) || (g_state == STATE_NORMAL_HEAT))
            {
                LOGD("abort heat");
                app_stop_heat();
                //post_msg_to_oled(op_stop_heat);
                start_oled_clear_black();// error has a high priority, and the error is directly black screen
            }
        }
    }
    if ((op_error_occur == pmsg->opcode) && (errorPosTemp <= flt_de_bat_i_sense_damage))
    {
        if(errorPosTemp == war_de_bat_low)
        {
            // post_msg_to_oled_with_arg(op_oled_battery_check_timer,app_get_bat_left());
            start_oled_display_battery_check(app_get_bat_left());
            set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK);
//            start_oled_lock_timer(4000);
        }
        else if(errorPosTemp == war_de_bat_empty)
        {
            // post_msg_to_oled_with_arg(op_oled_battery_check_timer,app_get_bat_left());
            start_oled_display_battery_check(0);
            set_oled_lock_flag(OLED_HALL_LOCK|OLED_BTN_LOCK);
//            start_oled_lock_timer(4000);
        }
        else if(errorPosTemp>=flt_de_tc_zone_imbalance&&errorPosTemp<=flt_de_bat_i_sense_damage)
        {
            start_error_ui(ERROR_WAIT_GENERAL);
        }
//        else if(errorPosTemp != flt_de_bat_hot_40_pre_ses)
//        {
//            //post_msg_to_oled(op_stop_heat);
//            start_oled_clear_black();// error has a high priority, and the error is directly black screen
//        }

    }

    return retVal;
}

/*************************************************************************************************
  * @brief    : Deal with the new error code message
  * @param1   : op_error_occur/op_error_clear
  * @param2   : The value of error code
  * @return   : 1(device need to get into error mode)/0(do nothing)
*************************************************************************************************/
uint8_t process_error_code(uint8_t errorType, uint32_t value)
{
    uint8_t retVal = 0;
    uint8_t errorPosTemp = 0xFF;
    LOGD("errorType=%d, errorPos=%d",errorType,value);
    errorPosTemp = value;
    if (errorPosTemp == 0xFF)
    {
        return retVal;
    }
    //LOGD("errorPosTemp=%d\n\r",errorPosTemp);
    if (op_error_occur == errorType)//error occur
    {
        write_error_occur_num_to_ram((errorCode_e)errorPosTemp, 1);
        upload_error((errorCode_e)errorPosTemp);
        if (errorPosTemp < error_max)
        {
            //LOGD("record error");
            mc.error_code |= ((uint64_t)1 << errorPosTemp);
            retVal = 1;
        }

        if (0xFF == mc.errorPos)//no existing continous error
        {
            mc.errorPos = errorPosTemp;
            err_type_e err_type = record_err_type(mc.errorPos);
            start_error_led_output(err_type);
            error_oled_output_ui(mc.errorPos);
            //LOGD("mc.errorPos=%d",mc.errorPos);
            retVal = 1;
        }
        
    }
    else if(op_error_clear == errorType)//error clear
    {
        boot_record_t *brt = get_boot_record_from_ram();

        //LOGD("mc.error_code:0x%x", mc.error_code);
        LOGD("clear errorPos is 0x%x", mc.errorPos);

        mc.error_code &= (~((uint64_t)1 << errorPosTemp));
        if(0xFFFFFFFF != brt->error_pos){
            mc.error_code |= ((uint64_t)1 << brt->error_pos);//return or reset error must be cleared by specific way
        }
       // LOGD("mc.error_code is %d", mc.error_code);
        if(!mc.error_code && NULL == g_error_timer)
        {
            if(mc.errorPos == flt_de_bat_voltage_over)
            {
                display_mode_after_clear_batvover();
            }
            mc.errorPos = 0xFF;
            app_chg_exit_error();
            change_state(STATE_IDLE);
           // LOGD("exit error state,g_state=%d\n\r",g_state);
            //TIMER_SAFE_DELETE(err_exist_timer);
            if(err_exist_timer){
                if(bat_timer_delete(err_exist_timer, portMAX_DELAY)==pdPASS){
                    err_exist_timer = NULL;
                }
            }
        }
    }
    else
    {
        ;
    }
    return retVal;
}

/*************************************************************************************************
  * @brief    : Handle error message
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_error(msg_st* pmsg)
{
    if (0 == pre_press_error_code(pmsg))
    {
        return;
    }
    if (1 == process_error_code(pmsg->opcode, pmsg->value))
    {
        if (STATE_ERROR != g_state)
        {
            change_state(STATE_ERROR);
        }
        //LOGD("enter error state,g_state=%d\n\r",g_state);
    }
}

/*************************************************************************************************
  * @brief    : Handle error led display finished message
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_error_led_output_finish(msg_st* pmsg)
{
    LOGD("-----------------handle_error_led_output_finish---------------------\n\r");

    mc.error_code &= CONTINUOUS_MASK;//clear one shot error record after error UI finish
    if(!mc.error_code)
    {
        app_chg_exit_error();
        change_state(STATE_IDLE);
//        start_oled_clear_black();
        if(mc.errorPos == flt_de_bat_voltage_over)
        {
            display_mode_after_clear_batvover();
        }
        mc.errorPos = 0xFF;
        LOGD("exit error state,g_state=%d",g_state);
        //TIMER_SAFE_DELETE(err_exist_timer);
        if(err_exist_timer){
            if(bat_timer_delete(err_exist_timer, portMAX_DELAY)==pdPASS){
                err_exist_timer = NULL;
            }
        }
    }
}

/*************************************************************************************************
  * @brief    : Clear return error flag
  * @param    : None
  * @return   : None
*************************************************************************************************/
void clear_return_err_flag(void)
{
    if(mc.error_exist_due_to_record == 1){
        boot_record_t * brt = get_boot_record_from_ram();
        if(ERR_EXIST == brt->return_err){
            brt->return_err = ERR_CLEAR;
            mc.error_code &= ~((uint64_t)1 << brt->error_pos);
            mc.error_exist_due_to_record = 0;
            LOGD("error_code is %lld", mc.error_code);
            if(!mc.error_code)
            {
                mc.errorPos = 0xFF;
                app_chg_exit_error();
                change_state(STATE_IDLE);
                //LOGD("exit error state,g_state=%d\n\r",g_state);
                //TIMER_SAFE_DELETE(err_exist_timer);
                if(err_exist_timer){
                    if(bat_timer_delete(err_exist_timer, portMAX_DELAY)==pdPASS){
                        err_exist_timer = NULL;
                    }
                }
            }
            brt->error_pos = 0xFFFFFFFF;
            update_data_flash(BOOT_RECORD, INVALID);
            power_soft_reset();
        }
    }
}

/*************************************************************************************************
  * @brief    : Handle existing error message
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_existing_error(msg_st* pmsg)
{
    if(RETURN_ERR == pmsg->value || RESET_ERR == pmsg->value){
        boot_record_t *brt = get_boot_record_from_ram();
        mc.error_exist_due_to_record = 1;
        if(0xFF != brt->error_pos)
        {
            mc.error_code |= (uint64_t)1 << brt->error_pos;
            upload_error((errorCode_e)(brt->error_pos));
        }
        start_error_led_output((err_type_e)pmsg->value);
        change_state(STATE_ERROR);
        //LOGD("enter error state,g_state=%d\n\r",g_state);
    }

    switch(pmsg->value)
    {
        case RETURN_ERR:
            LOGD("Device needs to be return to service center");
            break;
        case RESET_ERR:
            LOGD("Reset device may resolve this error");
            break;
        default:
            break;
    }
}
static void error_ui_repeat(void){
    if(NULL == g_error_timer){//no error ui displaying
            if(get_error_code() & CONTINUOUS_MASK){//has continuous error in record
                uint8_t error_pos = 0xFF;
                for(uint8_t i = 0; i < 64; i++)
                {
                    if(get_error_code() & ((uint64_t)1 << i))
                    {
                        error_pos = i;
                        err_type_e err_type = record_err_type(error_pos);
                        upload_error((errorCode_e)error_pos);
                        start_error_led_output(err_type);
                        error_oled_output_ui(error_pos);
                        break;
                    }
                }
            }
            else{
                LOGD("no continuous error but in error state,error_code is 0x%llx", get_error_code());
            }
        }
}



/*************************************************************************************************
  * @brief    : Handle hall message in ERR
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_error_ui_repeat_by_hall(msg_st* pmsg)
{
    handle_hall_door_mode_c(pmsg);
    if(get_oled_lock_flag(OLED_HALL_LOCK))
    {
        return;
    }
    error_ui_repeat();
}
/*************************************************************************************************
  * @brief    : Handle button down message
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_error_ui_repeat_by_btn(msg_st* pmsg)
{
    error_ui_repeat();
}

/*************************************************************************************************
  * @brief    : charge msg handler in error state
  * @param    : message pointer
  * @return   : none
*************************************************************************************************/
static void handle_charge_error(msg_st* pmsg)
{
    if(get_oled_lock_flag(OLED_CHG_LOCK))
    {
        return;
    }
    if(pmsg->value == cable_connect){
        error_ui_repeat();
    }
}
static void handle_error_ui_reset_continue(msg_st* pmsg)
{
    uint8_t error_pos = 0xFF;
    for(uint8_t i = 0; i < 64; i++)
    {
        if(get_error_code() & ((uint64_t)1 << i))
        {
        error_pos = i;
            err_type_e err_type = record_err_type(error_pos);
            if(err_type == RESET_ERR)
            {
                start_oled_error_reset_continue();
                set_oled_lock_flag(OLED_CHG_LOCK|OLED_HALL_LOCK);
            }
        }
    }
}
static void handle_error_ui_reset_oneshot(msg_st* pmsg)
{
    uint8_t error_pos = 0xFF;
    for(uint8_t i = 0; i < 64; i++)
    {
        if(get_error_code() & ((uint64_t)1 << i))
        {
        error_pos = i;
            err_type_e err_type = record_err_type(error_pos);
            if(err_type == RESET_ERR)
            {
                clear_all_oled_lock();
                start_oled_error_reset_oneshot();
            }
        }
    }
}

/*************************************************************************************************
  * @brief    : get cmd disable charge flag
  * @return   : None
*************************************************************************************************/
static void handle_cmd_disable_charge(msg_st* pmsg)
{
    app_chg_entry_suspend(CHG_CLI_SUSPEND);
    if (g_state == STATE_CHARGE)
    {
        change_state(STATE_IDLE);
    }

    app_setChargerState(CHG_LOCKOUT);
    LOGD("cmd_disable_charge");
}
/*************************************************************************************************
  * @brief    : stop heat when in UI updating
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_UI_updating(msg_st* pmsg)
{
    if(app_get_state()==STATE_NORMAL_HEAT || app_get_state()==STATE_BOOST_HEAT){
        app_stop_heat();
    }
    app_chg_entry_suspend(CHG_UPDATE_IMAGE_SUSPEND);
    stop_auto_shutdown_timer();
    start_oled_clear_black();
    dis_color_msg(OLED_BLUE);
}

/*************************************************************************************************
  * @brief    : cli charge enable msg handler 
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_cmd_enable_charge(msg_st* pmsg)
{
    app_chg_try_to_exit_suspend(CHG_CLI_SUSPEND);
}

/*************************************************************************************************
  * @brief    : timer call back for sw3v3
  * @return   : None
*************************************************************************************************/
//void mcu_sw33_init(const timer_t *tm, void* param)
//{
//    GPIO_InitTypeDef  GPIO_InitStruct;
//    //PD3 SW_3.3V
//    GPIO_InitStruct.Pin       = GPIO_PIN_3;
//    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull      = GPIO_NOPULL;
//
//    hwi_GPIO_Init(GPIOD, &GPIO_InitStruct);
    //hwi_GPIO_Init();

//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
    //hwi_GPIO_WritePin(EN_3V3_SW_E, HWI_PIN_SET);

    //LOGD("3.3 sw pin state: %d", HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3));
//}

/*************************************************************************************************
  * @brief    : handle_boot_complete
  * @param1   : Pointer to message
  * @return   : None
*************************************************************************************************/
static void handle_boot_complete(msg_st* pmsg)
{
    /*if(ERR_EXIST == brt->return_err){
        post_msg_to_manager_with_arg(op_existing_error, RETURN_ERR);
    }
    else if(ERR_EXIST == brt->reset_err){
        post_msg_to_manager_with_arg(op_existing_error, RESET_ERR);
    }
    else{*/
        app_chg_try_to_exit_suspend(CHG_HW_RESET_SUSPEND);
//        LOGD("boot complete,entry to IDLE");
        change_state(STATE_IDLE);
    //}
    /*start error check*/
    set_error_check_status(enable_s);
}

/*STATE_BOOTING*/
BEGIN_MESSAGE_MAP(state_booting)
ADD_NEW_MSG_ITEM(opt_boot_complete,  handle_boot_complete)
ADD_NEW_MSG_ITEM(op_hall_door_mode,  handle_hall_door_mode_boot)
ADD_NEW_MSG_ITEM(op_existing_error,  handle_existing_error)
ADD_NEW_MSG_ITEM(op_charge,  handle_charge_boot)
END_MESSAGE_MAP


/*STATE_IDLE*/
BEGIN_MESSAGE_MAP(state_idle)
ADD_NEW_MSG_ITEM(op_btn_short_down,    handle_button_short_down_idle)
ADD_NEW_MSG_ITEM(op_btn_short_up,      handle_button_short_up_idle)
ADD_NEW_MSG_ITEM(op_btn_1s5_down, handle_button_1s5_down_idle)
ADD_NEW_MSG_ITEM(op_btn_1s5_up,   handle_button_1s5_up_idle)
ADD_NEW_MSG_ITEM(op_btn_3short,   handle_button_3short_c)
//ADD_NEW_MSG_ITEM(op_btn_oneshort_onelong,    handle_show_stick_sensor)
ADD_NEW_MSG_ITEM(op_heat_finish,    handle_heat_start_fail)

//ADD_NEW_MSG_ITEM(op_btn_2s_up,   handle_button_idle)
//ADD_NEW_MSG_ITEM(op_btn_2s_down, handle_button2s_idle)
//ADD_NEW_MSG_ITEM(op_btn_3s_down, handle_button3s_idle)
//ADD_NEW_MSG_ITEM(op_btn_2s_up,   handle_button_2s_up_idle)
ADD_NEW_MSG_ITEM(op_hall_door_mode,  handle_hall_door_mode_idle)

ADD_NEW_MSG_ITEM(op_entry_cit,    handle_entry_cit_c)
ADD_NEW_MSG_ITEM(op_charge,       handle_charge_idle)
ADD_NEW_MSG_ITEM(op_charge_timeout,    handle_charge_timeout_idle)
//ADD_NEW_MSG_ITEM(op_bat_left,     handle_bat_left_idle)
ADD_NEW_MSG_ITEM(op_normal_heat,  handle_normal_heat)
ADD_NEW_MSG_ITEM(op_boost_heat,   handle_boost_heat)
ADD_NEW_MSG_ITEM(op_error_occur,  handle_error)
ADD_NEW_MSG_ITEM(opt_cmd_disable_charge,  handle_cmd_disable_charge)
ADD_NEW_MSG_ITEM(opt_cmd_enable_charge,  handle_cmd_enable_charge)
ADD_NEW_MSG_ITEM(op_oled_heatup_remove,    handle_heat_up_remove_ui_begin)
ADD_NEW_MSG_ITEM(op_oled_cancel_session,    handle_cancel_session_ui_begin)
ADD_NEW_MSG_ITEM(op_oled_cancel_heatup,    handle_cancel_heat_up_ui_begin)
ADD_NEW_MSG_ITEM(op_oled_session_to_remove,    handle_session_remove_ui_begin)
ADD_NEW_MSG_ITEM(op_oled_remove_stick_ui,    handle_remove_stick_ui_begin)
ADD_NEW_MSG_ITEM(op_oled_remove_mode_ui,    handle_remove_mode_ui_begin)
ADD_NEW_MSG_ITEM(op_oled_battery_check,    handle_battery_ui_begin)
ADD_NEW_MSG_ITEM(op_UI_updating,  handle_UI_updating)
END_MESSAGE_MAP

/*STATE_NORMAL_HEAT*/
BEGIN_MESSAGE_MAP(state_normal_heat)
ADD_NEW_MSG_ITEM(op_btn_short_up,   handle_button_short_up_nh)
ADD_NEW_MSG_ITEM(op_btn_1s5_up,     handle_button_1s5_up_c)
ADD_NEW_MSG_ITEM(op_btn_3s_down,    handle_button_3s_down_nh)
ADD_NEW_MSG_ITEM(op_btn_5s_down,    handle_button_5s_down_nh)
//ADD_NEW_MSG_ITEM(op_btn_2s_down,      handle_button2s_nh)
//ADD_NEW_MSG_ITEM(op_btn_2s_up,        handle_button2s_up_nh)

ADD_NEW_MSG_ITEM(op_heat_end_soon,   handle_heat_left10_nh)
ADD_NEW_MSG_ITEM(op_heat_session_extend,   handle_heat_session_extend_nh)
ADD_NEW_MSG_ITEM(op_oled_warm_up,   handle_warm_up_ui_begin)
ADD_NEW_MSG_ITEM(op_heat_warmup_finish_nh, handle_heat_warmup_end_nh)
ADD_NEW_MSG_ITEM(op_oled_session, handle_session_ui_begin)
ADD_NEW_MSG_ITEM(op_heat_finish,    handle_heat_finish_c)
ADD_NEW_MSG_ITEM(op_stop_heat,      handle_stop_heat)
ADD_NEW_MSG_ITEM(op_hall_door_mode,  handle_hall_door_mode_heat)
ADD_NEW_MSG_ITEM(op_error_occur,  handle_error)
ADD_NEW_MSG_ITEM(opt_cmd_disable_charge,  handle_cmd_disable_charge)
ADD_NEW_MSG_ITEM(opt_cmd_enable_charge,  handle_cmd_enable_charge)
ADD_NEW_MSG_ITEM(op_UI_updating,  handle_UI_updating)
ADD_NEW_MSG_ITEM(op_entry_cit,    handle_entry_cit_c)
END_MESSAGE_MAP

/*STATE_BOOST_HEAT*/
BEGIN_MESSAGE_MAP(state_boost_heat)
ADD_NEW_MSG_ITEM(op_btn_1s5_up,        handle_button_1s5_up_c)
ADD_NEW_MSG_ITEM(op_btn_3s_down,       handle_button_3s_down_bh)
ADD_NEW_MSG_ITEM(op_btn_5s_down,       handle_button_5s_down_bh)
//ADD_NEW_MSG_ITEM(op_btn_2s_down,      handle_button2s_bh)
//ADD_NEW_MSG_ITEM(op_btn_2s_up,   handle_button2s_up_bh)
ADD_NEW_MSG_ITEM(op_oled_warm_up,   handle_warm_up_ui_begin)
ADD_NEW_MSG_ITEM(op_heat_warmup_finish_bh, handle_heat_warmup_end_bh)
ADD_NEW_MSG_ITEM(op_oled_session, handle_session_ui_begin)
ADD_NEW_MSG_ITEM(op_heat_end_soon,   handle_heat_left10_bh)
ADD_NEW_MSG_ITEM(op_heat_finish,    handle_heat_finish_c)
ADD_NEW_MSG_ITEM(op_stop_heat,      handle_stop_heat)
ADD_NEW_MSG_ITEM(op_error_occur,  handle_error)
ADD_NEW_MSG_ITEM(op_hall_door_mode,  handle_hall_door_mode_heat)
ADD_NEW_MSG_ITEM(opt_cmd_disable_charge,  handle_cmd_disable_charge)
ADD_NEW_MSG_ITEM(opt_cmd_enable_charge,  handle_cmd_enable_charge)
ADD_NEW_MSG_ITEM(op_UI_updating,  handle_UI_updating)
ADD_NEW_MSG_ITEM(op_entry_cit,    handle_entry_cit_c)
END_MESSAGE_MAP

/*STATE_CHARGE*/
BEGIN_MESSAGE_MAP(state_charge)
ADD_NEW_MSG_ITEM(op_charge,      handle_charge_charge)
ADD_NEW_MSG_ITEM(op_entry_cit,   handle_entry_cit_c)
//ADD_NEW_MSG_ITEM(op_bat_left,    handle_bat_left_charge)
//ADD_NEW_MSG_ITEM(op_hall_door_mode,  handle_hall_door_mode_c)
ADD_NEW_MSG_ITEM(op_hall_door_mode,  handle_hall_door_mode_charge)
//ADD_NEW_MSG_ITEM(op_btn_short_down,    handle_button_short_down_charge)
ADD_NEW_MSG_ITEM(op_btn_short_up,       handle_button_short_up_charge)
ADD_NEW_MSG_ITEM(op_btn_1s5_down,  handle_button_1s5_down_charge)
//ADD_NEW_MSG_ITEM(op_btn_1s5_up,    handle_button_1s5_up_charge)
ADD_NEW_MSG_ITEM(op_btn_3short,    handle_button_3short_c)
//ADD_NEW_MSG_ITEM(op_btn_oneshort_onelong,    handle_show_stick_sensor)

//ADD_NEW_MSG_ITEM(op_btn_2s_down,  handle_button2s_charge)
//ADD_NEW_MSG_ITEM(op_btn_2s_up,    handle_button_2s_up_charge)
ADD_NEW_MSG_ITEM(op_charge_timeout,    handle_charge_timeout_charge)

ADD_NEW_MSG_ITEM(op_normal_heat,  handle_normal_heat)
ADD_NEW_MSG_ITEM(op_boost_heat,   handle_boost_heat)
ADD_NEW_MSG_ITEM(op_error_occur,  handle_error)
ADD_NEW_MSG_ITEM(opt_cmd_disable_charge,  handle_cmd_disable_charge)
ADD_NEW_MSG_ITEM(op_UI_updating,  handle_UI_updating)
END_MESSAGE_MAP

/*STATE_CIT*/
BEGIN_MESSAGE_MAP(state_cit)
ADD_NEW_MSG_ITEM(op_exit_cit,          handle_exit_cit_c)
ADD_NEW_MSG_ITEM(op_update_cit_timer,  handle_update_cit_timer)
ADD_NEW_MSG_ITEM(opt_cmd_disable_charge,  handle_cmd_disable_charge)
ADD_NEW_MSG_ITEM(opt_cmd_enable_charge,  handle_cmd_enable_charge)
END_MESSAGE_MAP

/*STATE_ERROR*/
BEGIN_MESSAGE_MAP(state_error)
ADD_NEW_MSG_ITEM(op_error_occur,  handle_error)
ADD_NEW_MSG_ITEM(op_error_clear,  handle_error)
ADD_NEW_MSG_ITEM(op_error_led_output_finish, handle_error_led_output_finish)
ADD_NEW_MSG_ITEM(op_charge,  handle_charge_error)
ADD_NEW_MSG_ITEM(op_hall_door_mode,  handle_error_ui_repeat_by_hall)
ADD_NEW_MSG_ITEM(op_btn_short_down,  handle_error_ui_repeat_by_btn)
ADD_NEW_MSG_ITEM(op_btn_1s5_down,  handle_error_ui_reset_continue)
ADD_NEW_MSG_ITEM(op_btn_1s5_up,  handle_error_ui_reset_oneshot)
//ADD_NEW_MSG_ITEM(op_btn_2s_down,  handle_error_ui_reset_continue)
//ADD_NEW_MSG_ITEM(op_btn_2s_up,  handle_error_ui_reset_oneshot)

END_MESSAGE_MAP

BEGIN_Register_State(trinity_manager)
ADD_Register_State(state_booting)
ADD_Register_State(state_idle)
ADD_Register_State(state_normal_heat)
ADD_Register_State(state_boost_heat)
ADD_Register_State(state_charge)
ADD_Register_State(state_cit)
ADD_Register_State(state_error)
END_Register_State



void manager_init(void)
{
    //init msg queue for manager
    uint8_t ret;
    ret = create_mngr_msg_queue();
    if(!ret){
        LOGE("create msgQueue failed");
    }
    else{
        LOGD("create msgQueue OK");
    }

    g_state = STATE_BOOTING;
//    mc.charge_type = 0;
    mc.pexit_cit_timer = NULL;
    //mc.pheat_timer = NULL;
    mc.pshutdown_timer = NULL;
    mc.shut_down_ms = AUTO_SHUT_DOWM_MS;
    mc.printf_timer = NULL;
    mc.record_when_err_timer = NULL;
    mc.comm_lock_timer = NULL;
//    mc.enable_sw_33_timer = NULL;
//    mc.batt_lv_disp_timer = NULL;
    mc.bat_left = app_get_bat_left();
    mc.hall_door_status = app_get_hall_door_status();
    mc.errorPos = 0xFF;
    mc.error_code = 0;
    mc.oled_lock_flag = 0;
    mc.error_exist_due_to_record = 0;
    start_printf_parameters_timer(IDLE_LOG);
    //TIMER_SAFE_RESET(mc.enable_sw_33_timer, 50, TIMER_OPT_ONESHOT, mcu_sw33_init, NULL);
    //LOGD("3.3 sw pin state: %d", HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3));
#ifdef SSCOM
    //TIMER_SAFE_RESET(mc.comm_lock_timer, 1000, TIMER_OPT_PERIOD, call_back_send_comm_lock_state, NULL);
    mc.comm_lock_timer = bat_timer_reset_ext(mc.comm_lock_timer, "comm_lock_timer", 1000, TIMER_OPT_PERIOD, call_back_send_comm_lock_state);
    bat_timer_start(mc.comm_lock_timer, portMAX_DELAY);
#endif

    LOGD("entry %s ...", g_state_name[g_state]);
}
void stop_comm_lock_timer(void)
{
    //TIMER_SAFE_DELETE(mc.pshutdown_timer);
    if(mc.comm_lock_timer){
        if(bat_timer_delete(mc.comm_lock_timer, portMAX_DELAY)==pdPASS){
            mc.comm_lock_timer = NULL;
        }
    }
}

/*************************************************************************************************
  * @brief    : manager task
  * @return   : None
*************************************************************************************************/
void manager_task(void)
{
    msg_st msg;
    /*get a message*/
    if(xQueueReceive(mngrQueueHandle, (void *)&msg, 0)){
        dispatch_message(GET_SMM_NAME(trinity_manager), &g_state, &msg);
    }
}

void test_idle_logs(void)
{
    LOGD("test idle log start");
    ilog.bat_temp = 40;
    ilog.usb_temp = 41;
    ilog.coil_temp = 42;
    ilog.coil_temp_b = 40;
    ilog.junc_temp = 42;
    ilog.tc1_temp = 43;
    ilog.tc2_temp = 43;
    ilog.vbus_v = 4.523;
    ilog.vbat_v = 4.323;
    ilog.i_sense = 0.223;
    ilog.bat_left = 96;
         
    send_idle_log_bytes((uint8_t*)&ilog, sizeof(idle_log_t));
    LOGD("test idle log end");
}






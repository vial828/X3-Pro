#include <string.h>
#include "stratos_defs.h"
#include "log.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "batTimer.h"
#include "app_oled_display.h"
#include "dev_oled_driver.h"
#include "gd25qxx.h"
#include "batTimer.h"
#include "kernel.h"
#include "app_oled_UI.h"
#include "app_heat.h"
#include "manager.h"
#include "app_charge.h"
#include "dev_adc.h"
#include "math.h"
#include "app_button.h"

#define DISPLAY_DURATION 4 //display 4s for battery or charge ui
typedef struct {
ptimer_t heat_ui_timer;
ptimer_t heat_count_timer;
uint8_t heat_state;
uint8_t fast_charge;
uint8_t brand_frame;
uint8_t reboot_frame;
uint8_t mode_select_frame;
uint8_t standard_flag;
uint8_t boost_flag;
uint8_t mode_heatup_frame;
uint8_t heat_up_frame;
uint8_t heat_cancel_flag;
uint8_t heat_up_cancel_frame;
uint8_t heatup_remove_frame;
uint8_t heatup_session_frame;
uint32_t session_timer_period;
uint8_t session_frame;
uint8_t extended_frame;
uint8_t enter_extend_frame;
uint8_t reverse_frame;
uint8_t session_extended_dis_flag;
uint8_t session_remove_frame;
uint8_t remove_frame;
uint8_t remove_mode_frame;
uint8_t battery_check_frame;
uint8_t charge_frame;
uint8_t charge_time_out_frame;
uint8_t cleaning_frame;
uint8_t stick_sensor_frame;
uint8_t error_reset_frame;
uint8_t closing_frame;
uint8_t ui_buffer1_48x112[16128];
uint8_t ui_buffer2_40x50[6000];
int16_t warm_up_count_down_num;
int16_t session_count_down_num;
int16_t cancel_count;
uint16_t mode_select;
uint16_t battery_level;
uint16_t stick_sensor_status;
uint8_t error_type;
uint8_t error_frame;
uint8_t over_voltage_frame;
uint8_t current_ui_name;
}display_context_st;
static ext_flash_record_t ext_flash_record_s;
static display_context_st dc;
extern uint32_t heat_all_time;
extern uint8_t flash_rx_buffer[76140];
uint8_t black_temp[38070]={0};
uint8_t certfct_graph_count;

static void remove_stick_ui(void);
//uint32_t battery_fill[16]={557,774,933,1058,1162,1249,1323,1386,1439,1483,1520,1550,1572,1587,1597,1600};
uint32_t battery_fill[12]={478,663,794,894,975,1039,1091,1131,1162,1183,1196,1200};

/*************************************************************************************************
  * @brief    :delete timer
  *param[in]  :none
  * @return   : none
*************************************************************************************************/
static void delete_ui_timer(void)
{
    if(dc.heat_count_timer){
        if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
            dc.heat_count_timer = NULL;
        }
    }
    if(dc.heat_ui_timer){
        if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
            dc.heat_ui_timer = NULL;
        }
    }
    oled_queue_reset();
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh black_clear frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_black_ui()
{
    start_oled_clear_black();
}

/*************************************************************************************************
  * @brief    : Get extFlash version data from ram
  * @return   : Pointer to bext_flash_record_s
*************************************************************************************************/
ext_flash_record_t * get_ext_flash_record_from_ram(void)
{
    return &ext_flash_record_s;
}

/*************************************************************************************************
  * @brief    : Get version data from ext_flash
  * @return   : Pointer to ext flash record
*************************************************************************************************/
ext_flash_record_t * get_version_record_from_ext_flash(void)
{
    memset(flash_rx_buffer, 0, 64);
    spi_flash_buffer_read(&flash_rx_buffer[0],VERSION_IMAGE_START_ADDR,64);
    memcpy((void*)&ext_flash_record_s, (char *)flash_rx_buffer,sizeof(ext_flash_record_t));
    return &ext_flash_record_s;
}
/*************************************************************************************************
    \brief      handle_oled_brand: to refresh the frames of welcome_brand
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_brand(msg_st* pmsg)
{
    if(dc.current_ui_name!=OPEN_UI)
    {
        return;
    }
    if(dc.brand_frame<22)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_START_ADDR+dc.brand_frame*11424,11424);
        dev_oled_set_region(10,118,77,173);
        dis_pic(&flash_rx_buffer[0],11424,0);
    }
    else if(dc.brand_frame<25)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_START_ADDR+0x3D5C0+(dc.brand_frame-22)*11424,11424);
        dev_oled_set_region(10,118,77,173);
        dis_pic(&flash_rx_buffer[0],11424,0);
    }
    else if(dc.brand_frame==25)
    {
        dev_oled_set_region(10,118,77,173);
        dis_pic(&black_temp[0],11424,0);
    }
    else if(dc.brand_frame<30)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_START_ADDR+0x45BA0+(dc.brand_frame-26)*4104,4104);
        dev_oled_set_region(26,118,61,155);
        dis_pic(&flash_rx_buffer[0],4104,0);
    }
    else if(dc.brand_frame<36)
    {
        //do nothing
    }
    else if(dc.brand_frame<46)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_START_ADDR+0x49BC0+6912*(dc.brand_frame-36),6912);
        dev_oled_set_region(26,94,61,157);
        dis_pic(&flash_rx_buffer[0],6912,0);
    }
    else if(dc.brand_frame==46)
    {
        if(app_get_hall_door_status() == door_base)
        {
            dc.mode_select=base_open;

            spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BASE_START_ADDR+(dc.brand_frame-46)*9408,9408);
            dev_oled_set_region(26,34,57,131);
            dis_pic(&flash_rx_buffer[0],9408,0);
        }
        else if(app_get_hall_door_status() == door_boost)
        {
            dc.mode_select=boost_open;
            dev_oled_set_region(26,122,61,157);
            dis_pic(&black_temp[0],3888,0);
            spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BOOST_START_ADDR+(dc.brand_frame-46)*10800,10800);
            dev_oled_set_region(26,22,61,121);
            dis_pic(&flash_rx_buffer[0],10800,0);
        }
        else
        {
            dc.brand_frame=53;
            LOGD("ui end:open AS door close/err");
            return;
        }
    }
    else if(dc.brand_frame<52)
    {
        switch(dc.mode_select)
        {
            case base_open:
            if(dc.brand_frame==47)
            {
            dev_oled_set_region(26,34,57,131);
            dis_pic(&black_temp[0],9408,0);//icon
            }
            spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BASE_START_ADDR+(dc.brand_frame-46)*9408,9408);
            dev_oled_set_region(26,34,57,113);
            dis_pic(&flash_rx_buffer[0],7680,0);//icon
            spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BASE_START_ADDR+0xDC80+(dc.brand_frame-47)*7680,7680);
            dev_oled_set_region(34,114,53,241);
            dis_pic(&flash_rx_buffer[0],7680,0);
            break;
            case boost_open:
            spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BOOST_START_ADDR+(dc.brand_frame-46)*10800,10800);
            dev_oled_set_region(26,22,61,121);
            dis_pic(&flash_rx_buffer[0],10800,0);//icon
            spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BOOST_START_ADDR+0xFD20+(dc.brand_frame-47)*5412,5412);
            dev_oled_set_region(34,136,55,217);
            dis_pic(&flash_rx_buffer[0],5412,0);
            break;
            default:
            break;
        }
    }
    else if(dc.brand_frame==52)
    {
        if(dc.mode_select == base_open)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_MODE_START_ADDR,19968);
            dev_oled_set_region(26,34,57,241);
            dis_pic(&flash_rx_buffer[0],19968,0);
            dc.standard_flag=1;
            LOGD("ui end:open");
        }
        else if(dc.mode_select == boost_open)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_MODE_START_ADDR,21168);
            dev_oled_set_region(26,22,61,217);
            dis_pic(&flash_rx_buffer[0],21168,0);
            dc.boost_flag=1;
            LOGD("ui end:open");
        }
    }
    else if(dc.brand_frame==(52+5000/FPS_25))
    {
        lcd_clear(&black_temp[0]);
        dc.boost_flag=0;
        dc.standard_flag=0;
    }
    dc.brand_frame++;
}



/*************************************************************************************************
  * @brief    : Timer call back function to refresh welcome_brand frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_brand_ui(const ptimer_t tm){
    if(dc.brand_frame==53)
    {
        clear_all_oled_lock();
        if(app_get_hall_door_status() == door_base &&dc.boost_flag==1)
        {
            start_oled_display_mode_select(boost2base);
            return;
        }
        else if(app_get_hall_door_status() == door_boost&&dc.standard_flag==1)
        {
            start_oled_display_mode_select(base2boost);
            return;
        }else if(app_get_hall_door_status() != door_boost&&app_get_hall_door_status() != door_base)
        {
            if(dc.heat_ui_timer){
                if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                    dc.heat_ui_timer = NULL;
                }
            }
            post_msg_to_oled(op_oled_clear_black);
            return;
        }
    }
    else if(dc.brand_frame>(52+5000/FPS_25))
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        return;
    }
    post_msg_to_oled(op_oled_brand);
}
/*************************************************************************************************
    \brief      start oled display for welcome_brand animation
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_oled_welcome_brand(void)
{
    certfct_graph_count = 0;
    dc.current_ui_name=OPEN_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.brand_frame=0;
    if(dc.heat_ui_timer)
    {
        delete_ui_timer();
        post_msg_to_oled(op_oled_clear_black);
    }
    else
    {
        post_msg_to_oled(op_oled_brand);
    }
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (FPS_25), TIMER_OPT_PERIOD, call_back_brand_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    LOGD("ui start:open");
}

/*************************************************************************************************
    \brief      handle_oled_reboot: to refresh the frames of reboot ui
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_reboot(msg_st* pmsg)
{
    if(dc.current_ui_name!=REBOOT_UI)
    {
        return;
    }
    if(dc.reboot_frame<20)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REBOOT_START_ADDR+8100*dc.reboot_frame,8100);
        dev_oled_set_region(22,114,75,165);
        dis_pic(&flash_rx_buffer[0],8100,0);
    }
    else if(dc.reboot_frame<35)
    {
        //do nothing
    }
    else if(dc.reboot_frame<39)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REBOOT_START_ADDR+0x278D0+8100*(dc.reboot_frame-35),8100);
        dev_oled_set_region(22,114,75,165);
        dis_pic(&flash_rx_buffer[0],8100,0);
    }else if(dc.reboot_frame==39)
    {
        dev_oled_set_region(22,114,75,165);
        dis_pic(&black_temp[0],8100,0);
        LOGD("ui end:reboot");
    }
    else if(dc.reboot_frame==40)
    {
        if(app_get_hall_door_status() == door_base)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_MODE_START_ADDR,19968);
            dev_oled_set_region(26,34,57,241);
            dis_pic(&flash_rx_buffer[0],19968,0);
            dc.standard_flag=1;
        }
        else if(app_get_hall_door_status() == door_boost)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_MODE_START_ADDR,21168);
            dev_oled_set_region(26,22,61,217);
            dis_pic(&flash_rx_buffer[0],21168,0);
            dc.boost_flag=1;
        }
    }
    dc.reboot_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh welcome_brand frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_reboot_ui(const ptimer_t tm){
    if(dc.reboot_frame>(40+5000/FPS_25))
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        post_msg_to_oled(op_oled_clear_black);
        return;
    }
    post_msg_to_oled(op_oled_reboot);
}
/*************************************************************************************************
    \brief      start oled display for welcome_brand animation
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_oled_reboot_ui(void)
{
    certfct_graph_count = 0;
    dc.current_ui_name=REBOOT_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.brand_frame=0;
    if(dc.heat_ui_timer)
    {
        delete_ui_timer();
        post_msg_to_oled(op_oled_clear_black);
    }
    else
    {
        post_msg_to_oled(op_oled_reboot);
    }
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (FPS_25), TIMER_OPT_PERIOD, call_back_reboot_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    LOGD("ui start:reboot");
}
/*************************************************************************************************
    \brief      handle_oled_door_mode: to refresh the frames of hall_door_mode
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_door_mode(msg_st* pmsg)
{//base2boost,boost2base,base2close
    if(dc.current_ui_name!=MODE_SELECT_UI)
    {
        return;
    }
    if(dc.mode_select == base2boost){
        if(dc.mode_select_frame<6){
            spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_TO_BOOST_MODE_START_ADDR+dc.mode_select_frame*6324,6324);
            dev_oled_set_region(28,22,61,83);
            dis_pic(&flash_rx_buffer[0],6324,0);
            spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_TO_BOOST_MODE_START_ADDR+0x9438+dc.mode_select_frame*8448,8448);
            dev_oled_set_region(34,114,55,241);
            dis_pic(&flash_rx_buffer[0],8448,0);
            if(dc.mode_select_frame==5)
            {
                dc.boost_flag=1;
                dc.standard_flag=0;
            }
        }
        else if(dc.mode_select_frame==5*1000/FPS_25)
        {
            lcd_clear(&black_temp[0]);
            dc.boost_flag=0;
            LOGD("ui end:base2boost");
        }
    }
    else if(dc.mode_select == boost2base){
        if(dc.mode_select_frame<6){
            spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_TO_BOOST_MODE_START_ADDR+(5-dc.mode_select_frame)*6324,6324);
            dev_oled_set_region(28,22,61,83);
            dis_pic(&flash_rx_buffer[0],6324,0);
            spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_TO_STANDARD_MODE_START_ADDR+0x9438+(5-dc.mode_select_frame)*8448,8448);
            dev_oled_set_region(34,114,55,241);
            dis_pic(&flash_rx_buffer[0],8448,0);
            if(dc.mode_select_frame==5)
            {
                dc.boost_flag=0;
                dc.standard_flag=1;
            }
        }
        else if(dc.mode_select_frame==5*1000/FPS_25)
        {
            lcd_clear(&black_temp[0]);
            dc.standard_flag=0;
            LOGD("ui end:boost2base");
        }
    }
    else if(dc.mode_select == base2close)
    {
        dc.standard_flag=0;
        dc.boost_flag=0;
        if(dc.mode_select_frame<2){
            //word
            spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0x2DB4+dc.mode_select_frame*8448,8448);
            dev_oled_set_region(34,114,55,241);
            dis_pic(&flash_rx_buffer[0],8448,0);
        }
        else if(dc.mode_select_frame<4)
        {
            //word
            spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0x2DB4+dc.mode_select_frame*8448,8448);
            dev_oled_set_region(34,114,55,241);
            dis_pic(&flash_rx_buffer[0],8448,0);
            //icon
            spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+(dc.mode_select_frame-2)*3900,3900);
            dev_oled_set_region(32,34,57,83);
            dis_pic(&flash_rx_buffer[0],3900,0);
        }
        else if(dc.mode_select_frame==4)
        {
            //icon
            spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+(dc.mode_select_frame-2)*3900,3900);
            dev_oled_set_region(32,34,57,83);
            dis_pic(&flash_rx_buffer[0],3900,0);
            dev_oled_set_region(34,114,55,241);
            dis_pic(&black_temp[0],8448,0);
        }
        else if(dc.mode_select_frame<9)
        {
            if(dc.mode_select_frame==5)
            {
                dev_oled_set_region(32,34,57,83);
                dis_pic(&black_temp[0],3900,0);
            }
            spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0xB1B4+(dc.mode_select_frame-5)*13800,13800);
            dev_oled_set_region(4,66,53,157);
            dis_pic(&flash_rx_buffer[0],13800,0);
        }
        else if (dc.mode_select_frame<26){
            if(dc.mode_select_frame==9)
            {
                dev_oled_set_region(4,66,53,119);
                dis_pic(&black_temp[0],8100,0);
            }
            spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0x1E294+(dc.mode_select_frame-9)*11316,11316);
            dev_oled_set_region(4,120,85,165);
            dis_pic(&flash_rx_buffer[0],11316,0);
        }
        else if(dc.mode_select_frame<31)
        {
            //do nothing
        }
        else if(dc.mode_select_frame<33)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0x1E294+0x2EF74+(dc.mode_select_frame-31)*11316,11316);
            dev_oled_set_region(4,120,85,165);
            dis_pic(&flash_rx_buffer[0],11316,0);
        }
        else if(dc.mode_select_frame==33){
            dev_oled_set_region(4,120,85,165);
            dis_pic(&black_temp[0],11316,0);
            LOGD("ui end:base2close");
        }
    }
    else if(dc.mode_select==base_open)
    {
        if(dc.mode_select_frame==0){
            spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_MODE_START_ADDR,19968);
            dev_oled_set_region(26,34,57,241);
            dis_pic(&flash_rx_buffer[0],19968,0);
            dc.standard_flag=1;
        }else if(dc.mode_select_frame==(5000/FPS_25))
        {
            lcd_clear(&black_temp[0]);
            dc.standard_flag=0;
        }
    }
    else if(dc.mode_select==boost_open)
    {
        if(dc.mode_select_frame==0){
            spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_MODE_START_ADDR,21168);
            dev_oled_set_region(26,22,61,217);
            dis_pic(&flash_rx_buffer[0],21168,0);
            dc.boost_flag=1;
        }else if(dc.mode_select_frame==(5000/FPS_25))
        {
            lcd_clear(&black_temp[0]);
            dc.boost_flag=0;
        }
    }
    dc.mode_select_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh hall_door_mode frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_door_mode_ui(const ptimer_t tm){
    if(dc.mode_select==base2close&&dc.mode_select_frame>33)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        clear_all_oled_lock();
        return;
    }
    else if(dc.mode_select_frame>5*1000/FPS_25)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        return;
    }
    post_msg_to_oled(op_hall_door_mode);
}


/*************************************************************************************************
    \brief      start oled display for battery_check
    \param[in]  battery_soc: specify the bat_left(00 - 100)
    \retval     None
*************************************************************************************************/
void start_oled_display_mode_select(uint8_t mode_select)
{
    dc.current_ui_name=MODE_SELECT_UI;
    dc.mode_select = mode_select;
    dc.mode_select_frame=0;
    if(dc.heat_ui_timer)
    {
        delete_ui_timer();
        if(dc.mode_select==base2close&&dc.standard_flag==0)
        {
            start_no_mode_closing_animation();
            return;
        }
        if(dc.standard_flag==1&&(dc.mode_select==base2boost||dc.mode_select==base2close))
        {
            post_msg_to_oled(op_hall_door_mode);
        }else if(dc.boost_flag==1&&dc.mode_select==boost2base)
        {
            post_msg_to_oled(op_hall_door_mode);
        }else
        {
            post_msg_to_oled(op_oled_clear_black);
        }
    }else{
        if(dc.mode_select==base2close)
        {
            start_no_mode_closing_animation();
            return;
        }else{
            post_msg_to_oled(op_oled_clear_black);
        }
    }
    if(dc.mode_select==base2close)
    {
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (FPS_15), TIMER_OPT_PERIOD, call_back_door_mode_ui);
    }else{
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (FPS_25), TIMER_OPT_PERIOD, call_back_door_mode_ui);
    }
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    if(dc.mode_select == base2boost){
        LOGD("ui start:base2boost");
    }else if(dc.mode_select == boost2base){
        LOGD("ui start:boost2base");
    }else if(dc.mode_select == base2close){
        LOGD("ui start:base2close");
    }
}
static void heat_up_ui(void)
{
    if(dc.current_ui_name!=HEAT_UP_UI)
    {
        return;
    }
    if(dc.heat_up_frame==0)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],WARM_UP_START_ADDR,8316);
        dev_oled_set_region(36,36,53,189);
        dis_pic(&flash_rx_buffer[0],8316,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],WARM_UP_START_ADDR+0x4260,180);
        memcpy(&dc.ui_buffer1_48x112[0],&flash_rx_buffer[0],180);
        memcpy(&dc.ui_buffer1_48x112[180],&dc.ui_buffer1_48x112[150],30);
        memcpy(&dc.ui_buffer1_48x112[210],&dc.ui_buffer1_48x112[150],30);
    }
    if( (dc.heat_up_frame>40) && (dc.heat_up_frame<134))
    {
        for(int j=10;j<80;j++)
        {
            if(dc.ui_buffer1_48x112[3*j]!=0&&dc.ui_buffer1_48x112[3*j+1]!=0)
                dc.ui_buffer1_48x112[3*j+1]-=0x01;
        }
    }
    if(dc.heat_up_frame<134)
    {
        if(dc.heat_up_frame%2==0)
        {
            dev_oled_set_region(40,174-dc.heat_up_frame,49,179-dc.heat_up_frame);
            dis_pic(&dc.ui_buffer1_48x112[0],180,30);
        }
        else
        {
            dev_oled_set_region(40,174-dc.heat_up_frame-1,49,179-dc.heat_up_frame+1);
            dis_pic(&dc.ui_buffer1_48x112[0],240,0);
        }
    }
    if(dc.heat_up_frame==133)
    {
        LOGD("ui end:warm_up");
    }
    dc.heat_up_frame++;
}
/*************************************************************************************************
    \brief      handle_oled_heat_up: to refresh the frames of warm_up
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_heat_up(msg_st* pmsg){
    heat_up_ui();
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh warm up frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_warm_up_ui(const ptimer_t tm){
    if(dc.heat_up_frame>133)
    {
        if(dc.heat_count_timer){
            if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
                dc.heat_count_timer = NULL;
           }
        }
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        if(dc.heat_state == BASE_WARM_UP){
            post_msg_to_manager(op_heat_warmup_finish_nh);
        }else if(dc.heat_state == BOOST_WARM_UP){
            post_msg_to_manager(op_heat_warmup_finish_bh);
        }
        return;
    }
    post_msg_to_oled(op_oled_warm_up);
}
static void session_ui(void)
{
    if(dc.current_ui_name!=SESSION_UI)
    {
        return;
    }
    if(dc.session_count_down_num!=0&&dc.heat_cancel_flag==1)
    {
        dev_oled_set_region(28,40,63,61);
        dis_pic(&black_temp[0],2376,0);
    }
    if(dc.session_frame==0)
    {
        memset(&dc.ui_buffer1_48x112[0],0x00,16128);
        memset(&dc.ui_buffer2_40x50[0],0x00,6000);
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR,42108);
        dev_oled_set_region(16,20,73,261);
        dis_pic(&flash_rx_buffer[0],42108,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2,3750);
        memcpy(&dc.ui_buffer1_48x112[150],&flash_rx_buffer[0],3750);
        memcpy(&dc.ui_buffer2_40x50[150],&flash_rx_buffer[0],3750);
        memcpy(&dc.ui_buffer1_48x112[3900],&dc.ui_buffer1_48x112[3750],150);
        memcpy(&dc.ui_buffer1_48x112[4050],&dc.ui_buffer1_48x112[3750],150);
        memcpy(&dc.ui_buffer2_40x50[3900],&dc.ui_buffer2_40x50[3750],150);
        memcpy(&dc.ui_buffer2_40x50[4050],&dc.ui_buffer2_40x50[3750],150);
    }
    if(dc.session_frame<27)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+0xA47C,42108);
        for(int t=0;(dc.session_frame+t)<26;t++)
        {
            for(int j=0;j<50;j++)
            {
                if(dc.ui_buffer2_40x50[50*3*t+3*j]==0x00&&dc.ui_buffer2_40x50[50*3*t+3*j+1]==0x00)
                {
                    dc.ui_buffer1_48x112[50*3*t+3*j]=flash_rx_buffer[58*3*(3+dc.session_frame+t)+12+3*j];
                    dc.ui_buffer1_48x112[50*3*t+3*j+1]=flash_rx_buffer[58*3*(3+dc.session_frame+t)+12+3*j+1];
                    dc.ui_buffer1_48x112[50*3*t+3*j+2]=flash_rx_buffer[58*3*(3+dc.session_frame+t)+12+3*j+2];
                }
            }
        }
    }
    if(dc.session_frame<182)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2,3750);
        memcpy(&dc.ui_buffer2_40x50[150],&flash_rx_buffer[0],3750);
        memcpy(&dc.ui_buffer2_40x50[3900],&dc.ui_buffer2_40x50[3750],150);
        memcpy(&dc.ui_buffer2_40x50[4050],&dc.ui_buffer2_40x50[3750],150);
        for(int j=100;j<1400;j++)
        {
            if(dc.ui_buffer2_40x50[3*j]!=0&&dc.ui_buffer2_40x50[3*j+1]!=0)
            {
                if(dc.session_frame%61==0)
                    dc.ui_buffer1_48x112[3*j]-=0x01;//R
                if(dc.session_frame%3==0)
                    dc.ui_buffer1_48x112[3*j+1]+=0x01;//G
                if(dc.session_frame%16==0)
                    dc.ui_buffer1_48x112[3*j+2]+=0x01;//B
            }
        }
        if(dc.session_frame%2==0)
        {
            dev_oled_set_region(20,24+dc.session_frame,69,49+dc.session_frame);
            dis_pic(&dc.ui_buffer1_48x112[0],3900,150);
        }
        else
        {
            dev_oled_set_region(20,23+dc.session_frame,69,50+dc.session_frame);
            dis_pic(&dc.ui_buffer1_48x112[0],4200,0);
        }
    }
    else if(dc.session_frame<212)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2+3750 + (dc.session_frame-182)*7800,7800);
        dev_oled_set_region(20,206,69,257);
        dis_pic(&flash_rx_buffer[0],7800,0);
        if(dc.session_frame==211)
        {
            LOGD("ui end:session");
        }
    }
    dc.session_frame++;
}
/*************************************************************************************************
    \brief      handle_oled_session: to refresh the frames of session
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_session(msg_st* pmsg){
    session_ui();
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh session  frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_session_ui(const ptimer_t tm){
    if (dc.session_frame>211){
        if(dc.heat_count_timer){
            if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
                dc.heat_count_timer = NULL;
            }
        }
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        return;
    }
    post_msg_to_oled(op_oled_session);
 }
/*************************************************************************************************
    \brief      handle_oled_warm_up_count_down: to refresh the frames of warm_up_count_down
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_warm_up_count_down(msg_st* pmsg){
    int warm_up_count_down_num_temp =0;
    if (dc.heat_state == BASE_WARM_UP){
        warm_up_count_down_num_temp = ((NH_WARMUP_FINISH_TIME/1000)) -dc.warm_up_count_down_num;
    }else if (dc.heat_state == BOOST_WARM_UP){
        warm_up_count_down_num_temp = ((BH_WARMUP_FINISH_TIME/1000)) -dc.warm_up_count_down_num;
    }
    dc.warm_up_count_down_num++;
    if (warm_up_count_down_num_temp>(-1)){
        if(warm_up_count_down_num_temp/10==0)
        {
            if(warm_up_count_down_num_temp%10==9){
                dev_oled_set_region(28,226,63,247);
                dis_pic(&black_temp[0],2376,0);
            }
            dev_oled_set_region(36,226,53,247);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+(warm_up_count_down_num_temp%10)*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
            dis_pic(&flash_rx_buffer[0],1188,0);
        }
        else{
            if(warm_up_count_down_num_temp/10==1)
            {
                if(warm_up_count_down_num_temp%10==9){
                    dev_oled_set_region(28,226,63,247);
                    dis_pic(&black_temp[0],2376,0);
                }
                dev_oled_set_region(24,226,41,247);
                spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+10*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
                dis_pic(&flash_rx_buffer[0],1188,0);
                dev_oled_set_region(42,226,59,247);
                spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+(warm_up_count_down_num_temp%10)*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
                dis_pic(&flash_rx_buffer[0],1188,0);
            }
            else
            {
                dev_oled_set_region(28,226,45,247);
                spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+(warm_up_count_down_num_temp/10)*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
                dis_pic(&flash_rx_buffer[0],1188,0);
                dev_oled_set_region(46,226,63,247);
                spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+(warm_up_count_down_num_temp%10)*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
                dis_pic(&flash_rx_buffer[0],1188,0);
           }
        }
    }
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh warm up count down frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_warm_up_count_down_ui(const ptimer_t tm){
    if(dc.heat_state==BASE_WARM_UP&&dc.warm_up_count_down_num>19)
    {
        if(dc.heat_count_timer){
            if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
                dc.heat_count_timer = NULL;
            }
        }
        return;
    }
    if(dc.heat_state==BOOST_WARM_UP&&dc.warm_up_count_down_num>14)
    {
        if(dc.heat_count_timer){
            if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
                dc.heat_count_timer = NULL;
            }
        }
        return;
    }
    post_msg_to_oled(op_oled_warm_up_count_down);
}
/*************************************************************************************************
    \brief      handle_oled_warm_up_count_down: to refresh the frames of session_count_down
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_session_count_down(msg_st* pmsg){
    int8_t session_count_down_num_temp = 10 - dc.session_count_down_num;
    if (session_count_down_num_temp > (-1)){
        if(session_count_down_num_temp/10==0)
        {
            if(session_count_down_num_temp%10==9){
                dev_oled_set_region(28,40,63,61);
                dis_pic(&black_temp[0],2376,0);
            }
            dev_oled_set_region(36,40,53,61);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+(session_count_down_num_temp%10)*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
            dis_pic(&flash_rx_buffer[0],1188,0);
        }
        else{
            dev_oled_set_region(28,40,45,61);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+session_count_down_num_temp/10*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
            dis_pic(&flash_rx_buffer[0],1188,0);
            dev_oled_set_region(46,40,63,61);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+(session_count_down_num_temp%10)*1188*6+1188*5,1188);//gImage_session_bg_withoutcolor
            dis_pic(&flash_rx_buffer[0],1188,0);
        }
        dc.session_count_down_num++;
    }
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh session count down frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_session_count_down_ui(const ptimer_t tm){
    post_msg_to_oled(op_oled_session_count_down);
}
/*************************************************************************************************
    \brief      start oled display for session_count_down
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_oled_session_count_down(void){
    dc.standard_flag=0;
    dc.boost_flag=0;
    if(dc.heat_count_timer){
        if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
            dc.heat_count_timer = NULL;
        }
    }
    dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", (1000), TIMER_OPT_PERIOD, call_back_session_count_down_ui);
    bat_timer_start(dc.heat_count_timer, portMAX_DELAY);
    post_msg_to_oled(op_oled_session_count_down);
}
/*************************************************************************************************
    \brief      start warm_up_ui after finish mode_to_heat_up animation
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_oled_warm_up_ui(void)
{
    if(dc.current_ui_name!=MODE_HEAT_UI)
    {
        return;
    }
    dc.current_ui_name=HEAT_UP_UI;
    dc.heat_up_frame=0;
    dc.warm_up_count_down_num=0;
    if(dc.heat_state==BASE_WARM_UP)
    {
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (NH_WARMUP_FINISH_TIME-40*16)/133, TIMER_OPT_PERIOD, call_back_warm_up_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);//ui
        dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", (1000), TIMER_OPT_PERIOD, call_back_warm_up_count_down_ui);
        bat_timer_start(dc.heat_count_timer, portMAX_DELAY);//num count down
        post_msg_to_oled(op_oled_warm_up_count_down);
    }
    else if(dc.heat_state==BOOST_WARM_UP)
    {
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (BH_WARMUP_FINISH_TIME-40*16)/133, TIMER_OPT_PERIOD, call_back_warm_up_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);//ui
        dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", (1000), TIMER_OPT_PERIOD, call_back_warm_up_count_down_ui);
        bat_timer_start(dc.heat_count_timer, portMAX_DELAY);//num count down
        post_msg_to_oled(op_oled_warm_up_count_down);
    }
    post_msg_to_oled(op_oled_warm_up);
    LOGD("ui start:warm_up");
}

/*************************************************************************************************
    \brief      handel_transition_mode_to_heatup: to refresh the frames of mode_to_warm_up
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handel_transition_mode_to_heatup(msg_st* pmsg)
{
    if(dc.current_ui_name!=MODE_HEAT_UI)
    {
        return;
    }
    if(dc.mode_heatup_frame<6)
    {
        switch(dc.heat_state)
        {
            case BASE_WARM_UP:
                spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_TO_HEAT_UP_START_ADDR+dc.mode_heatup_frame*15120,15120);
                dev_oled_set_region(34,32,57,241);
                dis_pic(&flash_rx_buffer[0],15120,0);
            break;
            case BOOST_WARM_UP:
                dev_oled_set_region(26,22,61,27);
                dis_pic(&black_temp[0],1584,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_TO_HEAT_UP_START_ADDR+dc.mode_heatup_frame*19380,19380);
                dev_oled_set_region(28,28,61,217);
                dis_pic(&flash_rx_buffer[0],19380,0);
            break;
            default:
            break;
        }
    }
    else if(dc.mode_heatup_frame<11)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],MODE_WARM_UP_START_ADDR+(dc.mode_heatup_frame-6)*15120,15120);
        dev_oled_set_region(34,32,57,241);
        dis_pic(&flash_rx_buffer[0],15120,0);
    }
    else if(dc.mode_heatup_frame<16)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],MODE_WARM_UP_START_ADDR+0x12750+(dc.mode_heatup_frame-11)*8316,8316);
        dev_oled_set_region(36,36,53,189);
        dis_pic(&flash_rx_buffer[0],8316,0);
        if(dc.heat_state==BASE_WARM_UP)
        {
            dev_oled_set_region(28,226,45,247);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+((NH_WARMUP_FINISH_TIME/1000)/10)*1188*6+1188*(dc.mode_heatup_frame-11),1188);
            dis_pic(&flash_rx_buffer[0],1188,0);
            dev_oled_set_region(46,226,63,247);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+((NH_WARMUP_FINISH_TIME/1000)%10)*1188*6+1188*(dc.mode_heatup_frame-11),1188);
            dis_pic(&flash_rx_buffer[0],1188,0);
            if(dc.mode_heatup_frame==15)
            {
                LOGD("ui end:base2warm_up");
            }
        }else if(dc.heat_state==BOOST_WARM_UP)
        {
            dev_oled_set_region(24,226,41,247);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+10*1188*6+1188*(dc.mode_heatup_frame-11),1188);
            dis_pic(&flash_rx_buffer[0],1188,0);
            dev_oled_set_region(42,226,59,247);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+((BH_WARMUP_FINISH_TIME/1000)%10)*1188*6+1188*(dc.mode_heatup_frame-11),1188);
            dis_pic(&flash_rx_buffer[0],1188,0);
            if(dc.mode_heatup_frame==15)
            {
                LOGD("ui end:boost2warm_up");
            }
        }
    }
    dc.mode_heatup_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh mode to heat up frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_transition_mode_to_heatupui(const ptimer_t tm)
{
    if(dc.mode_heatup_frame>15)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        if(dc.heat_cancel_flag==0)
        {
            post_msg_to_manager(op_oled_warm_up);
        }else if(dc.heat_cancel_flag==1)
        {
            dc.current_ui_name=NO_UI;
            dc.heat_up_frame=0;
            post_msg_to_manager(op_oled_cancel_heatup);
        }
        return;
    }
    post_msg_to_oled(op_oled_mode_to_heatup);
}

void start_oled_session_ui(void)
{
    if(dc.current_ui_name!=HEAT_SESSION_UI)
    {
        return;
    }
    dc.current_ui_name=SESSION_UI;
    dc.session_frame=0;
    dc.session_count_down_num=0;
    if(dc.heat_state==BASE_SESSION)
    {
        dc.session_timer_period=((heat_all_time-NH_WARMUP_FINISH_TIME-66*(16))/211);
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", ((heat_all_time-NH_WARMUP_FINISH_TIME-66*15)/211), TIMER_OPT_PERIOD, call_back_session_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);//ui
    }
    else if(dc.heat_state==BOOST_SESSION)
    {
        dc.session_timer_period=(heat_all_time-BH_WARMUP_FINISH_TIME-66*(16))/211;
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (heat_all_time-BH_WARMUP_FINISH_TIME-66*15)/211, TIMER_OPT_PERIOD, call_back_session_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);//ui
    }
    post_msg_to_oled(op_oled_session);
    LOGD("ui start:session");
}
/*************************************************************************************************
    \brief      handle_transition_heatup_to_session: to refresh the frames of warm_up_to_session
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_transition_heatup_to_session(msg_st* pmsg)
{
    if(dc.current_ui_name!=HEAT_SESSION_UI)
    {
        return;
    }
    if(dc.heatup_session_frame<5)
    {
        dev_oled_set_region(36,226,53,247);
        spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+1*1188*6+1188*(4-dc.heatup_session_frame),1188);
        dis_pic(&flash_rx_buffer[0],1188,0);
    }
    else if(dc.heatup_session_frame==5)
    {
        dev_oled_set_region(36,226,53,247);
        dis_pic(&black_temp[0],1188,0);
    }
    else if(dc.heatup_session_frame==6)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR,8316);
        dev_oled_set_region(36,36,53,189);
        dis_pic(&flash_rx_buffer[0],8316,0);
    }
    else if(dc.heatup_session_frame==7)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x207C,13572);
        dev_oled_set_region(32,32,57,205);
        dis_pic(&flash_rx_buffer[0],13572,0);
    }
    else if(dc.heatup_session_frame==8)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x5580,19176);
        dev_oled_set_region(28,30,61,217);
        dis_pic(&flash_rx_buffer[0],19176,0);
    }
    else if(dc.heatup_session_frame==9)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0xA068,22800);
        dev_oled_set_region(26,28,63,227);
        dis_pic(&flash_rx_buffer[0],22800,0);
    }
    else if(dc.heatup_session_frame==10)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0xF978,26460);
        dev_oled_set_region(24,26,65,235);
        dis_pic(&flash_rx_buffer[0],26460,0);
    }
    else if(dc.heatup_session_frame==11)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x160D4,30084);
        dev_oled_set_region(22,24,67,241);
        dis_pic(&flash_rx_buffer[0],30084,0);
    }
    else if(dc.heatup_session_frame==12)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x1D658,33600);
        dev_oled_set_region(20,24,69,247);
        dis_pic(&flash_rx_buffer[0],33600,0);
    }
    else if(dc.heatup_session_frame==13)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x25998,37260);
        dev_oled_set_region(18,22,71,251);
        dis_pic(&flash_rx_buffer[0],37260,0);
    }
    else if(dc.heatup_session_frame==14)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x2EB24,37908);
        dev_oled_set_region(18,22,71,255);
        dis_pic(&flash_rx_buffer[0],37908,0);
    }
    else if(dc.heatup_session_frame==15)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x37F38,38232);
        dev_oled_set_region(18,22,71,257);
        dis_pic(&flash_rx_buffer[0],38232,0);
    }
    else if(dc.heatup_session_frame==16)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x41490,41760);
        dev_oled_set_region(16,20,73,259);
        dis_pic(&flash_rx_buffer[0],41760,0);
        LOGD("ui end:warm_up2session");
    }
//        else if(dc.heatup_session_frame==17)
//        {
//            spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_HEAT_UP_TO_SESSION_START_ADDR+0x4B7B0,42108);
//            dev_oled_set_region(16,20,73,261);
//            dis_pic(&flash_rx_buffer[0],42108,0);
//        }
    dc.heatup_session_frame++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh heat up to session frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_transition_heatup_to_session(const ptimer_t tm)
{
    if(dc.heatup_session_frame>16)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        if(dc.heat_cancel_flag==1)
        {
            dc.current_ui_name=NO_UI;
            dc.session_frame=0;
            post_msg_to_manager(op_oled_cancel_session);
        }
        else
        {
            post_msg_to_manager(op_oled_session);
        }
        return;
    }
    post_msg_to_oled(op_oled_heatup_session);
}
/*************************************************************************************************
    \brief      handle_display_session_extended_ui: to display session_extended ui or not to display
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_display_session_extended_ui(msg_st* pmsg)
{
    if(dc.session_extended_dis_flag==1)//display session extended to select
    {
        if(dc.extended_frame<5)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_EXTENDED_START_ADDR+(4-dc.extended_frame)*16704,16704);
            dev_oled_set_region(16,20,73,115);
            dis_pic(&flash_rx_buffer[0],16704,0);
            if(dc.extended_frame==4)
            {
                LOGD("extend ui");
            }
        }
    }
    else if(dc.session_extended_dis_flag==0)//last 10s cancel display session extended to select or select extended
    {
        if(dc.extended_frame<5)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_EXTENDED_START_ADDR+dc.extended_frame*16704,16704);
            dev_oled_set_region(16,20,73,115);
            dis_pic(&flash_rx_buffer[0],16704,0);
        }
    }
    dc.extended_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh cancel display session extended ui op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_enter_extended_ui(const ptimer_t tm)
{
    if(dc.enter_extend_frame>7)
    {
        if(dc.session_frame>dc.reverse_frame){
            if((dc.session_frame-dc.reverse_frame)<211){
                dc.session_timer_period=(heat_all_time-NH_WARMUP_FINISH_TIME-66*16-40*6-dc.session_timer_period*dc.session_frame)/(211-(dc.session_frame-dc.reverse_frame));
                dc.session_frame=dc.session_frame-dc.reverse_frame;
            }
        }else{
            dc.session_timer_period=(heat_all_time-NH_WARMUP_FINISH_TIME-66*16-40*6-dc.session_timer_period*dc.session_frame)/211;
            dc.session_frame=0;
        }
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", dc.session_timer_period, TIMER_OPT_PERIOD, call_back_session_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
        post_msg_to_oled(op_oled_session);
        LOGD("enter extended");
        return;
    }
    post_msg_to_oled(op_oled_enter_extended);
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh cancel display session extended ui op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_session_extended_ui(const ptimer_t tm)
{
    if(dc.extended_frame>4)
    {
        if(dc.heat_count_timer){
            if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
                dc.heat_count_timer = NULL;
            }
        }
        if(dc.session_extended_dis_flag==0)
        {
            if(dc.heat_cancel_flag == 1)
            {
                post_msg_to_manager(op_oled_cancel_session);
            }else if(dc.heat_cancel_flag==0)
            {
                dc.enter_extend_frame=1;
                dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_enter_extended_ui);
                bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
                post_msg_to_oled(op_oled_enter_extended);
            }
        }
        return;
    }
    post_msg_to_oled_with_arg(op_oled_dis_extend_ui,dc.session_extended_dis_flag);//cancel display extended ui
}
/*************************************************************************************************
    \brief      start oled display for session_extended select
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_dis_session_extended_ui(uint8_t dis_flag)
{
    dc.session_extended_dis_flag=dis_flag;
    dc.extended_frame=0;
    dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", FPS_15, TIMER_OPT_PERIOD, call_back_session_extended_ui);
    bat_timer_start(dc.heat_count_timer, portMAX_DELAY);//ui
    post_msg_to_oled_with_arg(op_oled_dis_extend_ui,dc.session_extended_dis_flag);//display extended ui
}
/*************************************************************************************************
    \brief      handle_display_session_extended_ui: to display enter_session_extended ui 
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_enter_session_extended_ui(msg_st* pmsg)
{
    uint8_t session_frame_temp = 0;
    dc.reverse_frame=SESSION_EXTEND_PERIOD/dc.session_timer_period;
    session_frame_temp=dc.session_frame-dc.enter_extend_frame*dc.reverse_frame/6;
    if(dc.enter_extend_frame*dc.reverse_frame/6 >= dc.session_frame)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR,42108);
        dev_oled_set_region(16,20,73,261);
        dis_pic(&flash_rx_buffer[0],42108,0);
    }
    else if(session_frame_temp>=182)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2+3750+(session_frame_temp-182)*7800,7800);
        dev_oled_set_region(20,206,69,257);
        dis_pic(&flash_rx_buffer[0],7800,0);
    }
    else if(dc.enter_extend_frame>0 && dc.enter_extend_frame<7)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2,3750);
        memcpy(&dc.ui_buffer1_48x112[150],&flash_rx_buffer[0],3750);
        memcpy(&dc.ui_buffer1_48x112[3900],&dc.ui_buffer1_48x112[3750],150);
        memcpy(&dc.ui_buffer1_48x112[4050],&dc.ui_buffer1_48x112[3750],150);
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR,42108);
        for(int i=2;i<28+dc.enter_extend_frame*dc.reverse_frame/6+3;i++)
        {
            for(int j=0;j<50;j++)
            {
                if((3*50*i+3*j+2)<16128)
                {
                    if(i<28)
                    {
                        if(dc.ui_buffer2_40x50[3*50*i+3*j]==0xFB)//||dc.ui_buffer2_40x50[3*50*i+3*j+1]!=0||dc.ui_buffer2_40x50[3*50*i+3*j+2]!=0)
                        {
                            dc.ui_buffer1_48x112[3*50*i+3*j]=flash_rx_buffer[58*3*(3+i+session_frame_temp)+12+3*j];
                            dc.ui_buffer1_48x112[3*50*i+3*j+1]=flash_rx_buffer[3*58*(3+i+session_frame_temp)+12+3*j+1];
                            dc.ui_buffer1_48x112[3*50*i+3*j+2]=flash_rx_buffer[3*58*(3+i+session_frame_temp)+12+3*j+2];
                        }
                    }else
                    {
                        dc.ui_buffer1_48x112[3*50*i+3*j]=flash_rx_buffer[58*3*(3+i+session_frame_temp)+12+3*j];
                        dc.ui_buffer1_48x112[3*50*i+3*j+1]=flash_rx_buffer[3*58*(3+i+session_frame_temp)+12+3*j+1];
                        dc.ui_buffer1_48x112[3*50*i+3*j+2]=flash_rx_buffer[3*58*(3+i+session_frame_temp)+12+3*j+2];
                    }
                }
            }
        }
        if((49+dc.session_frame)>231 || (50+dc.session_frame)>231)
        {
            if(session_frame_temp%2==0)
            {
                dev_oled_set_region(20,24+session_frame_temp,69,231);
                dis_pic(&dc.ui_buffer1_48x112[0],3*50*(231-(24+session_frame_temp)+1),300);
            }
            else
            {
                dev_oled_set_region(20,23+session_frame_temp,69,231);
                dis_pic(&dc.ui_buffer1_48x112[0],3*50*(231-(23+session_frame_temp)+1),300);
            }
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2+3750,7800);
            dev_oled_set_region(20,232,69,257);
            dis_pic(&flash_rx_buffer[0],3900,3900);
        }
        else
        {
            if(session_frame_temp%2==0)
            {
                dev_oled_set_region(20,24+session_frame_temp,69,49+dc.session_frame);
                dis_pic(&dc.ui_buffer1_48x112[0],3*50*(49+dc.session_frame-(24+session_frame_temp)+1),300);
            }
            else
            {
                dev_oled_set_region(20,23+session_frame_temp,69,50+dc.session_frame);
                dis_pic(&dc.ui_buffer1_48x112[0],3*50*(50+dc.session_frame-(23+session_frame_temp)+1),300);
            }
        }
    }
    dc.enter_extend_frame++;
}
/*************************************************************************************************
    \brief      start oled display for enter_session_extended
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_enter_session_extended_ui(void)
{
    dc.heat_cancel_flag=0;
    dc.extended_frame=0;
    dc.session_extended_dis_flag=0;
    dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", 66, TIMER_OPT_PERIOD, call_back_session_extended_ui);
    bat_timer_start(dc.heat_count_timer, portMAX_DELAY);//ui
    post_msg_to_oled_with_arg(op_oled_dis_extend_ui,dc.session_extended_dis_flag);//do not display extended ui
}
/*************************************************************************************************
    \brief      start oled display for heating start
    \param[in]  heat_mode_state:    BASE_WARM_UP = 0x0,
                                    BASE_SESSION,
                                    BOOST_WARM_UP,
                                    BOOST_SESSION, 
    \retval     None
*************************************************************************************************/
void start_oled_heat(uint8_t heat_mode_state)
{
    delete_ui_timer();
    dc.heat_cancel_flag=0;
    dc.heat_state = heat_mode_state;
    if(dc.heat_state == BASE_WARM_UP){
        if(dc.standard_flag==1)
        {
            dc.current_ui_name=MODE_HEAT_UI;
            dc.mode_heatup_frame=0;
            dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_transition_mode_to_heatupui);
            bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
            post_msg_to_oled(op_oled_mode_to_heatup);
            LOGD("ui start:base2warm_up");
        }
        else
        {
            dc.current_ui_name=HEAT_UP_UI;
            post_msg_to_oled(op_oled_clear_black);
            dc.heat_up_frame=0;
            dc.warm_up_count_down_num=0;
            dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (NH_WARMUP_FINISH_TIME)/133, TIMER_OPT_PERIOD, call_back_warm_up_ui);
            bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);//ui
            dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", (1000), TIMER_OPT_PERIOD, call_back_warm_up_count_down_ui);
            bat_timer_start(dc.heat_count_timer, portMAX_DELAY);//num count down
            post_msg_to_oled(op_oled_warm_up);
            post_msg_to_oled(op_oled_warm_up_count_down);
            LOGD("ui start:warm_up");
        }
    }else if(dc.heat_state == BASE_SESSION){
        dc.current_ui_name=HEAT_SESSION_UI;
        dc.session_frame = 0;
        dc.session_count_down_num=0;
        dc.heatup_session_frame=0;
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_transition_heatup_to_session);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
        post_msg_to_oled(op_oled_heatup_session);
        LOGD("ui start:warm_up2session");
    }else if(dc.heat_state == BOOST_WARM_UP){
        if(dc.boost_flag==1)
        {
            dc.current_ui_name=MODE_HEAT_UI;
            dc.mode_heatup_frame=0;
            dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_transition_mode_to_heatupui);
            bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
            post_msg_to_oled(op_oled_mode_to_heatup);
            LOGD("ui start:boost2warm_up");
        }
        else
        {
            dc.current_ui_name=HEAT_UP_UI;
            post_msg_to_oled(op_oled_clear_black);
            dc.heat_up_frame=0;
            dc.warm_up_count_down_num=0;
            dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (BH_WARMUP_FINISH_TIME)/133, TIMER_OPT_PERIOD, call_back_warm_up_ui);
            bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);//ui
            dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", (1000), TIMER_OPT_PERIOD, call_back_warm_up_count_down_ui);
            bat_timer_start(dc.heat_count_timer, portMAX_DELAY);//num count down
            post_msg_to_oled(op_oled_warm_up);
            post_msg_to_oled(op_oled_warm_up_count_down);
            LOGD("ui start:warm_up");
        }
    }else if(dc.heat_state == BOOST_SESSION){
        dc.current_ui_name=HEAT_SESSION_UI;
        dc.session_frame = 0;
        dc.session_count_down_num=0;
        dc.heatup_session_frame=0;
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_transition_heatup_to_session);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
        post_msg_to_oled(op_oled_heatup_session);
        LOGD("ui start:warm_up2session");
    }
 }
/*************************************************************************************************
    \brief      start battery_fade_in_out:fade in or fade out of battery_level num,%,session_num etc.
    \param[in]  i:the frames of fade in or fade out
                battery_level:0-100,
                charging_flag:yes/no,
                fade_in_out:fade in/fade out,
                slow_fast_flag:fast charge/slow(normal)charge,
    \retval     None
*************************************************************************************************/
void battery_fade_in_out(uint8_t i,uint8_t charging_flag,uint8_t slow_fast_flag,uint8_t time_out_flag)
{
    if(dc.battery_level<6)
    {
        dev_oled_set_region(26,34,43,55);
        spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+dc.battery_level%10*1188*6+1188*i,1188);
        dis_pic(&flash_rx_buffer[0],1188,0);
        dev_oled_set_region(46,36,63,55);
        spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+1080*i,1080);
        dis_pic(&flash_rx_buffer[0],1080,0);
    }
    else if(dc.battery_level<100)
    {
        if(dc.battery_level/10==1)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+10*1188*6+1188*i,1188);
        }
        else
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+dc.battery_level/10*1188*6+1188*i,1188);
        }
        if(dc.battery_level>9)
        {
            dev_oled_set_region(18,34,35,55);
            dis_pic(&flash_rx_buffer[0],1188,0);//shiwei
            dev_oled_set_region(36,34,53,55);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+dc.battery_level%10*1188*6+1188*i,1188);
            dis_pic(&flash_rx_buffer[0],1188,0);//gewei
            dev_oled_set_region(56,36,73,55);
            spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+1080*i,1080);
            dis_pic(&flash_rx_buffer[0],1080,0);//  %
        }
        else
        {
            dev_oled_set_region(26,34,43,55);
            spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+dc.battery_level%10*1188*6+1188*i,1188);
            dis_pic(&flash_rx_buffer[0],1188,0);//danshu
            dev_oled_set_region(46,36,63,55);
            spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+1080*i,1080);
            dis_pic(&flash_rx_buffer[0],1080,0);//  %
        }
    }
    else if(dc.battery_level==100)
    {
        dev_oled_set_region(14,32,61,55);
        spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+11*1188*6+3456*i,3456);
        dis_pic(&flash_rx_buffer[0],3456,0);
        dev_oled_set_region(64,36,81,55);
        spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+1080*i,1080);
        dis_pic(&flash_rx_buffer[0],1080,0);//%
    }
    if(charging_flag)  //charge
    {
        if(slow_fast_flag==0)//slow charge
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+0x5A900+7560*i,7560);
            dev_oled_set_region(32,212,73,271);
            dis_pic(&flash_rx_buffer[0],7560,0);
        }
    }
    else if(time_out_flag)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+0x7A130+7200*i,7200);
        dev_oled_set_region(24,214,73,261);
        dis_pic(&flash_rx_buffer[0],7200,0);
    }
    else if(dc.battery_level>5&&dc.battery_level<30)
    {
        dev_oled_set_region(36,212,53,233);
        spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+((dc.battery_level/5)%10)*1188*6+1188*i,1188);
        dis_pic(&flash_rx_buffer[0],1188,0);//session num
//        dev_oled_set_region(46,216,61,233);
//        spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+0x1950+864*i,864);
//        dis_pic(&flash_rx_buffer[0],864,0);//X
        dev_oled_set_region(16,244,73,253);
        spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+0x2D90+1740*i,1740);
        dis_pic(&flash_rx_buffer[0],1740,0); //yan
    }
}

/*************************************************************************************************
    \brief      start count_row:fill the battery 
    \param[in]  start_row:the row of display
                battrery_color:red,orange,yellow,green,
                lightning:the lightning ,
                flag:charge/check, 
    \retval     None
*************************************************************************************************/
static void count_row(uint8_t fill_row,const unsigned char *battrery_color,const unsigned char *lightning,char flag){
    memcpy(&dc.ui_buffer1_48x112[0],&battrery_color[0],46*3*112);
    if(fill_row<97)//if fill_row is 97,should not change the pic,just when fill row less than 97
    {
        for(uint8_t i=fill_row+1;i<98;i++)//i will greater than fill_row,when i greater than fill_row should be black
        {//if fill_row is 0,so fill_row [1,97] should be black
            if(dc.battery_level>59)
            {
                if(i==97)//when row fill is 96,the 97th should be black,total is 97 row fill in battery bar
                    memcpy(&dc.ui_buffer1_48x112[46*3*11+30],&black_temp[0],78);
                else if(i==96)
                    memcpy(&dc.ui_buffer1_48x112[46*3*12+27],&black_temp[0],84);
                else if(i==95)
                    memcpy(&dc.ui_buffer1_48x112[46*3*13+24],&black_temp[0],90);
                else if(i==94)
                    memcpy(&dc.ui_buffer1_48x112[46*3*14+18],&black_temp[0],102);
                else if(i==93)
                    memcpy(&dc.ui_buffer1_48x112[46*3*15+15],&black_temp[0],108);
            }
            if(i==1)
                memcpy(&dc.ui_buffer1_48x112[46*3*107+30],&black_temp[0],78);
            else if(i==2)
                memcpy(&dc.ui_buffer1_48x112[46*3*106+27],&black_temp[0],84);
            else if(i==3)
                memcpy(&dc.ui_buffer1_48x112[46*3*105+24],&black_temp[0],90);
            else if(i==4)
                memcpy(&dc.ui_buffer1_48x112[46*3*104+21],&black_temp[0],96);
            else if(i==5)
                memcpy(&dc.ui_buffer1_48x112[46*3*103+18],&black_temp[0],102);
            else if(i==6)
                memcpy(&dc.ui_buffer1_48x112[46*3*102+15],&black_temp[0],108);
            if(i>6 && i<93)
                memcpy(&dc.ui_buffer1_48x112[46*3*(97-i+11)+12],&black_temp[0],114);
        }
    }
    if(fill_row>32&&fill_row<65&&flag&&(dc.fast_charge==0||dc.fast_charge==2))//when lightning row is coincide with fill row
    {//slow charge or normal charge,lightning row in battery row is in [33,64]
        for(uint8_t i=fill_row+1;i<65;i++)
        {
            memcpy(&dc.ui_buffer1_48x112[46*3*(97-i+11)+12],&lightning[114*(64-i)],114);
        }
    }
    if(fill_row<33&&flag&&(dc.fast_charge==0||dc.fast_charge==2))//fill row is less than lightning row
    {//battery row is less than 33
        for(uint8_t i=33;i<65;i++)
        {
            memcpy(&dc.ui_buffer1_48x112[46*3*(97-i+11)+12],&lightning[114*(64-i)],114);
        }
    }
    if(fill_row>28&&fill_row<71&&flag&&dc.fast_charge==1)//when lightning row is coincide with fill row
    { //fast charge,lightning row in battery row is in [29,70]
        for(uint8_t i=fill_row+1;i<71;i++)
        {
            memcpy(&dc.ui_buffer1_48x112[46*3*(97-i+11)+12],&lightning[114*(70-i)],114);//fill row=29buffer row=79,they coincide
        }
    }
    if(fill_row<29&&flag&&dc.fast_charge==1)//fill row is less than lightning row
    {//battery row is less than 29
        for(uint8_t i=29;i<71;i++)
        {
            memcpy(&dc.ui_buffer1_48x112[46*3*(97-i+11)+12],&lightning[114*(70-i)],114);
        }
    }
    if(!flag&&fill_row>32)
    {//battery check
        for(uint8_t i=33;i<65;i++)
        {
            memcpy(&dc.ui_buffer1_48x112[46*3*(97-i+11)+42],&dc.ui_buffer1_48x112[46*3*(97-i+11)+12],30);
            memcpy(&dc.ui_buffer1_48x112[46*3*(97-i+11)+72],&dc.ui_buffer1_48x112[46*3*(97-i+11)+12],30);
        }
    }
    dev_oled_set_region(22,76,67,187);
    dis_pic(&dc.ui_buffer1_48x112[0],46*3*112,0);
}


/*************************************************************************************************
    \brief      handle_oled_battery_check: to refresh the frames of battery_check
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_battery_check(msg_st* pmsg){
    if(dc.current_ui_name!=BATTERY_CHECK_UI)
    {
        return;
    }
    uint32_t battery_color_add =0;
    int8_t fill_row=0;
    if(dc.battery_level>=0&&dc.battery_level<5){
        fill_row=5;
    }
    else if(dc.battery_level<29)
    {
        fill_row=dc.battery_level;
    }
    else if(dc.battery_level==29)
    {
        fill_row=28;
    }
    else if(dc.battery_level<59)
    {
        fill_row=dc.battery_level-1;
    }
    else if(dc.battery_level==59)
    {
        fill_row=57;
    }
    else if(dc.battery_level<100)
    {
        fill_row=dc.battery_level-2;
    }
    else if(dc.battery_level==100)
    {
        fill_row=97;
    }
    if(dc.battery_level>=0&&dc.battery_level<10)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR;
    else if(dc.battery_level<30)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*6;
    else if(dc.battery_level<60)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*12;
    else if(dc.battery_level<=100)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*18;
    if(dc.battery_level>=0&&dc.battery_level<6)
    {
        if(dc.battery_check_frame<9) //fade in
        {
            dev_oled_set_region(36,214,53,281);
            spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+0x5658+3672*dc.battery_check_frame,3672);//plug fade in
            dis_pic(&flash_rx_buffer[0],3672,0);
            if(dc.battery_check_frame<6) //fade in
            {
                battery_fade_in_out(dc.battery_check_frame,0,0,0);//%,num fade in
                spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+15456*dc.battery_check_frame,15456);//border fade in
                count_row(0,&flash_rx_buffer[0],NULL,0);
            }
        }
        else if(dc.battery_check_frame<34)
        {
            if((dc.battery_check_frame-3)%6==0&&(dc.battery_check_frame-3)%12!=0)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+15456*5,15456);
                count_row(fill_row,&flash_rx_buffer[0],NULL,0);
            }
            else if((dc.battery_check_frame-3)%12==0){
                spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+15456*5,15456);
                count_row(0,&flash_rx_buffer[0],NULL,0);
            }
        }
        else if(dc.battery_check_frame<(DISPLAY_DURATION*1000/FPS_15-5))
        {
            //do nothing
        }
        else if(dc.battery_check_frame<(DISPLAY_DURATION*1000/FPS_15))//fade out
        {
            dev_oled_set_region(36,214,53,281);
            spi_flash_buffer_read(&flash_rx_buffer[0],BATTERY_ICON_START_ADDR+0xD770+3672*((dc.battery_check_frame+1)-(DISPLAY_DURATION*1000/FPS_15-5)),3672);//plug fade out
            dis_pic(&flash_rx_buffer[0],3672,0);//plug out
            battery_fade_in_out(((DISPLAY_DURATION*1000/FPS_15-1)-dc.battery_check_frame),0,0,0);//%,num fade out
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+((DISPLAY_DURATION*1000/FPS_15-1)-dc.battery_check_frame)*15456,15456);
            count_row(fill_row,&flash_rx_buffer[0],NULL,0);//border fade out
        }
        else if(dc.battery_check_frame==(DISPLAY_DURATION*1000/FPS_15))
        {
            lcd_clear(&black_temp[0]);
            LOGD("ui end:low soc");
        }
    }
    else if(dc.battery_level<=100)
    {
        if(dc.battery_check_frame<6) //fade in
        {
            battery_fade_in_out(dc.battery_check_frame,0,0,0);//%,num fade in
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+dc.battery_check_frame*15456,15456);//
            count_row(0,&flash_rx_buffer[0],NULL,0);//boder fade in
        }
        else if(dc.battery_check_frame<(6+12)){//fill
            if(dc.battery_level<10)
            {
                if(dc.battery_check_frame<10)
                {
                    spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+0x6AFB0+(dc.battery_check_frame-6)*15456,15456);
                }
                else
                {
                    spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+5*15456,15456);
                }
            }
            else{
                spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+5*15456,15456);
            }
            count_row(battery_fill[dc.battery_check_frame-6]*fill_row/(12*100),&flash_rx_buffer[0],NULL,0);
        }
        else if(dc.battery_check_frame==(6+12))
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+5*15456,15456);
            count_row(fill_row,&flash_rx_buffer[0],NULL,0);
        }
        else if(dc.battery_check_frame<((DISPLAY_DURATION*1000-40*12)/FPS_15-5+12))
        {
            //do nothing
        }
        else if(dc.battery_check_frame<(((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)) //fade out
        {
            battery_fade_in_out((((DISPLAY_DURATION*1000-40*12)/FPS_15)-1+12)-dc.battery_check_frame,0,0,0);//%,num fade out
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+((((DISPLAY_DURATION*1000-40*12)/FPS_15)-1+12)-dc.battery_check_frame)*15456,15456);
            count_row(fill_row,&flash_rx_buffer[0],NULL,0);//border fade out
        }
        else if(dc.battery_check_frame==(((DISPLAY_DURATION*1000-40*12)/FPS_15)+12))
        {
            LOGD("ui end:normal soc");
            dc.battery_check_frame++;
            lcd_clear(&black_temp[0]);
            return;
        }
        else if(dc.battery_check_frame==(((DISPLAY_DURATION*1000-40*12)/FPS_15)+1+12))
        {
            uint8_t hall_flag = app_get_hall_door_status();
            if(hall_flag == door_base)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_MODE_START_ADDR,19968);
                dev_oled_set_region(26,34,57,241);
                dis_pic(&flash_rx_buffer[0],19968,0);
                dc.standard_flag=1;
            }
            else if(hall_flag == door_boost)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_MODE_START_ADDR,21168);
                dev_oled_set_region(26,22,61,217);
                dis_pic(&flash_rx_buffer[0],21168,0);
                dc.boost_flag=1;
            }
        }
        else if(dc.battery_check_frame==((((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)+(5*1000/FPS_15)))
        {
            lcd_clear(&black_temp[0]);
            dc.standard_flag=0;
            dc.boost_flag=0;
        }
    }
    dc.battery_check_frame++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh battery_check frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_battery_check_ui(const ptimer_t tm){
    if(dc.battery_level>=6)
    {
        if(dc.battery_check_frame==5)
        {
            dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_battery_check_ui);
            bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
        }
        else if(dc.battery_check_frame==17)
        {
            dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_15, TIMER_OPT_PERIOD, call_back_battery_check_ui);
            bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
        }
        else if(dc.battery_check_frame==((DISPLAY_DURATION*1000-40*12)/FPS_15+12+1))
        {
            clear_all_oled_lock();
        }
        else if(dc.battery_check_frame > ((DISPLAY_DURATION*1000-40*12)/FPS_15+12+(5*1000/FPS_15)))
        {
            if(dc.heat_ui_timer){
                if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                    dc.heat_ui_timer = NULL;
                }
            }
            return;
        }
    }
    else if(dc.battery_level<6)
    {
        if(dc.battery_check_frame > ((DISPLAY_DURATION*1000)/FPS_15))
        {
            clear_all_oled_lock();
            if(dc.heat_ui_timer){
                if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                    dc.heat_ui_timer = NULL;
                }
            }
            return;
        }
    }
    post_msg_to_oled_with_arg(op_oled_battery_check,dc.battery_level);
}


/*************************************************************************************************
    \brief      start oled display for battery_check
    \param[in]  battery_soc: specify the bat_left(00 - 100)
    \retval     None
*************************************************************************************************/
void start_oled_display_battery_check(uint8_t battery_soc)
{
    delete_ui_timer();
    if(battery_soc==100)
    {
        if(app_check_usb_plug_status() == WELL_USB_PLUG)
        {
            start_oled_display_charging_check(battery_soc);
            return;
        }
    }
    post_msg_to_oled(op_oled_clear_black);
    dc.current_ui_name=BATTERY_CHECK_UI;
    dc.boost_flag=0;
    dc.battery_check_frame=0;
    dc.battery_level = battery_soc;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_15, TIMER_OPT_PERIOD, call_back_battery_check_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    if(dc.battery_level>=0&&dc.battery_level<6)
    {
        LOGD("ui start:low soc");
    }else if(dc.battery_level<=100){
        LOGD("ui start:normal soc");
    }else{
        LOGD("illegal soc %d",battery_soc);
    }
}

/*************************************************************************************************
  * @brief    : Timer call back function to start cleaning after battery
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_display_cleaning(const ptimer_t tm)
{
    if(dc.heat_count_timer){
        if(bat_timer_delete(dc.heat_count_timer, portMAX_DELAY)==pdPASS){
            dc.heat_count_timer = NULL;
        }
    }
    clear_all_oled_lock();
    start_oled_cleaning_prompt();
}
/*************************************************************************************************
    \brief      start oled display for battery_check and cleaning UI
    \param[in]  battery_soc: specify the bat_left(00 - 100)
    \retval     None
*************************************************************************************************/
void start_oled_batterycheck_and_cleaning(uint8_t battery_soc)
{
    delete_ui_timer();
    dc.standard_flag=0;
    dc.boost_flag=0;
    start_oled_display_battery_check(battery_soc);
    //start a one_shot timer, call the cleaning after 2 second
    dc.heat_count_timer = bat_timer_reset_ext(dc.heat_count_timer, "heat_count_timer", 3500, TIMER_OPT_ONESHOT, call_back_display_cleaning);
    bat_timer_start(dc.heat_count_timer, portMAX_DELAY);
}
/*************************************************************************************************
    \brief      handle_oled_battery_check: to refresh the frames of charging_check
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_charging_check(msg_st* pmsg){
    if(dc.current_ui_name!=CHARGE_UI)
    {
        return;
    }
    uint32_t battery_color_add =0;
    uint32_t battery_lightning_addr =0;
    uint16_t battery_lightning_size =0;
    int8_t fill_row=0;
    if(dc.battery_level>=0&&dc.battery_level<5){
        fill_row=5;
    }
    else if(dc.battery_level<29)
    {
        fill_row=dc.battery_level;
    }
    else if(dc.battery_level==29)
    {
        fill_row=28;
    }
    else if(dc.battery_level<59)
    {
        fill_row=dc.battery_level-1;
    }
    else if(dc.battery_level==59)
    {
        fill_row=57;
    }
    else if(dc.battery_level<100)
    {
        fill_row=dc.battery_level-2;
    }
    else if(dc.battery_level==100)
    {
        fill_row=97;
    }
    if(dc.fast_charge==1)
    {
        battery_lightning_addr=FAST_CHARGE_START_ADDR+0x5A900;
        battery_lightning_size=4788;
        if(dc.battery_level>=0&&dc.battery_level<10)
            battery_color_add=FAST_CHARGE_START_ADDR;
        else if(dc.battery_level<30)
            battery_color_add=FAST_CHARGE_START_ADDR+15456*6;
        else if(dc.battery_level<60)
            battery_color_add=FAST_CHARGE_START_ADDR+15456*12;
        else if(dc.battery_level<=100)
            battery_color_add=FAST_CHARGE_START_ADDR+15456*18;
    }
    else if(dc.fast_charge==0||dc.fast_charge==2)
    {
        battery_lightning_addr=NORMAL_SLOW_CHARGE_START_ADDR+0x65A30;
        battery_lightning_size=3648;
        if(dc.battery_level>=0&&dc.battery_level<10)
            battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR;
        else if(dc.battery_level<30)
            battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*6;
        else if(dc.battery_level<60)
            battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*12;
        else if(dc.battery_level<=100)
            battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*18;
    }
    if(dc.battery_level<=100)
    {
        if(dc.charge_frame<6) //fade in
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_lightning_addr+dc.charge_frame*battery_lightning_size,battery_lightning_size);
            memcpy(&dc.ui_buffer2_40x50[0],&flash_rx_buffer[0],battery_lightning_size);
            battery_fade_in_out(dc.charge_frame,1,dc.fast_charge,0);
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+dc.charge_frame*15456,15456);
            count_row(0,&flash_rx_buffer[0],&dc.ui_buffer2_40x50[0],1);
        }
        else if(dc.charge_frame<(12+6)){//fill
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_lightning_addr+5*battery_lightning_size,battery_lightning_size);
            memcpy(&dc.ui_buffer2_40x50[0],&flash_rx_buffer[0],battery_lightning_size);
            if(dc.battery_level<10)
            {
                if(dc.charge_frame<10)
                {
                    spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+0x6AFB0+(dc.charge_frame-6)*15456,15456);
                }
                else
                {
                    spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+5*15456,15456);
                }
            }
            else{
                spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+5*15456,15456);
            }
            count_row(battery_fill[dc.charge_frame-6]*fill_row/(12*100),&flash_rx_buffer[0],&dc.ui_buffer2_40x50[0],1);
        }
        else if(dc.charge_frame==(12+6))
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_lightning_addr+5*battery_lightning_size,battery_lightning_size);
            memcpy(&dc.ui_buffer2_40x50[0],&flash_rx_buffer[0],battery_lightning_size);
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+5*15456,15456);
            count_row(fill_row,&flash_rx_buffer[0],&dc.ui_buffer2_40x50[0],1);
        }
        else if(dc.charge_frame<((DISPLAY_DURATION*1000-40*12)/FPS_15-5+12))
        {
                //do nothing
        }
        else if(dc.charge_frame<(((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)) //fade out
        {
            battery_fade_in_out((((DISPLAY_DURATION*1000-40*12)/FPS_15)-1+12)-dc.charge_frame,1,dc.fast_charge,0);
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_lightning_addr+((((DISPLAY_DURATION*1000-40*12)/FPS_15)-1+12)-dc.charge_frame)*battery_lightning_size,battery_lightning_size);
            memcpy(&dc.ui_buffer2_40x50[0],&flash_rx_buffer[0],battery_lightning_size);
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+((((DISPLAY_DURATION*1000-40*12)/FPS_15)-1+12)-dc.charge_frame)*15456,15456);
            count_row(fill_row,&flash_rx_buffer[0],&dc.ui_buffer2_40x50[0],1);
        }else if(dc.charge_frame==(((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)){
            lcd_clear(&black_temp[0]);
            LOGD("ui end:charge");
        }
    }
    dc.charge_frame++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh charging_check frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_charging_check_ui(const ptimer_t tm){
    if(dc.charge_frame==5)
    {
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_charging_check_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    }
    else if(dc.charge_frame==17)
    {
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_15, TIMER_OPT_PERIOD, call_back_charging_check_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    }
    else if(dc.charge_frame>(((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)){
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        if(dc.heat_state!=HEAT_OVER)
        {
            uint8_t hall_flag = app_get_hall_door_status();
            if(hall_flag == door_base)
            {
                start_oled_display_mode_select(base_open);
            }
            else if(hall_flag == door_boost)
            {
                start_oled_display_mode_select(boost_open);
            }
            dc.heat_state=HEAT_OVER;
        }
        clear_all_oled_lock();
        return;
    }
    post_msg_to_oled_with_arg(op_oled_charging_check,dc.battery_level);
}


/*************************************************************************************************
    \brief      start oled display for charging_check
    \param[in]  battery_soc: specify the bat_left(00 - 100)
    \retval     None
*************************************************************************************************/
void start_oled_display_charging_check(uint8_t battery_soc)
{
    delete_ui_timer();
    post_msg_to_oled(op_oled_clear_black);
    dc.current_ui_name=CHARGE_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.battery_level = battery_soc;
//    if(dc.heat_ui_timer){
//        if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
//            dc.heat_ui_timer = NULL;
//        }
//    }
	//vTaskDelay(200);  ?why?
    dc.charge_frame=0;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (66), TIMER_OPT_PERIOD, call_back_charging_check_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);

    if (app_get_charging_mode()==FAST_CHARGE_MODE){
        dc.fast_charge=1;//fast charge
    }
    else if(app_get_charging_mode()==SLOW_CHARGE_MODE){
        dc.fast_charge=0; //slow charrge
    }
    else{
        dc.fast_charge=2; //normal charrge
    }
    if(dc.battery_level>=0&&dc.battery_level<=100)
    {
        LOGD("ui start:charge");
    }else{
        LOGD("illegal soc %d",battery_soc);
    }
}
/*************************************************************************************************
    \brief      handel_transition_heatup_to_remove: to refresh the frames of cancel warm_up to remove
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handel_transition_heatup_to_remove(msg_st* pmsg)
{
    if(dc.current_ui_name!=HEAT_REMOVE_UI)
    {
        return;
    }
    if(dc.heatup_remove_frame<8)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],HEAT_REMOVE_START_ADDR+dc.heatup_remove_frame*11340,11340);
        dev_oled_set_region(36,36,53,245);
        dis_pic(&flash_rx_buffer[0],11340,0);
    }
    else if(dc.heatup_remove_frame<17)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],HEAT_REMOVE_START_ADDR+0x16260+(dc.heatup_remove_frame-8)*4836,4836);
        dev_oled_set_region(14,224,75,249);
        dis_pic(&flash_rx_buffer[0],4836,0);
    }
    if(dc.heatup_remove_frame==16)
    {
        LOGD("ui end:warm_up2remove");
    }
    dc.heatup_remove_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh heatup_remove frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_heatup_remove(const ptimer_t tm)
{
    if(dc.heatup_remove_frame>16)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        post_msg_to_manager(op_oled_remove_stick_ui);
        return;
    }
    post_msg_to_oled(op_oled_heatup_remove);
}
/*************************************************************************************************
    \brief      start oled display for transition_heatup_to_remove_stickout
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_oled_transition_heatup_to_remove_stickout()
{
    delete_ui_timer();
    dc.current_ui_name=HEAT_REMOVE_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.heatup_remove_frame=0;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_heatup_remove);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    post_msg_to_oled(op_oled_heatup_remove);
    LOGD("ui start:warm_up2remove");
}
/*************************************************************************************************
    \brief      handle_oled_heat_up_cancel_ui: to refresh the frames of warm_up_cancel
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_heat_up_cancel_ui(msg_st* pmsg){
    if(dc.current_ui_name!=HEAT_UP_CANCEL_UI)
    {
        return;
    }
    if(dc.heat_up_cancel_frame==0)
    {
        memset(&dc.ui_buffer2_40x50[0],0x00,270);
        memcpy(&dc.ui_buffer2_40x50[270],&dc.ui_buffer1_48x112[30],150);
        memcpy(&dc.ui_buffer2_40x50[420],&dc.ui_buffer2_40x50[360],60);
        dev_oled_set_region(28,226,63,247);
        dis_pic(&black_temp[0],2376,0);//num
        dc.heat_up_cancel_frame++;
    }

    if( (dc.heat_up_frame>40) && (dc.heat_up_frame<135))
    {
        for(int j=90;j<160;j++)
        {
            if(dc.ui_buffer2_40x50[3*j]!=0&&dc.ui_buffer2_40x50[3*j+1]!=0)
                dc.ui_buffer2_40x50[3*j+1]+=0x06;
        }
    }
    if(dc.heat_up_frame>0&&dc.heat_up_frame<134)
    {
        if(dc.heat_up_frame%2==0)
        {
            dev_oled_set_region(40,172-dc.heat_up_frame,49,179-dc.heat_up_frame+6);
            dis_pic(&dc.ui_buffer2_40x50[0],420,60);
        }
        else
        {
            dev_oled_set_region(40,172-dc.heat_up_frame-1,49,179-dc.heat_up_frame+1+6);
            dis_pic(&dc.ui_buffer2_40x50[0],480,0);
        }
    }
    if(dc.heat_up_frame==0)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],WARM_UP_START_ADDR+0x207C,8316);
        dev_oled_set_region(36,36,53,189);
        dis_pic(&flash_rx_buffer[0],8316,0);
        dc.heat_up_frame=134;
        LOGD("ui end:cancel warm up");
    }
    if(dc.heat_up_frame<134)
    {
        if(dc.heat_up_frame>=6)
        {
            dc.heat_up_frame-=6;
        }
        else if(dc.heat_up_frame<6)
        {
            dc.heat_up_frame=0;
        }
    }
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh warm_up cancel frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_cancel_heatupui(const ptimer_t tm)
{
    if(dc.cancel_count>=0&&dc.cancel_count<134)
    {
        post_msg_to_oled(op_oled_cancel_heatup);
        if(dc.cancel_count==0)
            dc.cancel_count=-1;
        if(dc.cancel_count>=6)
        {
            dc.cancel_count-=6;
        }
        else if(dc.cancel_count<6)
        {
            dc.cancel_count=0;
        }
    }
    if(dc.heat_up_frame==134)
    {
        post_msg_to_manager(op_oled_heatup_remove);
    }
}

void handle_cancel_session_ui(msg_st* pmsg)
{
    if(dc.current_ui_name!=SESSION_CANCEL_UI)
    {
        return;
    }
    if(dc.heat_up_cancel_frame==0)
    {
        memset(&dc.ui_buffer2_40x50[0],0x00,6000);
        if(dc.session_frame==0)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR,42108);
            dev_oled_set_region(16,20,73,261);
            dis_pic(&flash_rx_buffer[0],42108,0);
            memset(&dc.ui_buffer1_48x112[0],0x00,6000);
            memset(&dc.ui_buffer2_40x50[0],0x00,6000);
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2,3750);
            memcpy(&dc.ui_buffer2_40x50[1650],&flash_rx_buffer[0],3750);
            memcpy(&dc.ui_buffer2_40x50[5400],&dc.ui_buffer2_40x50[5100],300);
            memcpy(&dc.ui_buffer1_48x112[1650],&flash_rx_buffer[0],3750);
            memcpy(&dc.ui_buffer1_48x112[5400],&dc.ui_buffer1_48x112[5100],300);
        }
        else
        {
            memcpy(&dc.ui_buffer2_40x50[1650],&dc.ui_buffer1_48x112[150],4050);
            memset(&dc.ui_buffer1_48x112[0],0x00,6000);
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2,3750);
            memcpy(&dc.ui_buffer1_48x112[1650],&flash_rx_buffer[0],3750);
            memcpy(&dc.ui_buffer1_48x112[5400],&dc.ui_buffer1_48x112[5100],300);
        }
        dc.heat_up_cancel_frame++;
    }
    if(dc.session_frame<27)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+0xA47C,42108);
        for(int t=0;dc.session_frame+t<36;t++)
        {
            for(int j=0;j<50;j++)
            {
                if((50*3*(t)+3*j+2)<6000)
                {
                    if(dc.ui_buffer1_48x112[50*3*(t)+3*j]==0x00&&dc.ui_buffer1_48x112[50*3*(t)+3*j+1]==0x00)
                    {
                        dc.ui_buffer2_40x50[50*3*(t)+3*j]=flash_rx_buffer[58*3*(3+dc.session_frame+t)+12+3*j];
                        dc.ui_buffer2_40x50[50*3*(t)+3*j+1]=flash_rx_buffer[58*3*(3+dc.session_frame+t)+12+3*j+1];
                        dc.ui_buffer2_40x50[50*3*(t)+3*j+2]=flash_rx_buffer[58*3*(3+dc.session_frame+t)+12+3*j+2];
                    }
                }
            }
        }
    }
    if(dc.session_frame<182)
    {
        for(int j=0;j<1900;j++)
        {
            if(dc.ui_buffer1_48x112[3*j+1]!=0)
            {
                dc.ui_buffer2_40x50[3*j+1]+=0x03;
            }
        }
        if(dc.session_frame%2==0)
        {
            dev_oled_set_region(20,24+dc.session_frame,69,59+dc.session_frame);
            dis_pic(&dc.ui_buffer2_40x50[0],5400,150);
        }
        else
        {
            dev_oled_set_region(20,23+dc.session_frame,69,60+dc.session_frame);
            dis_pic(&dc.ui_buffer2_40x50[0],5700,0);
        }
        if(dc.session_frame==171)
            dc.session_frame++;
    }
    else if(dc.session_frame<212)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_START_ADDR+42108*2+3750 + (dc.session_frame-182)*7800,7800);
        dev_oled_set_region(20,206,69,257);
        dis_pic(&flash_rx_buffer[0],7800,0);
    }
    if (dc.session_frame==211){
        LOGD("ui end:cancel session");
        dc.session_frame++;
    }
    if(dc.session_frame<162)
        dc.session_frame+=10;
    else if(dc.session_frame<172)
        dc.session_frame=171;
    else if(dc.session_frame<202)
        dc.session_frame+=10;
    else if(dc.session_frame<211)
        dc.session_frame=211;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh session cancel frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_cancel_session(const ptimer_t tm)
{
    if(dc.cancel_count<212)
    {
        post_msg_to_oled(op_oled_cancel_session);
        if(dc.cancel_count==171||dc.cancel_count==211)
            dc.cancel_count++;
        if(dc.cancel_count<162)
            dc.cancel_count+=10;
        else if(dc.cancel_count<172)
            dc.cancel_count=171;
        else if(dc.cancel_count<202)
            dc.cancel_count+=10;
        else if(dc.cancel_count<211)
            dc.cancel_count=211;
    }
    if(dc.session_frame==212)
    {
        post_msg_to_manager(op_oled_session_to_remove);
    }
}
/*************************************************************************************************
    \brief      start oled display for cancel heat
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_oled_heat_cancel(void){
    dc.heat_cancel_flag=1;
    if(dc.current_ui_name == HEAT_SESSION_UI)
    {
        return;
    }
    if(dc.current_ui_name == MODE_HEAT_UI)
    {
        return;
    }
    delete_ui_timer();
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.heat_up_cancel_frame=0;
    if(dc.heat_state == BASE_WARM_UP || dc.heat_state == BOOST_WARM_UP)
    {
        dc.current_ui_name=HEAT_UP_CANCEL_UI;
        if(dc.heat_up_frame<134)
        {
            dc.cancel_count=dc.heat_up_frame;
        }else{
            dc.cancel_count=133;
        }
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", 10, TIMER_OPT_PERIOD, call_back_cancel_heatupui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
        post_msg_to_oled(op_oled_cancel_heatup);
        LOGD("ui start:cancel warm up");
    }else if(dc.heat_state == BASE_SESSION || dc.heat_state == BOOST_SESSION)
    { //stop session
        if(app_get_session_extend_flag()==SESSION_EXTEND_TIME_START)
        {
            start_dis_session_extended_ui(0);
            return;
        }
        dc.current_ui_name=SESSION_CANCEL_UI;
        if(dc.session_frame>211)
        {
            dc.session_frame=211;
        }
        dc.cancel_count=dc.session_frame;
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", 15, TIMER_OPT_PERIOD, call_back_cancel_session);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
        post_msg_to_oled(op_oled_cancel_session);
        LOGD("ui start:cancel session");
    }
}
/*************************************************************************************************
    \brief      handle_oled_remove_mode_ui: to refresh the frames of remove_mode
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_remove_mode_ui(msg_st* pmsg)
{
    if(dc.current_ui_name!=REMOVE_MODE_UI)
    {
        return;
    }
    if(dc.remove_mode_frame < 5)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+dc.remove_mode_frame*11160,11160);
        dev_oled_set_region(14,190,75,249);
        dis_pic(&flash_rx_buffer[0],11160,0);
    }
    else if(dc.remove_mode_frame == 5)
    {
        dev_oled_set_region(14,190,75,249);
        dis_pic(&black_temp[0],11160,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0xD9F8+(dc.remove_mode_frame-5)*7884,7884);
        dev_oled_set_region(36,72,53,217);
        dis_pic(&flash_rx_buffer[0],7884,0);
    }else if(dc.remove_mode_frame == 6)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0xD9F8+(dc.remove_mode_frame-5)*7884,7884);
        dev_oled_set_region(36,72,53,217);
        dis_pic(&flash_rx_buffer[0],7884,0);
    }
    else if(dc.remove_mode_frame < 10)
    {
        if(dc.remove_mode_frame==7)
        {
            dev_oled_set_region(36,72,53,217);
            dis_pic(&black_temp[0],7884,0);
        }
        switch(app_get_hall_door_status())
        {
            case door_boost:
                spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0x11790+(dc.remove_mode_frame-7)*2016,2016);
                dev_oled_set_region(42,72,49,155);
                dis_pic(&flash_rx_buffer[0],2016,0);
            break;
            case door_base:
                spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0x12F30+(dc.remove_mode_frame-7)*2016,2016);
                dev_oled_set_region(42,72,49,155);
                dis_pic(&flash_rx_buffer[0],2016,0);
            break;
            default:
                lcd_clear(&black_temp[0]);
                dc.remove_mode_frame=17+5*1000/FPS_25;
            break;
        }
    }
    else if(dc.remove_mode_frame <15)
    {
        if(dc.remove_mode_frame==10)
        {
            dev_oled_set_region(42,72,49,155);
            dis_pic(&black_temp[0],2016,0);
        }
        switch(app_get_hall_door_status())
        {
            case door_boost:
                spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0x146D0+(dc.remove_mode_frame-10)*7752,7752);
                dev_oled_set_region(28,22,61,97);
                dis_pic(&flash_rx_buffer[0],7752,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BOOST_START_ADDR+0xFD20+(dc.remove_mode_frame-10)*5412,5412);
                dev_oled_set_region(34,136,55,217);
                dis_pic(&flash_rx_buffer[0],5412,0);
            break;
            case door_base:
                spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0x21AC8+(dc.remove_mode_frame-10)*5184,5184);
                dev_oled_set_region(34,32,57,103);
                dis_pic(&flash_rx_buffer[0],5184,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],BRAND_TO_BASE_START_ADDR+0xDC80+(dc.remove_mode_frame-10)*7680,7680);
                dev_oled_set_region(34,114,53,241);
                dis_pic(&flash_rx_buffer[0],7680,0);
            break;
            default:
                lcd_clear(&black_temp[0]);
                dc.remove_mode_frame=16+5*1000/FPS_25;
            break;
        }
    }
    else if(dc.remove_mode_frame == 15)
    {
        if(app_get_hall_door_status()==door_boost) //to boost
        {
            dc.boost_flag=1;
            spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0x146D0+(dc.remove_mode_frame-10)*7752,7752);
            dev_oled_set_region(28,22,61,97);
            dis_pic(&flash_rx_buffer[0],7752,0);
        }
        else if(app_get_hall_door_status()==door_base) //to base
        {
            dc.standard_flag=1;
            spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_MODE_START_ADDR,19968);
            dev_oled_set_region(26,34,57,241);
            dis_pic(&flash_rx_buffer[0],19968,0);
            LOGD("ui end:remove2base");
        }
    }
    else if(dc.remove_mode_frame == 16)
    {
        if(app_get_hall_door_status()==door_boost) //to boost
        {
            dc.boost_flag=1;
            spi_flash_buffer_read(&flash_rx_buffer[0],TRANS_REMOVE_TO_MODE_START_ADDR+0x146D0+(dc.remove_mode_frame-10)*7752,7752);
            dev_oled_set_region(28,22,61,97);
            dis_pic(&flash_rx_buffer[0],7752,0);
            LOGD("ui end:remove2boost");
        }
    }
    else if(dc.remove_mode_frame==16+5*1000/FPS_25)
    {
        dc.boost_flag=0;
        dc.standard_flag=0;
        lcd_clear(&black_temp[0]);
    }
    dc.remove_mode_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh remove_mode frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
void call_back_remove_mode_ui(const ptimer_t tm){
    if(dc.remove_mode_frame>(16+5*1000/FPS_25))
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        return;
    }
    post_msg_to_oled(op_oled_remove_mode_ui);
}
/*************************************************************************************************
    \brief      start oled display for remove_mode
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_op_oled_remove_mode_ui(void)
{
    dc.heat_state=HEAT_OVER;
    delete_ui_timer();
    dc.current_ui_name=REMOVE_MODE_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.remove_mode_frame=0;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_remove_mode_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    post_msg_to_oled(op_oled_remove_mode_ui);
    if(app_get_hall_door_status()==door_base)
    {
        LOGD("ui start:remove2base");
    }else if(app_get_hall_door_status()==door_boost){
        LOGD("ui start:remove2boost");
    }
}

/*************************************************************************************************
    \brief      handle_oled_remove_stick_ui: to refresh the frames of remove_stick
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_remove_stick_ui(msg_st* pmsg)
{
    if(dc.current_ui_name!=REMOVE_STICK_UI)
    {
        return;
    }
    if(dc.remove_frame<5)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REMOVE_STICK_OUT_START_ADDR+dc.remove_frame*13020,13020);
        dev_oled_set_region(14,180,75,249);
        dis_pic(&flash_rx_buffer[0],13020,0);
    }
    else if(dc.remove_frame<19)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REMOVE_STICK_OUT_START_ADDR+13020*5+(dc.remove_frame-5)*38316,38316);
        dev_oled_set_region(14,44,75,249);
        dis_pic(&flash_rx_buffer[0],38316,0);
    }
    else if(dc.remove_frame<23)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REMOVE_STICK_OUT_START_ADDR+13020*5+14*38316+9288*(dc.remove_frame-19),9288);
        dev_oled_set_region(36,44,53,215);
        dis_pic(&flash_rx_buffer[0],9288,0);
    }
    else if(dc.remove_frame==23)
    {
        dev_oled_set_region(36,44,53,215);
        dis_pic(&black_temp[0],9288,0);
    }
    else if(dc.remove_frame<(5+24))
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REMOVE_STICK_OUT_START_ADDR+(dc.remove_frame-24)*13020,13020);
        dev_oled_set_region(14,180,75,249);
        dis_pic(&flash_rx_buffer[0],13020,0);
    }
    else if(dc.remove_frame<(19+24))
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REMOVE_STICK_OUT_START_ADDR+13020*5+(dc.remove_frame-29)*38316,38316);
        dev_oled_set_region(14,44,75,249);
        dis_pic(&flash_rx_buffer[0],38316,0);
    }
    else if(dc.remove_frame<(23+24))
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],REMOVE_STICK_OUT_START_ADDR+13020*5+14*38316+9288*(dc.remove_frame-43),9288);
        dev_oled_set_region(36,44,53,215);
        dis_pic(&flash_rx_buffer[0],9288,0);
    }
    if(dc.remove_frame==47)
    {
        dev_oled_set_region(36,44,53,215);
        dis_pic(&black_temp[0],9288,0);
        LOGD("ui end:remove_stick");
    }
    dc.remove_frame++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh remove_stick frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
void call_back_remove_stick_ui(const ptimer_t tm){
    if(dc.remove_frame>47)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        post_msg_to_manager(op_oled_remove_mode_ui);
        return;
    }
    post_msg_to_oled(op_oled_remove_stick_ui);
}
/*************************************************************************************************
    \brief      start oled display for remove_stick
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_op_oled_remove_stick_ui(void)
{
    delete_ui_timer();
    dc.current_ui_name=REMOVE_STICK_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.remove_frame=0;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_15, TIMER_OPT_PERIOD, call_back_remove_stick_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    post_msg_to_oled(op_oled_remove_stick_ui);
    LOGD("ui start:remove_stick");
}

/*************************************************************************************************
    \brief      handle_oled_transition_session_to_remove_stickout: to refresh the frames of transition_session_to_remove_stickout
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_transition_session_to_remove_stickout(msg_st* pmsg)
{
    if(dc.current_ui_name!=SESSION_REMOVE_UI)
    {
        return;
    }
    if(app_get_session_extend_flag()==SESSION_EXTEND_TIME_START)
    {
        if(dc.session_remove_frame<5)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_EXTENDED_START_ADDR+dc.extended_frame*16704,16704);
            dev_oled_set_region(16,20,73,115);
            dis_pic(&flash_rx_buffer[0],16704,0);
        }
        else if(dc.session_remove_frame < 15){
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_TRANSITION_REMOVE_STICK_OUT_START_ADDR+(dc.session_remove_frame-5)*25404,25404);
            dev_oled_set_region(16,20,73,165);
            dis_pic(&flash_rx_buffer[0],25404,0);
        }
        else if(dc.session_remove_frame == 15){
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_TRANSITION_REMOVE_STICK_OUT_START_ADDR+10*25404,25056);
            dev_oled_set_region(16,118,73,261);
            dis_pic(&flash_rx_buffer[0],25056,0);
        }
        else if(dc.session_remove_frame == 16){
            dev_oled_set_region(16,118,73,213);
            dis_pic(&black_temp[0],16704,0);
            dev_oled_set_region(16,256,73,261);
            dis_pic(&black_temp[0],1044,0);
        }
        else if(dc.session_remove_frame < 26){
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_TRANSITION_REMOVE_STICK_OUT_START_ADDR+0x44238+(dc.session_remove_frame-18)*7812,7812);
            dev_oled_set_region(14,214,75,255);
            dis_pic(&flash_rx_buffer[0],7812,0);
            if(dc.session_remove_frame==25)
            {
                LOGD("ui end:session2remove");
            }
        }
    }
    else
    {
        if(dc.session_remove_frame < 10){
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_TRANSITION_REMOVE_STICK_OUT_START_ADDR+dc.session_remove_frame*25404,25404);
            dev_oled_set_region(16,20,73,165);
            dis_pic(&flash_rx_buffer[0],25404,0);
        }else if(dc.session_remove_frame == 10){
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_TRANSITION_REMOVE_STICK_OUT_START_ADDR+10*25404,25056);
            dev_oled_set_region(16,118,73,261);
            dis_pic(&flash_rx_buffer[0],25056,0);
        }
        else if(dc.session_remove_frame == 11){
            dev_oled_set_region(16,118,73,213);
            dis_pic(&black_temp[0],16704,0);
            dev_oled_set_region(16,256,73,261);
            dis_pic(&black_temp[0],1044,0);
        }else if(dc.session_remove_frame < 21){
            spi_flash_buffer_read(&flash_rx_buffer[0],SESSION_TRANSITION_REMOVE_STICK_OUT_START_ADDR+0x44238+(dc.session_remove_frame-12)*7812,7812);
            dev_oled_set_region(14,214,75,255);
            dis_pic(&flash_rx_buffer[0],7812,0);
            if(dc.session_remove_frame==20)
            {
                LOGD("ui end:session2remove");
            }
        }
    }
    dc.session_remove_frame++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh transition session to remove stickout frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_oled_transition_session_to_remove_stickout(const ptimer_t tm)
{
    if(app_get_session_extend_flag()==SESSION_EXTEND_TIME_START)
    {
        if(dc.session_remove_frame>25)
        {
            if(dc.heat_ui_timer){
                if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                    dc.heat_ui_timer = NULL;
                }
            }
            post_msg_to_manager(op_oled_remove_stick_ui);
            return;
        }
    }else
    {
        if(dc.session_remove_frame>20)
        {
            if(dc.heat_ui_timer){
                if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                    dc.heat_ui_timer = NULL;
                }
            }
            post_msg_to_manager(op_oled_remove_stick_ui);
            return;
        }
    }
    post_msg_to_oled(op_oled_session_to_remove);
}
/*************************************************************************************************
    \brief      start oled display for transition_session_to_remove_stickout
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_oled_transition_session_to_remove_stickout(void)
{
    delete_ui_timer();
    dc.current_ui_name=SESSION_REMOVE_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.session_remove_frame = 0;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25,TIMER_OPT_PERIOD, call_back_oled_transition_session_to_remove_stickout);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    post_msg_to_oled(op_oled_session_to_remove);
    LOGD("ui start:session2remove");
}
/*************************************************************************************************
    \brief      handle_oled_over_voltage: to refresh the frames of over voltage
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_over_voltage(msg_st* pmsg)
{
    if(dc.current_ui_name!=OVER_VOLTAGE_UI)
    {
        return;
    }
    if(dc.over_voltage_frame==0)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
        dev_oled_set_region(20,20,69,69);
        dis_pic(&flash_rx_buffer[0],7500,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x90018,15900);
        dev_oled_set_region(20,88,69,193);
        dis_pic(&flash_rx_buffer[0],15900,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x90018+0x3E1C,10032);
        dev_oled_set_region(26,194,63,281);
        dis_pic(&flash_rx_buffer[0],10032,0);
    }
    else if(dc.over_voltage_frame<3)
    {
        //do nothing
    }
    else if(dc.over_voltage_frame<36)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x90018+0x3E1C+10032*(dc.over_voltage_frame-2),10032);
        dev_oled_set_region(26,194,63,281);
        dis_pic(&flash_rx_buffer[0],10032,0);
    }else if(dc.over_voltage_frame==36)
    {
        dev_oled_set_region(26,200,63,281);
        dis_pic(&black_temp[0],9348,0);
    }
    else if(dc.over_voltage_frame<71)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x90018+0x3E1C+10032*(dc.over_voltage_frame-37),10032);
        dev_oled_set_region(26,194,63,281);
        dis_pic(&flash_rx_buffer[0],10032,0);
    }else if(dc.over_voltage_frame==71)
    {
        dev_oled_set_region(26,200,63,281);
        dis_pic(&black_temp[0],9348,0);
    }else if(dc.over_voltage_frame<106)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x90018+0x3E1C+10032*(dc.over_voltage_frame-72),10032);
        dev_oled_set_region(26,194,63,281);
        dis_pic(&flash_rx_buffer[0],10032,0);
    }else if(dc.over_voltage_frame==106)
    {
        dev_oled_set_region(26,200,63,281);
        dis_pic(&black_temp[0],9348,0);
    }
    dc.over_voltage_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh over voltage frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_over_voltage_ui(const ptimer_t tm){
    if(dc.over_voltage_frame>(5000/FPS_25-1))
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        LOGD("ui end:over_voltage");
        post_msg_to_oled(op_oled_clear_black);
        return;
    }
    post_msg_to_oled(op_oled_over_voltage);
}
/*************************************************************************************************
    \brief      start oled display for over voltage animation
    \param[in]  none
    \retval     None
*************************************************************************************************/
void start_oled_over_voltage_ui(void)
{
    dc.heat_state=HEAT_OVER;
    dc.current_ui_name=OVER_VOLTAGE_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.over_voltage_frame=0;
    if(dc.heat_ui_timer)
    {
        delete_ui_timer();
        post_msg_to_oled(op_oled_clear_black);
    }
    else
    {
        post_msg_to_oled(op_oled_over_voltage);
    }
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (FPS_25), TIMER_OPT_PERIOD, call_back_over_voltage_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    LOGD("ui start:over_voltage");
}
/*************************************************************************************************
    \brief      handle_oled_error_code: to refresh the frames of error
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_error_code(msg_st* pmsg){
    if(dc.current_ui_name!=WAIT_ERROR_UI)
    {
        return;
    }
    if(dc.error_frame == 0)
    {
        switch(dc.error_type)
        {
            case ERROR_RETURN:
                LOGD("ui start:RETURN");
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
                dev_oled_set_region(20,20,69,69);
                dis_pic(&flash_rx_buffer[0],7500,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x8A12C,24300);//QR
                dev_oled_set_region(0,96,89,185);
                dis_pic(&flash_rx_buffer[0],24300,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x802C8,6348);
                dev_oled_set_region(22,214,67,259);
                dis_pic(&flash_rx_buffer[0],6348,0);
            break;
            case ERROR_EOL:
                LOGD("ui start:EOL");
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
                dev_oled_set_region(20,20,69,69);
                dis_pic(&flash_rx_buffer[0],7500,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x84240,24300);//QR
                dev_oled_set_region(0,96,89,185);
                dis_pic(&flash_rx_buffer[0],24300,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x81B94,9900);
                dev_oled_set_region(20,204,69,269);
                dis_pic(&flash_rx_buffer[0],9900,0);
            break;
            case ERROR_WAIT_GENERAL:
                LOGD("ui start:WAIT_GENERAL");
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
                dev_oled_set_region(20,20,69,69);
                dis_pic(&flash_rx_buffer[0],7500,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x7B39C,9108);//wait
                dev_oled_set_region(22,108,67,173);
                dis_pic(&flash_rx_buffer[0],9108,0);
            break;
            case ERROR_TOO_COLD:
                LOGD("ui start:TOO_COLD");
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
                dev_oled_set_region(20,20,69,69);
                dis_pic(&flash_rx_buffer[0],7500,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x7B39C,9108);//wait
                dev_oled_set_region(22,108,67,173);
                dis_pic(&flash_rx_buffer[0],9108,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x7D730,5580);//too cold
                dev_oled_set_region(30,210,59,271);
                dis_pic(&flash_rx_buffer[0],5580,0);
            break;
            case ERROR_TOO_HOT:
                LOGD("ui start:TOO_HOT");
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
                dev_oled_set_region(20,20,69,69);
                dis_pic(&flash_rx_buffer[0],7500,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x7B39C,9108);//wait
                dev_oled_set_region(22,108,67,173);
                dis_pic(&flash_rx_buffer[0],9108,0);
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x7ECFC,5580);//too hot
                dev_oled_set_region(30,210,59,271);
                dis_pic(&flash_rx_buffer[0],5580,0);
            break;
            default:
            break;
        }
    }
    else if(dc.error_frame < 5000/FPS_15)
    {
        if(dc.error_type == ERROR_RETURN || dc.error_type == ERROR_EOL)
        {
            dc.error_frame++;
            return;
        }
        if(dc.error_frame<24)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x46C38+(dc.error_frame-1)*5508,5508);//too cold
            dev_oled_set_region(28,114,61,169);
            dis_pic(&flash_rx_buffer[0],5508,0);
        }
        else if(dc.error_frame<30)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x46C38+0x1EEDC+(dc.error_frame-24)*14700,14700);//too cold
            dev_oled_set_region(10,106,79,175);
            dis_pic(&flash_rx_buffer[0],14700,0);
        }
        else if(dc.error_frame<53)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x46C38+(dc.error_frame-30)*5508,5508);//too cold
            dev_oled_set_region(28,114,61,169);
            dis_pic(&flash_rx_buffer[0],5508,0);
        }
        else if(dc.error_frame<59)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x46C38+0x1EEDC+(dc.error_frame-53)*14700,14700);//too cold
            dev_oled_set_region(10,106,79,175);
            dis_pic(&flash_rx_buffer[0],14700,0);
        }
        else if(dc.error_frame==(5000/FPS_15-1))
        {
            switch(dc.error_type)
            {
                case ERROR_WAIT_GENERAL:
                    LOGD("ui end:WAIT_GENERAL");
                break;
                case ERROR_TOO_COLD:
                    LOGD("ui end:TOO_COLD");
                break;
                case ERROR_TOO_HOT:
                    LOGD("ui end:TOO_HOT");
                break;
                default:
                break;
            }
        }
    }
    else if(dc.error_frame==(10000/FPS_15-1))
    {
        switch(dc.error_type)
        {
            case ERROR_RETURN:
                LOGD("ui end:RETURN");
            break;
            case ERROR_EOL:
                LOGD("ui end:EOL");
            break;
            default:
            break;
        }
    }
    dc.error_frame++;
}
/*************************************************************************************************
  * @brief    : Timer call back function to refresh error frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_error_ui(const ptimer_t tm)
{
    if(dc.error_type==ERROR_TOO_HOT||dc.error_type==ERROR_TOO_COLD||dc.error_type==ERROR_WAIT_GENERAL)
    {
        if(dc.error_frame>(5000/FPS_15-1))
        {
            if(dc.heat_ui_timer){
                if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                    dc.heat_ui_timer = NULL;
                }
            }
            post_msg_to_oled(op_oled_clear_black);
            return;
        }
    }
    else
    {
        if(dc.error_frame>(10000/FPS_15-1))
        {
            if(dc.heat_ui_timer){
                if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                    dc.heat_ui_timer = NULL;
                }
            }
            post_msg_to_oled(op_oled_clear_black);
            return;
        }
    }
    post_msg_to_oled(op_error_occur);
}
/*************************************************************************************************
    \brief      start oled display for error
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_error_ui(uint8_t error_type)
{
    delete_ui_timer();
    dc.heat_state=HEAT_OVER;
    dc.current_ui_name=WAIT_ERROR_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.error_frame=0;
    dc.error_type=error_type;
    post_msg_to_oled(op_oled_clear_black);
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (FPS_15), TIMER_OPT_PERIOD, call_back_error_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
}
/*************************************************************************************************
    \brief      handle_oled_cleaning_prompt_timer: to refresh the frames of cleaning
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_cleaning_prompt_timer(msg_st* pmsg)
{
    if(dc.current_ui_name!=CLEANING_UI)
    {
        return;
    }
    if(dc.cleaning_frame<8) //fade in
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CLEANING_START_ADDR+dc.cleaning_frame*28428,28428);
        dev_oled_set_region(22,24,67,229);
        dis_pic(&flash_rx_buffer[0],28428,0);
    }
    else if(dc.cleaning_frame < (8+41))  //top stick
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CLEANING_START_ADDR+0x3E76C+(dc.cleaning_frame-8)*1560,1560);
        dev_oled_set_region(50,76,59,127);
        dis_pic(&flash_rx_buffer[0],1560,0);
    }
    else if(dc.cleaning_frame == (8+41))
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CLEANING_START_ADDR+28428*8,28428);
        dev_oled_set_region(22,24,67,229);
        dis_pic(&flash_rx_buffer[0],28428,0);
    }
    else if(dc.cleaning_frame < (8+41+1+44))
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CLEANING_START_ADDR+0x4E144+(dc.cleaning_frame-(8+41+1))*1560,1560);
        dev_oled_set_region(50,228,59,279);
        dis_pic(&flash_rx_buffer[0],1560,0);
    }
    else if(dc.cleaning_frame < (8+41+1+44+5)) //fade out
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CLEANING_START_ADDR+0x5ED64+(dc.cleaning_frame-(8+41+1+44))*35328,35328);
        dev_oled_set_region(22,24,67,279);
        dis_pic(&flash_rx_buffer[0],35328,0);
    }
    else if(dc.cleaning_frame == (8+41+1+44+5))
    {
        lcd_clear(&black_temp[0]);
        LOGD("ui end:clean");
    }
    else if(dc.cleaning_frame == (8+41+1+44+5+1))
    {
        if(app_get_bat_left()>=6)
        {
            if(app_get_hall_door_status() == door_base)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_MODE_START_ADDR,19968);
                dev_oled_set_region(26,34,57,241);
                dis_pic(&flash_rx_buffer[0],19968,0);
                dc.standard_flag=1;
            }
            else if(app_get_hall_door_status() == door_boost)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_MODE_START_ADDR,21168);
                dev_oled_set_region(26,22,61,217);
                dis_pic(&flash_rx_buffer[0],21168,0);
                dc.boost_flag=1;
            }
        }
    }
    else if(dc.cleaning_frame == (8+41+1+44+5+1+5*1000/40))
    {
        lcd_clear(&black_temp[0]);
        dc.standard_flag=0;
        dc.boost_flag=0;
    }
    dc.cleaning_frame ++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh cleaning frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_cleaning_ui(const ptimer_t tm){
    if(dc.cleaning_frame > (8+41+1+44+5+1+5*1000/40))
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        return;
    }
    post_msg_to_oled(op_oled_cleaning_prompt_timer);
}
/*************************************************************************************************
    \brief      start oled display for cleaning
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_oled_cleaning_prompt(void)
{
    delete_ui_timer();
    dc.current_ui_name=CLEANING_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.cleaning_frame=0;
    post_msg_to_oled(op_oled_clear_black);
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (40), TIMER_OPT_PERIOD, call_back_cleaning_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    LOGD("ui start:clean");
}
/*************************************************************************************************
    \brief      handle_oled_clear_black: to refresh the frames of black_clear
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_clear_black(msg_st* pmsg){
    dc.standard_flag=0;
    dc.boost_flag=0;
    lcd_clear(&black_temp[0]);
    switch(dc.current_ui_name)
    {
        case OPEN_UI:
            dc.brand_frame=0;
        break;
        case MODE_SELECT_UI:
            dc.mode_select_frame=0;
        break;
        case BATTERY_CHECK_UI:
            dc.battery_check_frame=0;
        break;
        case CHARGE_UI:
            dc.charge_frame=0;
        break;
        case CLEANING_UI:
            dc.cleaning_frame=0;
        break;
        case STICK_SENSOR_UI:
            dc.stick_sensor_frame=0;
        break;
        case NO_MODE_CLOSE_UI:
            dc.closing_frame=0;
        break;
        case RESET_ERROR_UI:
            dc.error_reset_frame=0;
        break;
        case WAIT_ERROR_UI:
            dc.error_frame=0;
        break;
        default:
        break;
    }
}
/*************************************************************************************************
    \brief      start oled display for black_clear
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_oled_clear_black(void){
    delete_ui_timer();
    dc.standard_flag=0;
    dc.boost_flag=0;
    clear_all_oled_lock();
    post_msg_to_oled(op_oled_clear_black);
}

void handle_oled_stick_sensor(msg_st* pmsg)
{
    if(dc.current_ui_name!=STICK_SENSOR_UI)
    {
        return;
    }
    if(dc.stick_sensor_status==0)//stick sensor off
    {
        if(dc.stick_sensor_frame<7)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],STICK_SENSOR_START_ADDR+7*44352+dc.stick_sensor_frame*44352,44352);
            dev_oled_set_region(12,22,77,245);
            dis_pic(&flash_rx_buffer[0],44352,0);
        }
        else if(dc.stick_sensor_frame < ((4000-80*7)/80))
        {
            //do nothing
        }
        else if(dc.stick_sensor_frame < (4000/80))
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],STICK_SENSOR_START_ADDR+7*44352+((4000/80-1)-dc.stick_sensor_frame)*44352,44352);
            dev_oled_set_region(12,22,77,245);
            dis_pic(&flash_rx_buffer[0],44352,0);
        }
        else if(dc.stick_sensor_frame == (4000/80))
        {
            lcd_clear(&black_temp[0]);
        }
    }
    else if(dc.stick_sensor_status==1)//stick sensor on
    {
        if(dc.stick_sensor_frame<7)
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],STICK_SENSOR_START_ADDR+dc.stick_sensor_frame*44352,44352);
            dev_oled_set_region(12,22,77,245);
            dis_pic(&flash_rx_buffer[0],44352,0);
        }
        else if(dc.stick_sensor_frame < ((4000-80*7)/80))
        {
            //do nothing
        }
        else if(dc.stick_sensor_frame < (4000/80))
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],STICK_SENSOR_START_ADDR+((4000/80-1)-dc.stick_sensor_frame)*44352,44352);
            dev_oled_set_region(12,22,77,245);
            dis_pic(&flash_rx_buffer[0],44352,0);
        }
        else if(dc.stick_sensor_frame == (4000/80))
        {
            lcd_clear(&black_temp[0]);
        }
    }
    dc.stick_sensor_frame++;
}
static void call_back_stick_sensor_ui(const ptimer_t tm)
{
    if(dc.stick_sensor_frame > (4000/80-1))
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        post_msg_to_oled(op_oled_stick_sensor);
        clear_all_oled_lock();
        return;
    }
    post_msg_to_oled(op_oled_stick_sensor);
}

void start_stick_sensor_ui(uint16_t stick_sensor_status)
{
    delete_ui_timer();
    dc.current_ui_name=STICK_SENSOR_UI;
    dc.stick_sensor_status = stick_sensor_status;
    dc.stick_sensor_frame=0;
    post_msg_to_oled(op_oled_clear_black);
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", 80, TIMER_OPT_PERIOD, call_back_stick_sensor_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    LOGD("stick sensor ui");
}
void change_stick_sensor_ui(uint16_t stick_sensor_status)
{
    delete_ui_timer();
    if(dc.stick_sensor_frame<6)
    {
    }
    else if(dc.stick_sensor_frame<((4000-80*7)/80))
    {
        dc.stick_sensor_frame=5;
    }
    else if(dc.stick_sensor_frame < (4000/80-2))
    {
        dc.stick_sensor_frame=(4000/80-1)-dc.stick_sensor_frame-1;
    }
    else if(dc.stick_sensor_frame >= (4000/80 -2))
    {
        dc.stick_sensor_frame=0;
    }
    dc.stick_sensor_status = stick_sensor_status;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", 80, TIMER_OPT_PERIOD, call_back_stick_sensor_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    post_msg_to_oled(op_oled_stick_sensor);
}

void handle_oled_certfct_graph(msg_st* pmsg)
{
    if(dc.current_ui_name!=CERTFCT_GRAPH_UI)
    {
        return;
    }
    if(certfct_graph_count == 0)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CERTIFICATION_GRAPHICS_START_ADDR,48312);
        dev_oled_set_region(16,20,81,263);
        dis_pic(&flash_rx_buffer[0],48312,0);
        certfct_graph_count++;
    }else if (certfct_graph_count == 1)
    {
        lcd_clear(&black_temp[0]);
        spi_flash_buffer_read(&flash_rx_buffer[0],CERTIFICATION_GRAPHICS_START_ADDR+0xBCB8,38412);
        dev_oled_set_region(12,54,77,247);
        dis_pic(&flash_rx_buffer[0],38412,0);
        certfct_graph_count++;
    }else if (certfct_graph_count == 2)
    {
        lcd_clear(&black_temp[0]);
        spi_flash_buffer_read(&flash_rx_buffer[0],CERTIFICATION_GRAPHICS_START_ADDR+0x152C4,38352);
        dev_oled_set_region(12,52,79,239);
        dis_pic(&flash_rx_buffer[0],38352,0);
        certfct_graph_count++;
    }else if (certfct_graph_count == 3)
    {
        lcd_clear(&black_temp[0]);
        spi_flash_buffer_read(&flash_rx_buffer[0],CERTIFICATION_GRAPHICS_START_ADDR+0x1E894,22464);
        dev_oled_set_region(20,70,71,213);
        dis_pic(&flash_rx_buffer[0],22464,0);
        certfct_graph_count++;
    }else if (certfct_graph_count == 4)
    {
        lcd_clear(&black_temp[0]);
        certfct_graph_count++;
        LOGD("ui end:CE");
    }
}
static void call_back_certfct_graph(const ptimer_t tm){
    if(certfct_graph_count > 4)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        certfct_graph_count = 0;
        return;
    }
    post_msg_to_oled(op_oled_certfct_graph);
}

void start_oled_certfct_graph(void)
{
    delete_ui_timer();
    dc.current_ui_name=CERTFCT_GRAPH_UI;
    certfct_graph_count = 0;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", 5000, TIMER_OPT_PERIOD, call_back_certfct_graph);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    post_msg_to_oled(op_oled_clear_black);
    post_msg_to_oled(op_oled_certfct_graph);
    LOGD("ui start:CE");
}

/*************************************************************************************************
    \brief      handle_oled_error_reset_oneshot: to refresh the frames of error_reset_oneshot
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_error_reset_oneshot(msg_st* pmsg)
{
    if(dc.current_ui_name!=RESET_ERROR_UI)
    {
        return;
    }
    if(dc.error_reset_frame == 0)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
        dev_oled_set_region(20,20,69,69);
        dis_pic(&flash_rx_buffer[0],7500,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],NUM_START_ADDR+1188*2*6+5*1188,1188);//black 22
        dev_oled_set_region(28,226,45,247);
        dis_pic(&flash_rx_buffer[0],1188,0);
        dev_oled_set_region(46,226,63,247);
        dis_pic(&flash_rx_buffer[0],1188,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x7248+dc.error_reset_frame*16128,16128);
        dev_oled_set_region(16,106,71,201);
        dis_pic(&flash_rx_buffer[0],16128,0);
    }
    else if(dc.error_reset_frame < 14)//hand button
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x7248+dc.error_reset_frame*16128,16128);
        dev_oled_set_region(16,106,71,201);
        dis_pic(&flash_rx_buffer[0],16128,0);
    }else if(dc.error_reset_frame == (5*1000/FPS_15-1))
    {
        LOGD("ui end:reset err");
    }
    dc.error_reset_frame++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh error reset oneshot frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_oled_error_reset_oneshot(const ptimer_t tm)
{
    if(dc.error_reset_frame > (5*1000/FPS_15-1))
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                 dc.heat_ui_timer = NULL;
            }
        }
        post_msg_to_oled(op_oled_clear_black);
        return;
    }
    post_msg_to_oled(op_oled_error_reset_oneshot);
}


/*************************************************************************************************
    \brief      start oled display for error_reset_oneshot
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_oled_error_reset_oneshot(void)
{
    delete_ui_timer();
    dc.heat_state=HEAT_OVER;
    dc.current_ui_name=RESET_ERROR_UI;
    post_msg_to_oled(op_oled_clear_black);
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_15,TIMER_OPT_PERIOD, call_back_oled_error_reset_oneshot);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    dc.error_reset_frame = 0;
    LOGD("ui start:reset err");
}


/*************************************************************************************************
    \brief      handle_oled_error_reset_continue: to refresh the frames of error_reset_continue
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_error_reset_continue(msg_st* pmsg)
{
    if(dc.session_frame == 22)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x37B0,7500);//!
        dev_oled_set_region(20,20,69,69);
        dis_pic(&flash_rx_buffer[0],7500,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x7248+13*16128,16128);
        dev_oled_set_region(16,106,71,201);
        dis_pic(&flash_rx_buffer[0],16128,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x54FC,7500);
        dev_oled_set_region(20,212,69,261);
        dis_pic(&flash_rx_buffer[0],7500,0);//white
        spi_flash_buffer_read(&flash_rx_buffer[0],dc.session_frame/10*1296+0x87F0+ERROR_START_ADDR,1296);
        dev_oled_set_region(26,226,43,249);
        dis_pic(&flash_rx_buffer[0],1296,0);
        spi_flash_buffer_read(&flash_rx_buffer[0],dc.session_frame%10*1296+0x87F0+ERROR_START_ADDR,1296);
        dev_oled_set_region(44,226,61,249);
        dis_pic(&flash_rx_buffer[0],1296,0);
    }
    else if(dc.session_frame/10>0)
    {
        if(dc.session_frame/10==1)
        {
            if(dc.session_frame%10==9)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x54FC,7500);
                dev_oled_set_region(20,212,69,261);
                dis_pic(&flash_rx_buffer[0],7500,0);//white
            }
                 spi_flash_buffer_read(&flash_rx_buffer[0],10*1296+0x87F0+ERROR_START_ADDR,1296);
                 dev_oled_set_region(26,226,43,249);
                 dis_pic(&flash_rx_buffer[0],1296,0);
                 spi_flash_buffer_read(&flash_rx_buffer[0],dc.session_frame%10*1296+0x87F0+ERROR_START_ADDR,1296);
                 dev_oled_set_region(44,226,61,249);
                 dis_pic(&flash_rx_buffer[0],1296,0);
         }
         else
         {
             spi_flash_buffer_read(&flash_rx_buffer[0],dc.session_frame/10*1296+0x87F0+ERROR_START_ADDR,1296);
             dev_oled_set_region(28,226,45,249);
             dis_pic(&flash_rx_buffer[0],1296,0);
             spi_flash_buffer_read(&flash_rx_buffer[0],dc.session_frame%10*1296+0x87F0+ERROR_START_ADDR,1296);
             dev_oled_set_region(46,226,63,249);
             dis_pic(&flash_rx_buffer[0],1296,0);
          }
    }
    else if(dc.session_frame%10==9)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR+0x87F0+0x54FC,7500);
        dev_oled_set_region(20,212,69,261);
        dis_pic(&flash_rx_buffer[0],7500,0);//white
        spi_flash_buffer_read(&flash_rx_buffer[0],dc.session_frame%10*1296+0x87F0+ERROR_START_ADDR,1296);
        dev_oled_set_region(36,226,53,249);
        dis_pic(&flash_rx_buffer[0],1296,0);
    }
    else{
        spi_flash_buffer_read(&flash_rx_buffer[0],dc.session_frame%10*1296+0x87F0+ERROR_START_ADDR,1296);
        dev_oled_set_region(36,226,53,249);
        dis_pic(&flash_rx_buffer[0],1296,0);
    }
    dc.session_frame--;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh error reset continue frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_oled_error_reset_continue(const ptimer_t tm)
{
    if(dc.session_frame == 0)
    {
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        post_msg_to_oled(op_oled_clear_black);
        LOGD("ui end:reset countdown");
        return;
    }
    post_msg_to_oled(op_oled_error_reset_continue);
}

/*************************************************************************************************
    \brief      start oled display for error_reset_continue
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_oled_error_reset_continue(void)
{
    delete_ui_timer();
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", 1000,TIMER_OPT_PERIOD, call_back_oled_error_reset_continue);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    dc.session_frame = 22;
    post_msg_to_oled(op_oled_error_reset_continue);
    LOGD("ui start:reset countdown");
}
/*************************************************************************************************
    \brief      handle_oled_shutdown_ui: to refresh the frames of no_mode_close
    \param[in]  message pointer
    \retval     None
*************************************************************************************************/
void handle_oled_shutdown_ui(msg_st* pmsg)
{
    if(dc.current_ui_name!=NO_MODE_CLOSE_UI)
    {
        return;
    }
    if(dc.closing_frame<6)
    {
        //do nothing
    }
    else if(dc.closing_frame<10)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0x18954+5712*(dc.closing_frame-6),5712);
        dev_oled_set_region(4,100,37,155);
        dis_pic(&flash_rx_buffer[0],5712,0);
    }
    else if(dc.closing_frame<27)
    {
        if(dc.closing_frame==10)
        {
            dev_oled_set_region(4,100,37,119);
            dis_pic(&black_temp[0],2040,0);
        }
        spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0x1E294+(dc.closing_frame-10)*11316,11316);
        dev_oled_set_region(4,120,85,165);
        dis_pic(&flash_rx_buffer[0],11316,0);
    }
    else if(dc.closing_frame<31)
    {
        //do nothing
    }
    else if(dc.closing_frame<33)
    {
        spi_flash_buffer_read(&flash_rx_buffer[0],CLOSING_BYE_START_ADDR+0x1E294+0x2EF74+(dc.closing_frame-31)*11316,11316);
        dev_oled_set_region(4,120,85,165);
        dis_pic(&flash_rx_buffer[0],11316,0);
    }
    else if(dc.closing_frame==33)
    {
        dev_oled_set_region(4,120,85,165);
        dis_pic(&black_temp[0],11316,0);
        LOGD("ui end:no_mode_bye");
    }
    dc.closing_frame++;
}/*************************************************************************************************
  * @brief    : Timer call back function to no_mode_close frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_oled_shutdown_ui(const ptimer_t tm)
{
    if(dc.closing_frame>33)
    {
        clear_all_oled_lock();
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        return;
    }
    post_msg_to_oled(op_oled_shutdown_ui);
}
/*************************************************************************************************
    \brief      start oled display for no_mode_close
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_no_mode_closing_animation(void)
{
    dc.heat_state=HEAT_OVER;
    delete_ui_timer();
    dc.current_ui_name=NO_MODE_CLOSE_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.closing_frame=0;
    post_msg_to_oled(op_oled_clear_black);
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (FPS_25), TIMER_OPT_PERIOD, call_back_oled_shutdown_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    LOGD("ui start:no_mode_bye");
}
/*************************************************************************************************
    \brief      start_oled_wrong_charger
    \param[in]  None
    \retval     None
*************************************************************************************************/
void start_oled_wrong_charger(void)
{
    delete_ui_timer();
    dc.heat_state=HEAT_OVER;
    dc.standard_flag=0;
    dc.boost_flag=0;
    post_msg_to_oled(op_oled_clear_black);
    post_msg_to_oled(op_oled_wrong_charger);
    LOGD("ui start:wrong charge");
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", 5000, TIMER_OPT_ONESHOT, call_back_black_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
}

/*************************************************************************************************
    \brief      handle_oled_wrong_charger
    \param[in]  None
    \retval     None
**************************************************************************************************/
void handle_oled_wrong_charger(msg_st* pmsg)
{
    spi_flash_buffer_read(&flash_rx_buffer[0],ERROR_START_ADDR,34800);
    dev_oled_set_region(20,20,69,251);
    dis_pic(&flash_rx_buffer[0],34800,0);
}

void handle_oled_get_flash_version(msg_st* pmsg)
{
    lcd_clear(&black_temp[0]);
    spi_flash_buffer_read(&flash_rx_buffer[0],VERSION_IMAGE_START_ADDR+64,VERSION_IMAGE_SIZE);
    dev_oled_set_region(10,46,79,245);
    dis_pic(&flash_rx_buffer[0],VERSION_IMAGE_SIZE,0);
}

void get_flash_version(void)
{
    delete_ui_timer();
    post_msg_to_oled(op_oled_flash_version);
}

void dis_color_msg(uint8_t oled_color)
{
    delete_ui_timer();
    post_msg_to_oled_with_arg(op_oled_dis_color,oled_color);
}

void handle_oled_dis_color(msg_st* pmsg)
{
    uint8_t oled_color = pmsg->value;
    app_show_oled_color(oled_color);
}

void dis_color(uint16_t rvalues,uint16_t gvalues,uint16_t bvalues)
{
    for(int i=0;i<90*282;i++)
    {
        flash_rx_buffer[3*i] = rvalues;
        flash_rx_buffer[3*i+1]  = gvalues;
        flash_rx_buffer[3*i+2] = bvalues;
    }
    lcd_clear(&flash_rx_buffer[0]);
}

void app_show_oled_color(uint8_t oled_color)
{
    switch(oled_color)
    {
        case OLED_WHITE:
            dis_color(0XFF,0XFF,0XFF);
            break;
        case OLED_BLACK:
            dis_color(0X00,0X00,0X00);
            break;
        case OLED_RED:
            dis_color(0XFF,0X00,0X00);
            break;
        case OLED_GREEN:
            dis_color(0X00,0XFF,0X00);
            break;
        case OLED_BLUE:
            dis_color(0X00,0X00,0XFF);
            break;
        default:
            break;
    }
}
void handle_oled_over_time_ui(msg_st* pmsg)
{
    if(dc.current_ui_name!=CHARGE_TIME_OUT_UI)
    {
        return;
    }
    uint32_t battery_color_add =0;
    uint32_t battery_lightning_addr =0;
    uint16_t battery_lightning_size =0;
    int8_t fill_row=0;
    if(dc.battery_level>=0&&dc.battery_level<5){
        fill_row=5;
    }
    else if(dc.battery_level<29)
    {
        fill_row=dc.battery_level;
    }
    else if(dc.battery_level==29)
    {
        fill_row=28;
    }
    else if(dc.battery_level<59)
    {
        fill_row=dc.battery_level-1;
    }
    else if(dc.battery_level==59)
    {
        fill_row=57;
    }
    else if(dc.battery_level<100)
    {
        fill_row=dc.battery_level-2;
    }
    else if(dc.battery_level==100)
    {
        fill_row=97;
    }
    if(dc.battery_level>=0&&dc.battery_level<10)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR;
    else if(dc.battery_level<30)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*6;
    else if(dc.battery_level<60)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*12;
    else if(dc.battery_level<=100)
        battery_color_add=NORMAL_SLOW_CHARGE_START_ADDR+15456*18;
    if(dc.battery_level<=100)
    {
        if(dc.charge_time_out_frame<6) //fade in
        {
            battery_fade_in_out(dc.charge_time_out_frame,0,0,1);//%,num fade in
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+dc.charge_time_out_frame*15456,15456);//
            count_row(0,&flash_rx_buffer[0],NULL,0);//boder fade in
        }
        else if(dc.charge_time_out_frame<(6+12)){//fill
            if(dc.battery_level<10)
            {
                if(dc.charge_time_out_frame<10)
                {
                    spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+0x6AFB0+(dc.charge_time_out_frame-6)*15456,15456);
                }
                else
                {
                    spi_flash_buffer_read(&flash_rx_buffer[0],NORMAL_SLOW_CHARGE_START_ADDR+5*15456,15456);
                }
            }
            else{
                spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+5*15456,15456);
            }
            count_row(battery_fill[dc.charge_time_out_frame-6]*fill_row/(12*100),&flash_rx_buffer[0],NULL,0);
        }
        else if(dc.charge_time_out_frame==(6+12))
        {
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+5*15456,15456);
            count_row(fill_row,&flash_rx_buffer[0],NULL,0);
        }
        else if(dc.charge_time_out_frame<((DISPLAY_DURATION*1000-40*12)/FPS_15-5+12))
        {
            //do nothing
        }
        else if(dc.charge_time_out_frame<(((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)) //fade out
        {
            battery_fade_in_out((((DISPLAY_DURATION*1000-40*12)/FPS_15)-1+12)-dc.charge_time_out_frame,0,0,1);//%,num fade out
            spi_flash_buffer_read(&flash_rx_buffer[0],battery_color_add+((((DISPLAY_DURATION*1000-40*12)/FPS_15)-1+12)-dc.charge_time_out_frame)*15456,15456);
            count_row(fill_row,&flash_rx_buffer[0],NULL,0);//border fade out
        }
        else if(dc.charge_time_out_frame==(((DISPLAY_DURATION*1000-40*12)/FPS_15)+12))
        {
            lcd_clear(&black_temp[0]);
            LOGD("ui end:charge_time_out");
        }
        else if(dc.charge_time_out_frame==(((DISPLAY_DURATION*1000-40*12)/FPS_15)+1+12))
        {
            uint8_t hall_flag = app_get_hall_door_status();
            if(hall_flag == door_base)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],STANDARD_MODE_START_ADDR,19968);
                dev_oled_set_region(26,34,57,241);
                dis_pic(&flash_rx_buffer[0],19968,0);
                dc.standard_flag=1;
            }
            else if(hall_flag == door_boost)
            {
                spi_flash_buffer_read(&flash_rx_buffer[0],BOOST_MODE_START_ADDR,21168);
                dev_oled_set_region(26,22,61,217);
                dis_pic(&flash_rx_buffer[0],21168,0);
                dc.boost_flag=1;
            }
        }
        else if(dc.charge_time_out_frame==((((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)+(5*1000/FPS_15)))
        {
            lcd_clear(&black_temp[0]);
            dc.standard_flag=0;
            dc.boost_flag=0;
        }
    }
    dc.charge_time_out_frame++;
}

/*************************************************************************************************
  * @brief    : Timer call back function to refresh charging_check frame UI op_msg
  * @param1   : Pointer to timer structure
  * @return   : None
*************************************************************************************************/
static void call_back_over_time_ui(const ptimer_t tm){
    if(dc.charge_time_out_frame==5)
    {
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_25, TIMER_OPT_PERIOD, call_back_over_time_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    }
    else if(dc.charge_time_out_frame==17)
    {
        dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", FPS_15, TIMER_OPT_PERIOD, call_back_over_time_ui);
        bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    }
    else if(dc.charge_time_out_frame==((DISPLAY_DURATION*1000-40*12)/FPS_15-1+12))
    {
        clear_all_oled_lock();
    }
    else if(dc.charge_time_out_frame>((((DISPLAY_DURATION*1000-40*12)/FPS_15)+12)+(5*1000/FPS_15))){
        if(dc.heat_ui_timer){
            if(bat_timer_delete(dc.heat_ui_timer, portMAX_DELAY)==pdPASS){
                dc.heat_ui_timer = NULL;
            }
        }
        post_msg_to_oled(op_oled_clear_black);
        return;
    }
    post_msg_to_oled(op_oled_over_time);
}



/*************************************************************************************************
    \brief      start oled display for charge time out
    \param[in]  battery_soc: specify the bat_left(00 - 100)
    \retval     None
*************************************************************************************************/
void start_oled_charge_over_time_ui(uint8_t battery_soc)
{
    delete_ui_timer();
    post_msg_to_oled(op_oled_clear_black);
    dc.current_ui_name=CHARGE_TIME_OUT_UI;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.battery_level = battery_soc;
    dc.charge_time_out_frame=0;
    dc.heat_ui_timer = bat_timer_reset_ext(dc.heat_ui_timer, "heat_ui_timer", (66), TIMER_OPT_PERIOD, call_back_over_time_ui);
    bat_timer_start(dc.heat_ui_timer, portMAX_DELAY);
    if(dc.battery_level>=0&&dc.battery_level<=100)
    {
        LOGD("ui start:charge_time_out");
    }else{
        LOGD("illegal soc %d",battery_soc);
    }
}
void oled_ui_parm_init(void)
{
    dc.heat_state=HEAT_OVER;
    dc.standard_flag=0;
    dc.boost_flag=0;
    dc.cancel_count=0;
    dc.heat_cancel_flag=0;
    dc.current_ui_name=NO_UI;
}
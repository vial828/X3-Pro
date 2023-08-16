#include "kernel.h"
#include "log.h"
#include "manager.h"
#include "HWI_Hal.h"
#include "app_button.h"
#include "error_code_led_output.h"
#include "HWI_gpio.h"
#include "batTimer.h"
#define BTN_PRESS_1S   1*1000
#define BTN_PRESS_1P5S   (1.5*1000 - 100)
#define BTN_PRESS_2S   (2*1000 - 100)
#define BTN_PRESS_3S   (3*1000 - 100)
#define BTN_PRESS_5S   (5*1000 - 100)
#define BTN_PRESS_20S   (20*1000 - 100)

#define BTN_DOWN_MODE 0
#define BTN_UP_MODE   1

#define MODE_BASE     1
#define MODE_BOOST    0

#define BTN_NUM 2
#define BUTTON_GAP 350

hall_status_e  hall_door_last_status = door_close;
static uint8_t  g_btn_cnt;
static uint8_t  oneshort_onelong_flag;
btn_status_e btn_status = btn_up;
uint8_t btn_check_time = 0;

ptimer_t button_timer;

/*************************************************************************************************
 * @brief  :read hall door value
 * @return :btn_down or btn_up
*************************************************************************************************/
static hall_status_e read_hall_door(void)
{
    hall_status_e  hall_door_status;
    static uint8_t check_door_cnt = 0;
    static uint8_t check_mode_cnt = 0;
    static hwi_GPIO_Value hall_door_last = 0;
    static hwi_GPIO_Value hall_mode_last = 0;
    static hwi_GPIO_Value hall_mode = 0;
    static hwi_GPIO_Value hall_door = 0;
    static uint8_t first_flag = 0;
    hwi_GPIO_Value hall_door_new = hwi_GPIO_ReadPin(HALL_INT_DOOR_E);
    hwi_GPIO_Value hall_mode_new = hwi_GPIO_ReadPin(HALL_INT_MODE_E);
    if(first_flag == 0)
    {
        hall_door = hwi_GPIO_ReadPin(HALL_INT_DOOR_E);
        hall_mode = hwi_GPIO_ReadPin(HALL_INT_MODE_E);
        first_flag = 1;
    }
    if(hall_mode_new == hall_mode_last)
    {
        check_mode_cnt++;
    }
    else
    {
        check_mode_cnt=0;
    }
    if(hall_door_new == hall_door_last)
    {
        check_door_cnt++;
    }
    else
    {
        check_door_cnt= 0;
    }
    if(check_mode_cnt == 5)
    {
        hall_mode = hall_mode_new;
        check_mode_cnt=0;
    }
    if(check_door_cnt == 5)
    {
        hall_door = hall_door_new;
        check_door_cnt= 0;
    }
    //LOGD("check_door_cnt =%d,check_mode_cnt =%d",check_door_cnt,check_mode_cnt);
    if( HWI_PIN_RESET == hall_door && HWI_PIN_SET == hall_mode)
    {
        hall_door_status = door_close;
    }
    else if( HWI_PIN_SET == hall_door && HWI_PIN_RESET == hall_mode)
    {
        hall_door_status = door_boost;
    }
    else if( HWI_PIN_SET == hall_door && HWI_PIN_SET == hall_mode)
    {
        hall_door_status = door_base;
    }
    else
    {
        hall_door_status = door_error; 
    }
    hall_door_last = hall_door_new;
    hall_mode_last = hall_mode_new;
    return hall_door_status;
}

/*************************************************************************************************
 * @brief  :retun the hall door status
 * @return :void
*************************************************************************************************/
hall_status_e app_get_hall_door_status(void)
{
    return hall_door_last_status ;
}

/*************************************************************************************************
 * @brief  :init button gpio and global flags
 * @return :void
*************************************************************************************************/
void app_hall_door_init(void)
{
    hall_door_last_status = read_hall_door();
    LOGD("DOOR_INIT: %d", hall_door_last_status);
}

/*************************************************************************************************
 * @brief  :hall door task that will send door moving status to manager
 * @return :void
*************************************************************************************************/
void app_hall_door_task(void)
{
    hall_status_e  hall_door_new_status = read_hall_door();

    if(hall_door_new_status == door_error && hall_door_last_status != door_error)
    {
        //flt_de_hall_error
        //post_msg_to_manager_with_arg(op_error_occur, ...);
        LOGD("DOOR_STATUS: ERROR OCCUR");
        hall_door_last_status = hall_door_new_status;
        return;
    }
    else if(hall_door_new_status != door_error && hall_door_last_status == door_error)
    {
        //flt_de_hall_error
        //post_msg_to_manager_with_arg(op_error_clear, ...);
        LOGD("DOOR_STATUS: ERROR CLEAR");
    }

    if(hall_door_new_status == door_base)
    {
        if(hall_door_last_status == door_boost)
        {
            //op_boost2base
            post_msg_to_manager_with_arg(op_hall_door_mode,boost2base);
            LOGD("DOOR_STATUS: boost2base");
        }
        else if(hall_door_last_status == door_close)
        {
            //op_close2base
            post_msg_to_manager_with_arg(op_hall_door_mode,close2base);
            LOGD("DOOR_STATUS: close2base");
        }
        else
        {  /*as base is in the middle, if door_error, we donot know door 
            was moved from close or boost, so keep it as not changing*/ 

            //hall door NOT changing
        }
    }
    else if(hall_door_new_status == door_boost)
    {
        if(hall_door_last_status == door_base)
        {
            //op_base2boost
            post_msg_to_manager_with_arg(op_hall_door_mode,base2boost);
            LOGD("DOOR_STATUS: base2boost");
        }
        else if(hall_door_last_status == door_close)
        {
            //op_close2base
            post_msg_to_manager_with_arg(op_hall_door_mode,close2base);
            //op_base2boost
            post_msg_to_manager_with_arg(op_hall_door_mode,base2boost);
            LOGD("DOOR_STATUS: close2boost");
        }
        else if(hall_door_last_status == door_error)
        {
            //op_close2base
            post_msg_to_manager_with_arg(op_hall_door_mode,close2base);
            //op_base2boost
            post_msg_to_manager_with_arg(op_hall_door_mode,base2boost);
            LOGD("DOOR_STATUS: close2boost");
        }
        else
        {
            //hall door NOT changing
        }
    }
    else if(hall_door_new_status == door_close)
    {
        if(hall_door_last_status == door_base)
        {
            //op_base2close
            post_msg_to_manager_with_arg(op_hall_door_mode,base2close);
            LOGD("DOOR_STATUS: base2close");
        }
        else if(hall_door_last_status == door_boost)
        {
            //op_boost2base
            post_msg_to_manager_with_arg(op_hall_door_mode,boost2base);
            //op_base2close
            post_msg_to_manager_with_arg(op_hall_door_mode,base2close);
            LOGD("DOOR_STATUS: boost2base2close");
        }
        else if(hall_door_last_status == door_error)
        {
            //op_boost2base
            post_msg_to_manager_with_arg(op_hall_door_mode,boost2base);
            //op_base2close
            post_msg_to_manager_with_arg(op_hall_door_mode,base2close);
            LOGD("DOOR_STATUS: error2base2close");
        }
        else
        {
            //hall door NOT changing
        }
    }
    //back up the door status
    hall_door_last_status = hall_door_new_status;
  
}

void button_status_check(void)
{
    static uint8_t btn_status_temp[2];
    if(btn_check_time<2)
    {
       btn_status_temp[btn_check_time] = app_read_button();
       btn_check_time++;
    }
     if(btn_check_time % 2 == 0){
          if((btn_status_temp[0]==btn_status_temp[1]))
          {
              btn_check_time = 0;
              btn_status = app_read_button();
          }else{
              btn_status_temp[0]=btn_status_temp[1];
              btn_check_time = 1;
          }
     }

}


/*************************************************************************************************
 * @brief  :read button value
 * @return :btn_down or btn_up
*************************************************************************************************/
btn_status_e app_read_button(void)
{
    return (hwi_GPIO_ReadPin(SWITCH_IN_E)? btn_down:btn_up);
}
#if 1
static void call_back_button_gap(const ptimer_t tm)
{
    if(g_btn_cnt < 3){
        g_btn_cnt = 0;
        if(app_read_button() == btn_up){
            //oneshort_onelong_flag = 0;
            post_msg_to_manager(op_btn_short_up);
        }
        if(app_read_button() == btn_down){
            //oneshort_onelong_flag = 1;
        }
    }
    if(button_timer)
    {
        if(bat_timer_delete(button_timer, portMAX_DELAY)==pdPASS){
            button_timer = NULL;
        }
    }
}

static void button_times_handle(void)
{
    g_btn_cnt++;
    switch (g_btn_cnt){
        case 1:
            button_timer = bat_timer_reset_ext(button_timer, "button_timer",BUTTON_GAP, TIMER_OPT_ONESHOT, call_back_button_gap);
            bat_timer_start(button_timer, portMAX_DELAY);
        break;
        case 2:
            button_timer = bat_timer_reset_ext(button_timer, "button_timer",BUTTON_GAP, TIMER_OPT_ONESHOT, call_back_button_gap);
            bat_timer_start(button_timer, portMAX_DELAY);
        break;
        case 3:
            if(button_timer)
            {
                if(bat_timer_delete(button_timer, portMAX_DELAY)==pdPASS){
                    button_timer = NULL;
                }
            }
            post_msg_to_manager(op_btn_3short);
            g_btn_cnt = 0;
        break;
        default:
            g_btn_cnt = 0;
        break;
    }
}
#endif


/*************************************************************************************************
 * @brief  :init button gpio and global flags
 * @return :void
*************************************************************************************************/
void app_button_init(void)
{
//    GPIO_InitTypeDef  GPIO_InitStruct;
//
//    /* MODE BUTTON GPIO pin configuration  */
//    GPIO_InitStruct.Pin       = MODE_BTN_PIN;
//    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull      = GPIO_NOPULL;
//
//    hwi_GPIO_Init(MODE_BTN_PORT, &GPIO_InitStruct);
//
//    /* START BUTTON GPIO pin configuration  */
//    GPIO_InitStruct.Pin       = START_BTN_PIN;
//    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull      = GPIO_NOPULL;
//
//    hwi_GPIO_Init(START_BTN_PORT, &GPIO_InitStruct);
//	hwi_GPIO_Init();

}


/*************************************************************************************************
 * @brief  :button detect task that will send button msg to manager
 * @return :void
*************************************************************************************************/
void app_button_task(void)
{
    static uint8_t btn_state = BTN_UP_MODE;
    //btn_status_e btn_status;
    static uint32_t btn_down_tick;
    static uint32_t btn_down_long_flag = 0;

    static uint8_t wakeup = 0;
    if(!wakeup){
        wakeup = 1;
        if(app_read_button() == btn_up){
            post_msg_to_manager(op_btn_short_up);
        }
    }
    //btn_status = app_read_button();
    button_status_check();
    switch(btn_state)
    {
        case BTN_DOWN_MODE:
            if(btn_status == btn_up){
                btn_state = BTN_UP_MODE;
                if(TicsSince(btn_down_tick) >= BTN_PRESS_1P5S){
                        post_msg_to_manager(op_btn_1s5_up);
                }else if(TicsSince(btn_down_tick) < BTN_PRESS_1P5S){
                    button_times_handle();
                    //post_msg_to_manager(op_btn_short_up);
                }
            }else{
                if(TicsSince(btn_down_tick)>= BTN_PRESS_1P5S && btn_down_long_flag == 0){
                    btn_down_long_flag = BTN_PRESS_1P5S;
                    post_msg_to_manager(op_btn_1s5_down);
                }else if(TicsSince(btn_down_tick)>= BTN_PRESS_3S && btn_down_long_flag == BTN_PRESS_1P5S){
                    btn_down_long_flag = BTN_PRESS_3S;
                    post_msg_to_manager(op_btn_3s_down);
//                    if(oneshort_onelong_flag == 1)
//                    {
//                        post_msg_to_manager(op_btn_oneshort_onelong);
//                    }
                }else if(TicsSince(btn_down_tick)>= BTN_PRESS_5S && btn_down_long_flag == BTN_PRESS_3S){
                    btn_down_long_flag = BTN_PRESS_5S;
                    post_msg_to_manager(op_btn_5s_down);
                }
            }
        break;

        case BTN_UP_MODE:
            if(btn_status == btn_down){
                 btn_down_tick = GetTick();
                 btn_state = BTN_DOWN_MODE;
                 post_msg_to_manager(op_btn_short_down);
                 btn_down_long_flag = 0;
            }
        break;

        default:
        break;
    }
}

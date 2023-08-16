#include "error_code_led_output.h"
#include "spi_flash.h"
#include "manager.h"
#include "app_haptic.h"
#include "log.h"
#include "usr_cmd.h"
#include "power.h"
#include "app_charge.h"
#include "batTimer.h"

//timer_t * g_error_timer = NULL;
//timer_t * err_exist_timer = NULL;
//static timer_t * g_error_haptic_timer = NULL;
ptimer_t g_error_timer = NULL;
ptimer_t err_exist_timer = NULL;
static ptimer_t g_error_haptic_timer = NULL;

static err_type_e current_err_type = NO_SUCH_ERR;

/*************************************************************************************************
  * @brief    : Get a position of one of the existing errors
  * @param1   : The value that represents the existing errors
  * @return   : 0xFF(no existing error)/other value(a position of one of the existing errors)
*************************************************************************************************/
uint8_t get_error_pos(uint32_t value)
{
    uint8_t i;
    uint32_t temp=value;

    for(i=0;i<32;i++)
    {
        if(temp &0x00000001)
            return i;
        temp= temp>>1;
    }
    return 0xFF;
}

/*************************************************************************************************
  * @brief    : Timer call back
  * @param1   : Pointer to the timer structure
  * @param2   : Pointer to the parameter that will be transfered to call back function
  * @return   : None
*************************************************************************************************/
static void call_back_recover_err_UI_over(const ptimer_t tm)
{
    //TIMER_SAFE_DELETE(g_error_timer);
    if(g_error_timer){
        if(bat_timer_delete(g_error_timer, portMAX_DELAY)==pdPASS){
            g_error_timer = NULL;
        }
    }
    post_msg_to_manager(op_error_led_output_finish);
}

/*************************************************************************************************
  * @brief    : Timer call back
  * @param1   : Pointer to the timer structure
  * @param2   : Pointer to the parameter that will be transfered to call back function
  * @return   : None
*************************************************************************************************/
static void call_back_reset_or_return_err_UI_over(const ptimer_t tm)
{
    //TIMER_SAFE_DELETE(g_error_timer);
    if(g_error_timer){
        if(bat_timer_delete(g_error_timer, portMAX_DELAY)==pdPASS){
            g_error_timer = NULL;
        }
    }    
}

static void call_back_err_exist_time_over(const ptimer_t tm)
{
    if(current_err_type == RETURN_ERR || current_err_type == RESET_ERR){
        LOG_NOW("system shutdown");
        power_shutdown_mode();
    }
    else if( current_err_type == RECOVER_ERR){
        if(!app_check_usb_plug_status()){
            LOG_NOW("system shutdown");
            power_shutdown_mode();
        }
    }
    else{
        LOGE("no such error type");
    }
    //TIMER_SAFE_DELETE(err_exist_timer);
    if(err_exist_timer){
        if(bat_timer_delete(err_exist_timer, portMAX_DELAY)==pdPASS){
            err_exist_timer = NULL;
        }
    }

}

err_type_e record_err_type(uint8_t errorPos)
{
    err_type_e error_type = NO_SUCH_ERR;
    boot_record_t *brt = get_boot_record_from_ram();

    /* Return */
    if(errorPos == flt_de_bat_damage || errorPos == flt_de_cic_output_voltage || errorPos == flt_de_end_of_life)
    {
        error_type = RETURN_ERR;
        brt->return_err = ERR_EXIST;
        brt->error_pos = errorPos;
        update_data_flash(BOOT_RECORD, INVALID);
    }
    /* Reset */
    else if(errorPos == flt_de_bat_discharge_current_over ||errorPos == flt_de_bat_charge_current_over || errorPos == flt_de_cic_config_error)
    {
        error_type = RESET_ERR;
        brt->reset_err = ERR_EXIST;
        brt->error_pos = errorPos;
        update_data_flash(BOOT_RECORD, INVALID);
    }
    else if(errorPos == war_de_bat_low)
    {
        error_type = LOW_BATV_ERR;
    }
    /* Wait recoverable */
    else
    {
        error_type = RECOVER_ERR;
    }

    return error_type;
}

/*************************************************************************************************
  * @brief    : Timer call back
  * @param1   : Pointer to the timer structure
  * @param2   : Pointer to the parameter that will be transfered to call back function
  * @return   : None
*************************************************************************************************/
//static void call_back_haptic_intensity_restore(const ptimer_t tm)
//{
//    //TIMER_SAFE_DELETE(g_error_haptic_timer);
//    if(g_error_haptic_timer){
//        if(bat_timer_delete(g_error_haptic_timer, portMAX_DELAY)==pdPASS){
//            g_error_haptic_timer = NULL;
//        }
//    }
//    app_set_haptic_intensity(haptic_intensity_record);
//}


/*************************************************************************************************
  * @brief    : Start error code haptic display based on the error type
  * @param1   : error type
  * @return   : None
*************************************************************************************************/
void start_error_haptic_UI(err_type_e error_type)
{
    //uint32_t haptic_d_duration = 3150;

    if(error_type == RETURN_ERR || error_type == RESET_ERR || error_type == RECOVER_ERR)
    {
        //haptic_intensity_record = app_get_haptic_intensity();
        //app_set_haptic_intensity(HP_HALF_INTENSITY_LEVEL);
        app_haptic_buzz('d');
        /* plus 50ms in case aw86224 restore intensity fail */
        //g_error_haptic_timer = timer_create(haptic_d_duration + 50, TIMER_OPT_ONESHOT, call_back_haptic_intensity_restore, NULL);
        //g_error_haptic_timer = bat_timer_reset_ext(g_error_haptic_timer, "g_error_haptic_timer", haptic_d_duration + 50, TIMER_OPT_ONESHOT, call_back_haptic_intensity_restore);
        //bat_timer_start(g_error_haptic_timer, portMAX_DELAY);
    }
    else if(error_type == LOW_BATV_ERR)
    {
        app_haptic_buzz('a');
    }
    else
    {
        LOGE("No such error");
    }
}
/*************************************************************************************************
  * @brief    : Start error code led display based on the error type
  * @param1   : error type
  * @return   : None
*************************************************************************************************/
void start_error_led_output(err_type_e error_type)
{
    switch(error_type)
    {
        case RETURN_ERR:/* Return */
        {
            //g_error_timer = timer_create(err_ui_duration, TIMER_OPT_ONESHOT, call_back_reset_or_return_err_UI_over, NULL);
            g_error_timer = bat_timer_reset_ext(g_error_timer, "g_error_timer", ERR_QR_TIME, TIMER_OPT_ONESHOT, call_back_reset_or_return_err_UI_over);
            bat_timer_start(g_error_timer, portMAX_DELAY);
            break;
        }
        case RESET_ERR:/* Reset */
        {
            //g_error_timer = timer_create(err_ui_duration, TIMER_OPT_ONESHOT, call_back_reset_or_return_err_UI_over, NULL);
            g_error_timer = bat_timer_reset_ext(g_error_timer, "g_error_timer", ERR_UI_TIME, TIMER_OPT_ONESHOT, call_back_reset_or_return_err_UI_over);
            bat_timer_start(g_error_timer, portMAX_DELAY);
            break;
        }
        case RECOVER_ERR:/* Wait recoverable */
        {
            //g_error_timer = timer_create(err_ui_duration, TIMER_OPT_ONESHOT, call_back_recover_err_UI_over, NULL);
            g_error_timer = bat_timer_reset_ext(g_error_timer, "g_error_timer", ERR_UI_TIME, TIMER_OPT_ONESHOT, call_back_recover_err_UI_over);
            bat_timer_start(g_error_timer, portMAX_DELAY);
            break;
        }
        case LOW_BATV_ERR:
        {
            //g_error_timer = timer_create(err_ui_duration, TIMER_OPT_ONESHOT, call_back_reset_or_return_err_UI_over, NULL);
            g_error_timer = bat_timer_reset_ext(g_error_timer, "g_error_timer", ERR_LOW_BATV_TIME, TIMER_OPT_ONESHOT, call_back_recover_err_UI_over);
            bat_timer_start(g_error_timer, portMAX_DELAY);
            break;
        }
        default:
            break;
    }

    start_error_haptic_UI(error_type);
    current_err_type = error_type;
    //TIMER_SAFE_RESET(err_exist_timer, ERR_EXIST_TIME, TIMER_OPT_ONESHOT, call_back_err_exist_time_over, NULL);
    err_exist_timer = bat_timer_reset_ext(err_exist_timer, "err_exist_timer", ERR_EXIST_TIME, TIMER_OPT_ONESHOT, call_back_err_exist_time_over);
    bat_timer_start(err_exist_timer, portMAX_DELAY);
}

/*************************************************************************************************
  * @brief    : Report error code through uart
  * @param1   : Error code
  * @return   : None
*************************************************************************************************/
void upload_error(errorCode_e error_code)
{
    uint8_t error = (uint8_t)error_code;
    if (0 == get_upload_error_flag())
    {
        return;
    }
    else
    {
        LOGD("upload type[%d] error!",error);
        //error -= 2;
        respond_usr_cmd(USR_CMD_READ_CURRENT_ERROR, &error, 1);
    }
}





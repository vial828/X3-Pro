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

#define BEGIN_OLED_MESSAGE_MAP  const static msgnode_st  oled_msg_node_array[]=  {         
#define ADD_NEW_OLED_MSG_ITEM(MsgOpcode,OnMsg)                               {MsgOpcode,OnMsg},
#define END_OLED_MESSAGE_MAP                                                 };      

static QueueHandle_t oledQueueHandle = NULL;


BEGIN_OLED_MESSAGE_MAP
ADD_NEW_OLED_MSG_ITEM(op_oled_brand, handle_oled_brand)
ADD_NEW_OLED_MSG_ITEM(op_oled_reboot, handle_oled_reboot)
ADD_NEW_OLED_MSG_ITEM(op_oled_over_time, handle_oled_over_time_ui)
//ADD_NEW_OLED_MSG_ITEM(op_hall_door_mode_start_timer, handle_oled_start_mode_timer)
ADD_NEW_OLED_MSG_ITEM(op_hall_door_mode, handle_oled_door_mode)
//ADD_NEW_OLED_MSG_ITEM(op_oled_heat, handle_oled_heat)
ADD_NEW_OLED_MSG_ITEM(op_oled_mode_to_heatup, handel_transition_mode_to_heatup)
ADD_NEW_OLED_MSG_ITEM(op_oled_warm_up, handle_oled_heat_up)
ADD_NEW_OLED_MSG_ITEM(op_oled_heatup_session, handle_transition_heatup_to_session)
ADD_NEW_OLED_MSG_ITEM(op_oled_session, handle_oled_session)
ADD_NEW_OLED_MSG_ITEM(op_oled_dis_extend_ui, handle_display_session_extended_ui)
ADD_NEW_OLED_MSG_ITEM(op_oled_enter_extended, handle_enter_session_extended_ui)
ADD_NEW_OLED_MSG_ITEM(op_oled_warm_up_count_down, handle_oled_warm_up_count_down)
ADD_NEW_OLED_MSG_ITEM(op_oled_session_count_down, handle_oled_session_count_down)
//ADD_NEW_OLED_MSG_ITEM(op_oled_battery_check_timer, handle_oled_start_battery_check_timer)
ADD_NEW_OLED_MSG_ITEM(op_oled_battery_check, handle_oled_battery_check)
//ADD_NEW_OLED_MSG_ITEM(op_oled_charging_check_timer, handle_oled_start_charging_check_timer)
ADD_NEW_OLED_MSG_ITEM(op_oled_charging_check, handle_oled_charging_check)
ADD_NEW_OLED_MSG_ITEM(op_oled_cancel_heatup, handle_oled_heat_up_cancel_ui)
ADD_NEW_OLED_MSG_ITEM(op_oled_cancel_session, handle_cancel_session_ui)
ADD_NEW_OLED_MSG_ITEM(op_oled_heatup_remove, handel_transition_heatup_to_remove)
ADD_NEW_OLED_MSG_ITEM(op_oled_session_to_remove, handle_oled_transition_session_to_remove_stickout)
ADD_NEW_OLED_MSG_ITEM(op_error_occur, handle_oled_error_code)
ADD_NEW_OLED_MSG_ITEM(op_oled_cleaning_prompt_timer, handle_oled_cleaning_prompt_timer)
ADD_NEW_OLED_MSG_ITEM(op_oled_clear_black, handle_oled_clear_black)
ADD_NEW_OLED_MSG_ITEM(op_oled_certfct_graph, handle_oled_certfct_graph)
ADD_NEW_OLED_MSG_ITEM(op_oled_error_reset_oneshot, handle_oled_error_reset_oneshot)
ADD_NEW_OLED_MSG_ITEM(op_oled_error_reset_continue, handle_oled_error_reset_continue)
ADD_NEW_OLED_MSG_ITEM(op_oled_over_voltage, handle_oled_over_voltage)
ADD_NEW_OLED_MSG_ITEM(op_oled_shutdown_ui, handle_oled_shutdown_ui)
ADD_NEW_OLED_MSG_ITEM(op_oled_remove_mode_ui, handle_oled_remove_mode_ui)
ADD_NEW_OLED_MSG_ITEM(op_oled_remove_stick_ui, handle_oled_remove_stick_ui)
ADD_NEW_OLED_MSG_ITEM(op_oled_wrong_charger, handle_oled_wrong_charger)
ADD_NEW_OLED_MSG_ITEM(op_oled_flash_version, handle_oled_get_flash_version)
ADD_NEW_OLED_MSG_ITEM(op_oled_dis_color, handle_oled_dis_color)
ADD_NEW_OLED_MSG_ITEM(op_oled_stick_sensor, handle_oled_stick_sensor)
END_OLED_MESSAGE_MAP
/*************************************************************************************************
  * @brief    : create a message queue for display
  * @param1   : msg id
  * @return   : none
*************************************************************************************************/
uint8_t create_oled_msg_queue(void)
{
   oledQueueHandle = xQueueCreate(Max_QueueMsg_Size, sizeof(msg_st));
   if(oledQueueHandle){
      return 1;
   }else{
      return 0;
   }
}
/*************************************************************************************************
  * @brief    : post a message to oled msg queue
  * @param1   : msg id
  * @return   : none
*************************************************************************************************/
void post_msg_to_oled(opcode_e opcode)
{
    msg_st msg;
   
    msg.opcode = opcode;
    if(oledQueueHandle == 0)
    {
        //LOGD("No oledQueueHandle exist.\r\n");
        return;
    }
    xQueueSend(oledQueueHandle, (void *)&msg, portMAX_DELAY);
    //LOGD("post %s msg\r\n", get_opcode_name(opcode));

}

/*************************************************************************************************
  * @brief    : post a message to oled msg queue
  * @param1   : msg id
  * @param1   : argument
  * @return   : none
*************************************************************************************************/
void post_msg_to_oled_with_arg(opcode_e opcode, uint32_t arg)
{
    msg_st msg;
    msg.opcode = opcode;
    msg.value = arg;
    if(oledQueueHandle == 0)
    {
        //LOGD("No oledQueueHandle exist.\r\n");
        return;
    }
    xQueueSend(oledQueueHandle, (void *)&msg, portMAX_DELAY);
//    LOGD("post %s msg,arg=%d\r\n", get_opcode_name(opcode), msg.value);
}

/*************************************************************************************************
 * @brief   :init queue for the oled display task 
 * @param   :none
 * @return  :none
*************************************************************************************************/
void oled_display_init(void)
{
    //init msg queue for oled_display
    uint8_t ret;
    ret = create_oled_msg_queue();
    if(!ret){
        LOGE("create oled msgQueue failed ...\r\n");
    }
    else{
        LOGD("create oled msgQueue OK!\r\n");
    }
    oled_ui_parm_init();
}

/*************************************************************************************************
 * @brief   :dispatch_oled_message
 * @param   :the message pointer
 * @return  :none
*************************************************************************************************/
void dispatch_oled_message(msg_st* pmsg)
{
  //todo: may be hook to do something else
    uint16_t i;
  /*look up the message handler*/
  for(i = 0; i < ARRAY_SIZE_OF(oled_msg_node_array); i++){
    if(oled_msg_node_array[i].msg_opcode == pmsg->opcode && oled_msg_node_array[i].handler != NULL){
        oled_msg_node_array[i].handler(pmsg);
        return;
    }
  }
}


/*************************************************************************************************
  * @brief    : oled dislapy task
  * @return   : None
*************************************************************************************************/
void oled_display_task(void)
{
    msg_st msg;
    /*get a message*/
    if(xQueueReceive(oledQueueHandle, (void *)&msg, portMAX_DELAY)){
        dispatch_oled_message(&msg);
    }
}
void oled_queue_reset(void){
    xQueueReset(oledQueueHandle);
}

/* Copyright ? 2010-2020 BYD Corporation. All rights reserved. * message.c
 *
 *  Created on: 2020å¹?æœ?6æ—?
 *      Author: BYD
 */

#include "message.h"
#include "string.h"
#include "log.h"

static char g_opcode_name[op_max][32]={
    {"op_btn_short_up"},
    {"op_btn_short_down"},
    {"op_btn_1s5_down"},
    {"op_btn_1s5_up"},
    {"op_btn_3s_down"},
    {"op_btn_5s_down"},
    {"op_btn_oneshort_onelong"},
    {"op_btn_3short"},
    {"op_heat_end_soon"},
    {"op_heat_session_extend"},
    {"op_heat_warmup_finish_nh"},
    {"op_heat_warmup_finish_bh"},
    {"op_heat_finish"},
    {"op_entry_cit"},
    {"op_exit_cit"},
    {"op_update_cit_timer"},
    {"op_charge"},
    {"op_charge_timeout"},
    //{"op_bat_left"},
    {"op_normal_heat"},
    {"op_boost_heat"},
    {"op_stop_heat"},
    {"op_error_occur"},
    {"op_error_clear"},
    {"op_existing_error"},
    {"op_error_led_output_finish"},
    {"opt_cmd_disable_charge"},
    {"opt_cmd_enable_charge"},
    {"opt_boot_complete"},
    {"op_oled_brand"},
    {"op_oled_reboot"},
//    {"op_hall_door_mode_start_timer"},
    {"op_hall_door_mode"},
    {"op_UI_updating"},
//    {"op_oled_heat"},
    {"op_oled_mode_to_heatup"},
    {"op_oled_warm_up"},
    {"op_oled_cancel_heatup"},
    {"op_oled_heatup_remove"},
    {"op_oled_heatup_session"},
    {"op_oled_session"},
    {"op_oled_cancel_session"},
    {"op_oled_dis_extend_ui"},
    {"op_oled_enter_extended"},
    {"op_oled_warm_up_count_down"},
    {"op_oled_session_count_down"},
//    {"op_oled_battery_check_timer"},
    {"op_oled_battery_check"},
//    {"op_oled_charging_check_timer"},
    {"op_oled_charging_check"},
    {"op_oled_cleaning_prompt_timer"},
    {"op_oled_clear_black"},
    {"op_oled_stick_sensor"},
    {"op_oled_certfct_graph"},
    {"op_oled_error_reset_oneshot"},
    {"op_oled_error_reset_continue"},
    {"op_oled_over_time"},
    {"op_oled_over_voltage"},
    {"op_oled_shutdown_ui"},
    {"op_oled_remove_mode_ui"},
    {"op_oled_remove_stick_ui"},
    {"op_oled_session_to_remove"},
    {"op_oled_wrong_charger"},
    {"op_oled_flash_version"},
    {"op_oled_dis_color"},
    {"op_invalid"},
};

/*************************************************************************************************
 * @brief   :get opcode(message) name
 * @param   :message id
 * @return  :message name
*************************************************************************************************/
char* get_opcode_name(opcode_e i)
{
	if(i < op_max){
		return g_opcode_name[i];
	}else{
		return "op_max";
	}
}


/*************************************************************************************************
 * @brief   :dispatch_message
 * @param   :machine state arrange pointer
 * @param   :the current machine state index pointer
 * @param   :the message pointer
 * @return  :none
*************************************************************************************************/
static void default_dispose_message(state_msg_map_st* psmm, uint16_t* pstate, msg_st* pmsg)
{
  uint16_t i;
  uint16_t state = *pstate;
  uint16_t count = psmm[state].count;

  /*look up the message handler*/
  for(i = 0; i < count; i++){
    if(psmm[state].p_msgnode_array[i].msg_opcode == pmsg->opcode && psmm[state].p_msgnode_array[i].handler != NULL){
        psmm[state].p_msgnode_array[i].handler(pmsg);
        return;
    }
  }
}

/*************************************************************************************************
 * @brief   :dispatch_message
 * @param   :machine state arrange pointer
 * @param   :the current machine state index pointer
 * @param   :the message pointer
 * @return  :none
*************************************************************************************************/
void dispatch_message(state_msg_map_st* psmm, uint16_t* pstate, msg_st* pmsg)
{
  //todo: may be hook to do something else
  default_dispose_message(psmm, pstate, pmsg);
}





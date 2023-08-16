#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "stddef.h"
#include "stdint.h"

#define Max_QueueMsg_Size 64

typedef enum{
    op_btn_short_up= 0,
    op_btn_short_down,
    op_btn_1s5_down,
    op_btn_1s5_up,
    op_btn_3s_down,
    op_btn_5s_down,
    op_btn_oneshort_onelong,
    op_btn_3short,
    op_heat_end_soon,
    op_heat_session_extend,
    op_heat_warmup_finish_nh,
    op_heat_warmup_finish_bh,
    op_heat_finish,
    op_entry_cit,
    op_exit_cit,
    op_update_cit_timer,
    op_charge,
    op_charge_timeout,
   //op_bat_left,
    op_normal_heat,
    op_boost_heat,
    op_stop_heat,
    op_error_occur,
    op_error_clear,
    op_existing_error,
    op_error_led_output_finish,
    opt_cmd_disable_charge,
    opt_cmd_enable_charge,
    opt_boot_complete,
    op_oled_brand,
    op_oled_reboot,
//    op_hall_door_mode_start_timer,
    op_hall_door_mode,
    op_UI_updating,
//    op_oled_heat,
    op_oled_mode_to_heatup,
    op_oled_warm_up,
    op_oled_cancel_heatup,
    op_oled_heatup_remove,
    op_oled_heatup_session,
    op_oled_session,
    op_oled_cancel_session,
    op_oled_dis_extend_ui,
    op_oled_enter_extended,
    op_oled_warm_up_count_down,
    op_oled_session_count_down,
//    op_oled_battery_check_timer,
    op_oled_battery_check,
//    op_oled_charging_check_timer,
    op_oled_charging_check,
    op_oled_cleaning_prompt_timer,
    op_oled_clear_black,
    op_oled_stick_sensor,
    op_oled_certfct_graph,
    op_oled_error_reset_oneshot,
    op_oled_error_reset_continue,
    op_oled_over_time,
    op_oled_over_voltage,
    op_oled_shutdown_ui,
    op_oled_remove_mode_ui,
    op_oled_remove_stick_ui,
    op_oled_session_to_remove,
    op_oled_wrong_charger,
    op_oled_flash_version,
    op_oled_dis_color,
    op_invalid,
    op_max
}opcode_e;

typedef struct
{
  opcode_e opcode;
  uint32_t value;
}msg_st;


typedef void (*on_msg_handler)(msg_st* msg);

typedef struct
{
  opcode_e msg_opcode;
  on_msg_handler handler;
}msgnode_st;

typedef enum{
  close2base = 0,
  close2boost,
  base2boost,
  boost2base,
  base2close,
  boost2close,
  base_open,
  boost_open,
  hall_error,
}op_hall_door_e;


typedef struct
{
  msgnode_st* p_msgnode_array;
  uint16_t count;
}state_msg_map_st;

#define ARRAY_SIZE_OF(msg_array) (sizeof(msg_array)/sizeof(msg_array[0]))

#define BEGIN_MESSAGE_MAP(StateName) static msgnode_st msg_node_array_##StateName[]={
#define ADD_NEW_MSG_ITEM(MsgOpcode,OnMsg)                                         {MsgOpcode,OnMsg},
#define END_MESSAGE_MAP                                                      };

#define BEGIN_Register_State(TaskName) static state_msg_map_st StateMsgMap_##TaskName[]={
#define ADD_Register_State(StateName)                                               {(msgnode_st*)msg_node_array_##StateName, ARRAY_SIZE_OF(msg_node_array_##StateName)},
#define END_Register_State                                                       };

#define GET_SMM_NAME(NAME)    (StateMsgMap_##NAME)

char* get_opcode_name(opcode_e i);
void dispatch_message(state_msg_map_st* psmm, uint16_t* pstate, msg_st* pmsg);

#endif /* MESSAGE_H_ */

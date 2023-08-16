#ifndef __APP_OLED_UI_H
#define __APP_OLED_UI_H

#include "stdint.h"
#include "message.h"

#define VERSION_IMAGE_SIZE 70*200*3

#define VERSION_IMAGE_START_ADDR                                   0x0
#define BRAND_START_ADDR                                           0x0000A450
#define BRAND_TO_BASE_START_ADDR                                   0x00064E10
#define BRAND_TO_BOOST_START_ADDR                                  0x0007C090
//#define OFF_TO_STANDARD_MODE_START_ADDR                          0x00111D18
#define STANDARD_TO_BOOST_MODE_START_ADDR                          0x00092764
#define BOOST_TO_STANDARD_MODE_START_ADDR                          0x00092764
#define STANDARD_MODE_START_ADDR                                   0x000A819C
#define BOOST_MODE_START_ADDR                                      0x000ACF9C
//#define STANDARD_TO_OFF_START_ADDR                               0x001ADF70
//#define BOOST_TO_OFF_START_ADDR                                  0x00841B2E
#define STANDARD_TO_HEAT_UP_START_ADDR                             0x000B224C
#define BOOST_TO_HEAT_UP_START_ADDR                                0x000C84AC
#define MODE_WARM_UP_START_ADDR                                    0x000E4AE4
#define WARM_UP_START_ADDR                                         0x001014A0
#define TRANS_HEAT_UP_TO_SESSION_START_ADDR                        0x001057B4
#define SESSION_START_ADDR                                         0x00150F64
#define SESSION_EXTENDED_START_ADDR                                0x0019F912
#define SESSION_TRANSITION_REMOVE_STICK_OUT_START_ADDR             0x001B3F52
#define REMOVE_STICK_OUT_START_ADDR                                0x0020942E
#define TRANS_REMOVE_TO_MODE_START_ADDR                            0x002A5302
#define NUM_START_ADDR                                             0x002CD30A
#define FAST_CHARGE_START_ADDR                                     0x002E5652
#define NORMAL_SLOW_CHARGE_START_ADDR                              0x00346F8A
#define BATTERY_ICON_START_ADDR                                    0x003CB97A
#define CLOSING_BYE_START_ADDR                                     0x003DE6FA
#define CLEANING_START_ADDR                                        0x0043116A
#define ERROR_START_ADDR                                           0x004BB0CE
#define STICK_SENSOR_START_ADDR                                    0x005A2362
#define CERTIFICATION_GRAPHICS_START_ADDR                          0x0063AE9A
#define HEAT_REMOVE_START_ADDR                                     0x0065EEEE
#define REBOOT_START_ADDR                                          0x0067FB52




#define FPS_15 66  //  1000s/15
#define FPS_25 40  //  1000s/25

#pragma pack(1)
typedef struct
{
    uint8_t ui_version[32];
}ext_flash_record_t;
#pragma pack()

typedef enum
{
    OLED_WHITE = 0,
    OLED_BLACK,
    OLED_RED,
    OLED_GREEN,
    OLED_BLUE,
    OLED_ALL = 0xFF
}color_value_e;
typedef enum
{
    NO_UI = 0,
    OPEN_UI,
    REBOOT_UI,
    MODE_SELECT_UI,
    MODE_HEAT_UI,
    HEAT_UP_UI,
    HEAT_UP_CANCEL_UI,
    HEAT_REMOVE_UI,
    HEAT_SESSION_UI,
    SESSION_CANCEL_UI,
    SESSION_UI,
    SESSION_REMOVE_UI,
    REMOVE_STICK_UI,
    REMOVE_MODE_UI,
    BATTERY_CHECK_UI,
    CHARGE_UI,
    CHARGE_TIME_OUT_UI,
    NO_MODE_CLOSE_UI,
    RESET_ERROR_UI,
    WAIT_ERROR_UI,
    OVER_VOLTAGE_UI,
    CLEANING_UI,
    STICK_SENSOR_UI,
    CERTFCT_GRAPH_UI,
    UI_ALL = 0xFF
}ui_value_e;

ext_flash_record_t * get_version_record_from_ext_flash(void);
ext_flash_record_t * get_ext_flash_record_from_ram(void);
void handle_oled_reboot(msg_st* pmsg);
void start_oled_reboot_ui(void);
void start_oled_warm_up_ui(void);
void start_oled_session_ui(void);
void handle_oled_normal_heat(msg_st* pmsg);
void handle_oled_brand(msg_st* pmsg);
void start_oled_welcome_brand(void);
//void handle_oled_start_mode_timer(msg_st* pmsg);
void start_oled_display_mode_select(uint8_t mode_select);
void handle_oled_door_mode(msg_st* pmsg);
void handle_oled_off_normal(msg_st* pmsg);
void handle_oled_normal_to_boost(msg_st* pmsg);
//void handle_oled_heat(msg_st* pmsg);
void start_oled_heat(uint8_t heat_mode_state);
void handel_transition_mode_to_heatup(msg_st* pmsg);
void handle_oled_heat_up(msg_st* pmsg);
void handle_transition_heatup_to_session(msg_st* pmsg);
void handle_oled_session(msg_st* pmsg);
void start_enter_session_extended_ui(void);
void handle_enter_session_extended_ui(msg_st* pmsg);
void start_dis_session_extended_ui(uint8_t dis_flag);
void handle_display_session_extended_ui(msg_st* pmsg);
void handle_oled_warm_up_count_down(msg_st* pmsg);
void handle_oled_session_count_down(msg_st* pmsg);
void start_oled_session_count_down(void);
//void handle_oled_start_battery_check_timer(msg_st* pmsg);
void start_oled_display_battery_check(uint8_t battery_soc);
void start_oled_batterycheck_and_cleaning(uint8_t battery_soc);
void handle_oled_battery_check(msg_st* pmsg);
//void handle_oled_start_charging_check_timer(msg_st* pmsg);
void start_oled_display_charging_check(uint8_t battery_soc);
void handle_oled_charging_check(msg_st* pmsg);
void start_oled_heat_cancel(void);
void handle_oled_heat_up_cancel_ui(msg_st* pmsg);
void handle_cancel_session_ui(msg_st* pmsg);
void handel_transition_heatup_to_remove(msg_st* pmsg);
void start_oled_transition_heatup_to_remove_stickout(void);
void handle_oled_transition_session_to_remove_stickout(msg_st* pmsg);
void start_oled_transition_session_to_remove_stickout(void);
void handle_oled_error_code(msg_st* pmsg);
void start_error_ui(uint8_t error_type);
void dis_color(uint16_t rvalues,uint16_t gvalues,uint16_t bvalues);
void start_oled_cleaning_prompt(void);
void handle_oled_cleaning_prompt_timer(msg_st* pmsg);
void handle_oled_clear_black(msg_st* pmsg);
void start_oled_clear_black(void);
void app_show_oled_color(uint8_t oled_color);
void handle_oled_stick_sensor(msg_st* pmsg);
void start_stick_sensor_ui(uint16_t stick_sensor_status);
void change_stick_sensor_ui(uint16_t stick_sensor_status);
void handle_oled_certfct_graph(msg_st* pmsg);
void start_oled_certfct_graph(void);
void handle_oled_error_reset_oneshot(msg_st* pmsg);
void start_oled_error_reset_oneshot(void);
void handle_oled_error_reset_continue(msg_st* pmsg);
void start_oled_error_reset_continue(void);
void start_no_mode_closing_animation(void);
void handle_oled_shutdown_ui(msg_st* pmsg);
void handle_oled_remove_mode_ui(msg_st* pmsg);
void start_op_oled_remove_mode_ui(void);
void handle_oled_remove_stick_ui(msg_st* pmsg);
void start_op_oled_remove_stick_ui(void);
void handle_oled_get_flash_version(msg_st* pmsg);
void start_oled_wrong_charger(void);
void handle_oled_wrong_charger(msg_st* pmsg);
void get_flash_version(void);
void handle_oled_dis_color(msg_st* pmsg);
void dis_color_msg(uint8_t oled_color);
void handle_oled_over_voltage(msg_st* pmsg);
void start_oled_over_voltage_ui(void);
void handle_oled_over_time_ui(msg_st* pmsg);
void start_oled_charge_over_time_ui(uint8_t battery_soc);
void oled_ui_parm_init(void);
#endif

#ifndef __MANAGER_H
#define __MANAGER_H

#include "message.h"

typedef enum {
    STATE_BOOTING = 0,
    STATE_IDLE,
    STATE_NORMAL_HEAT,
    STATE_BOOST_HEAT,
    STATE_CHARGE,
    STATE_CIT,
    STATE_ERROR,
    STATE_MAX
}system_state_e;

enum
{
    LOG_POA1 = 0x24,
    HEAT_LOG = 0x27,
    CHARGE_LOG,
    IDLE_LOG,
    CHARGE_REG_LOG,
    CIGAR_DECT_LOG = 0x31,
    ERROR_LOG = 0x32,
};
enum
{
    charge_full = 0,
    charge_begin,
    cable_disconnected,
    cable_connect,
    wrong_charge,
};
enum
{
    chg_timeout_occur = 0,
    chg_timeout_clear = 1,
    chg_timeout_show,
    chg_timeout_noshow,
};

enum
{
    OTHER2CHARGE = 0x0,
    CHARGE2CHARGE,
    CHARGE2OTHER,
};
enum
{
    ERROR_NO_CHARGE = 0x0,
    ERROR_TOO_COLD,
    ERROR_TOO_HOT,
    ERROR_RETURN,
    ERROR_EOL,
    ERROR_WAIT_GENERAL,
};
enum
{

    HEAT_OVER = 0x0,
    BASE_WARM_UP,
    BASE_SESSION,
    BOOST_WARM_UP,
    BOOST_SESSION,
};
enum
{
    HEAT_FINISH_PROFILE_ERROR = 0x0,
    HEAT_FINISH_COMPLETELY,
};

enum
{   
    NONE_SESSION_EXTEND = 0x0,
    HAVE_SESSION_EXTEND = 0x1,
    SESSION_EXTEND_TIME_START ,
    SESSION_EXTEND_TIME_END,
    SESSION_EXTEND_FUCTION_ON,
};

enum{
  OLED_BTN_LOCK = 0x01,
  OLED_HALL_LOCK = 0x02,
  OLED_CHG_LOCK = 0x04,
  OLED_STICK_SENSOR_LOCK = 0x08,
  OLED_LONG_BTN_LOCK = 0x10,
};

enum
{
    NO_USB_PLUG = 0,
    WELL_USB_PLUG = 1,
    WRONG_USB_PLUG = 2,
};

void manager_init(void);
void manager_task(void);
void post_msg_to_manager(opcode_e opcode);
void post_msg_to_manager_with_arg(opcode_e opcode, uint32_t arg);
void set_auto_shut_down_ms(uint32_t ms);
uint64_t get_error_code(void);
void clear_return_err_flag(void);
void test_idle_logs(void);
uint16_t app_get_state(void);
void change_state(system_state_e newState);
void start_auto_shutdown_timer(void);
void stop_auto_shutdown_timer(void);
void set_auto_test_flag(uint8_t flag);
void clear_all_oled_lock(void);
void show_wrong_charger(void);
void stop_comm_lock_timer(void);
#endif



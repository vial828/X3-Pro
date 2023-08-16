#ifndef CHARGE_H
#define CHARGE_H

#include "stdint.h"
#include "self_flash.h"

#define CHG_SAFETY    1
#define CHG_SETUP     2
#define CHG_CHARGE    3
#define CHG_TEMP      4
#define CHG_FULL      5
#define CHG_DELAY     6
#define CHG_FAULT     7
#define CHG_LOCKOUT   8
#define CHG_USBHOT    9
#define CHG_INVALID    0xFF

#define CHG_PARTAB_SUSPEND 0
#define CHG_PARTAB_SLOW_CHG_BAT_COLD 1
#define CHG_PARTAB_SLOW_CHG_41 2
#define CHG_PARTAB_NORMAL 3
//#define CHG_PARTAB_SUSPEND_BAT_HOT 4
#define CHG_PARTAB_INVALID    0xFF


#define CHG_TIME_OUT 5*60*60

#define FAST_CHARGE_CURR_LIMIT_0A 0
#define FAST_CHARGE_CURR_LIMIT_02C 576
#define FAST_CHARGE_CURR_LIMIT_033C 1088
#define FAST_CHARGE_CURR_LIMIT_2A 1984


#define MIN_VOLT    (PRE_HEAT_CUTOFF_VOLT/1000.0f)

#define TEMP_15         15.0f

enum{
    temp_Section_normal=1,
    temp_Section_protect,
    temp_Section_abnormal
};

enum
{
    FAST_CHARGE_MODE = 0,
    NORMAL_CHARGE_MODE,
    SLOW_CHARGE_MODE
};

typedef enum{
  left10 = 0,
  left25,
  left50,
  left75,
  left95,
  left100,
  max_left
}bat_energy_left_e;

enum{
  CHG_CIT_SUSPEND = 0x01,
  CHG_ERROR_SUSPEND = 0x02,
  CHG_CLI_SUSPEND = 0x04,
  CHG_HEAT_SUSPEND = 0x08,
  CHG_HW_RESET_SUSPEND = 0x10,
  CHG_UPDATE_IMAGE_SUSPEND = 0x20,
  CHG_DISPLAY_SUSPEND = 0x40,
  CHG_TIMEOUT_SUSPEND = 0x80,
};

void app_charge_enable(uint8_t en);
void app_charge_init(void);
void app_charge_task(void);
uint8_t app_get_bat_left(void);

typedef enum
{
    no_DRIVER = 0,
    BQ25898E,
    MP2731
}chrg_driver_e;


typedef enum{
    BATTERY_GP = 0,
    BATTERY_BYD,
}BATTERY_TYPE_E;
//void check_charge_type(void);

extern chrg_driver_e current_chrg_drv;

chrg_driver_e app_charge_read_driver_id(void);
void app_SendChargeICParameter(void);
uint16_t app_GetChargeICParameter(void);
void app_charge_ic_task(void);
uint16_t app_GetVbusVolt(void);
void app_setChargerState(uint8_t state);
void setChargerPartab(uint8_t value);
uint8_t app_updateFue(void);
void app_RecalculateBat(void);
void app_send_charge_IC_reg(void);
uint8_t app_GetChargerState(void);
uint8_t app_GetChargerPartab(void);
float app_GetBatJoules(void);
void app_CheckChrgFault(uint8_t *pInputVolFault, uint8_t *pTempOutRangeFault, uint8_t *pChargTimeOutFault);
uint8_t app_CheckWatchdogFault(void);
uint8_t app_CheckOutputVoltage(void);
uint8_t app_CheckNTCFault(void);
void app_CheckVoltageSelfCheck(uint8_t *pVoltageSelfCheckFault, uint8_t *pCurrentSelfCheckFault);
void app_ConfigChargIC(void);

void app_RstChargeICWdgTimer(void);
void app_CheckChgICRegData(void);
void app_GetTSPCT(uint8_t *pVal);
float app_GetNTCTemp(void);
uint8_t app_get_bat_left_percentage(void);
void app_chg_entry_cit(void);
void app_chg_exit_cit(void);
void app_chg_entry_suspend(uint8_t reason);
uint8_t app_get_suspend_state(uint8_t reason);
void app_chg_try_to_exit_suspend(uint8_t reason);
void app_chg_entry_error(void);
void app_chg_exit_error(void);
chrg_driver_e app_get_current_chrg_drv(void);
uint8_t app_check_usb_plug_status(void);
uint8_t app_get_battery_temp_section(float temp);
void  app_set_charge_profile(uint8_t arg);
void  app_set_charge_part_ab(uint8_t arg);
void app_SetNormalChargeCurrent(void);
void app_SetMinChargeCurrent(void);
void app_SetLowPowerChargerCurrent(void);
void read_soc_from_gauge(void);
/*************************************************************************************************
  * @brief    : Read battery type function
  * @param1   : void
  * @return   : 0-BAK; 1-Samsung
*************************************************************************************************/
BATTERY_TYPE_E app_read_bat_id(void);

/*************************************************************************************************
  * @brief    : init GPIO PB11 port
  * @param1   : void
  * @return   : void
*************************************************************************************************/
void app_bat_id_init(void);
void app_set_usb_status(uint8_t status);
void app_check_usb_Volt(uint16_t vbusVolt);
void dev_first_get_soc(void);
uint8_t app_get_charging_mode(void);
void app_CheckGaugeData(void);
#endif


#ifndef  _DEV_BQ25898X_H
#define  _DEV_BQ25898X_H

#include "bq25898x_reg.h"
#include "stdint.h"
//#include "stm32g0xx_hal_gpio.h"

#define BAT_ID_PORT     GPIOB
#define BAT_ID_PIN      GPIO_PIN_11

#define BQ25898X_REG_ID      (0x00)
//! \brief Defines a macro to read a block of registers.
//!
#define BQ25898x_READBLOCK(reg, buff, len) \
  hwi_I2C_Mem_Read(GPIO_CHANNEL , BQ25898X_ADDR , reg , I2C_MEMADD_SIZE_8BIT , buff , len , 0xfff)
//error num
#define  I2C_READ_ERROR             0x11
#define  I2C_WRITE_ERROR            0x12
#define  ADC_NOT_ENABLED            0x13

//charge ic config value
#define PRECHARGE_CURR_LIMIT  192
#define ADJ_PRECHARGE_CURR_LIMIT  128
#define FAST_CHARGE_CURR_LIMIT 3008
#define FAST_CHARGE_CURR_STEP1_LIMIT 2688
#define FAST_CHARGE_CURR_STEP2_LIMIT 2688
#define FAST_CHARGE_CURR_STEP3_LIMIT 2688
#define FAST_CHARGE_CURR_STEP4_LIMIT 2432
#define MIN_FAST_CHARGE_CURR_LIMIT 896
#define ADJ_FAST_CHARGE_CURR_LIMIT 448
#define TEMP_FAST_CHARGE_CURR_LIMIT 1024//temporary current for 38-41
#define TERM_CURR_LIMIT 192
#define CHARGE_VOLT_LIMIT 4400 //mV
#define CHARGE_VOLT_LIMIT_STEP1 4352 //mV
#define CHARGE_VOLT_LIMIT_STEP2 4304 //mV
#define CHARGE_VOLT_LIMIT_STEP3 4256 //mV
#define CHARGE_VOLT_LIMIT_STEP4 4208 //mV

#define RECHARGE_THRESHOLD_OFFSET   BQ25898X_VRECHG_100MV
#define PRECHARGE_TO_FAST_CHARGE_THRESHOLD BQ25898X_BATLOWV_3000MV
#define FAST_CHARGE_TIMER BQ25898X_CHG_TIMER_8HOURS //8 hours
#define LOW_TEMP_CURR_LIMIT BQ25898X_JEITA_ISET_50PCT
#define THERMAL_REGULATION_THRESHOLD BQ25898X_TREG_120C
#define BOOST_MODE_CURR_LIMIT BQ25898X_BOOST_LIM_1500MA
#define INPUT_CURR_LIMIT  2500 //mA
#define INPUT_VOLT_LIMIT 5000 //mV
#define HIGH_TEMP_VOLT_LIMIT BQ25898X_JEITA_VSET_N200MV
#define SYS_MIN_VOLT_LIMIT 3000
#define BAT_COMP_LIMIT 140
#define VCLAMP_LIMIT 32
#define VDPM_OS BQ25898X_VDPM_OS_0P4A

typedef struct
{
    uint8_t reg;
    uint8_t mask;
    uint8_t shift;
    uint8_t value;
}REG_CONFIG_INFO_T;

enum awinic_chip_id {

    BQ25898E_CHIP_ID = 0x02,
};

typedef enum
{
    BQ25898_WTDG_FAULT,
    BQ25898_BOOST_FAULT,
    BQ25898_FAULTCHRG_FAULT,
    BQ25898_FAULTBAT_FAULT,
    BQ25898_FAULTNTC_FAULT,
    BQ25898_FAULT_FLAG_NUM
}BQ25898_FAULT_FLAG_E;

typedef enum{
    CHRG_FLT_NORMAL = 0,
    CHRG_FLT_INPUT_FLT,
    CHRG_FLT_THERM_OFF,
    CHRG_FLT_TIMER_EXPIR,
}CHRG_FALUT_E;

typedef enum{
    NTC_FLT_NORMAL = 0,
    NTC_FLT_WARM = 2,
    NTC_FLT_COOL = 3,
    NTC_FLT_COLD = 5,
    NTC_FLT_HOT = 6,
}NTC_FAULT_E;


extern uint8_t g_adcEnabled;
void session_count_check(uint8_t temp_status);
uint8_t Dev_BQ25898X_Init(void);

uint8_t Dev_BQ25898X_EnableCharge(uint8_t enable);
uint8_t Dev_BQ25898X_EnableADC(uint8_t enable);
uint8_t Dev_BQ25898X_EnableOtg(uint8_t enable);
uint8_t Dev_BQ25898X_GetStatus(uint8_t *vbusStatus, uint8_t *chgStatus, uint8_t *pgStatus, uint8_t *vsysStatus);
uint8_t Dev_BQ25898X_GetAdc(uint16_t *curr, uint16_t *vBusVolt, uint16_t *selfBatVolt, uint16_t  *vsysVolt, uint16_t *tsValue);
uint8_t Dev_BQ25898X_GetVindpm(uint8_t *forceVIndpm, uint16_t *vinDpm);
uint8_t Dev_BQ25898X_ChrgCurrent(uint16_t u16Current);
uint8_t Dev_BQ25898X_GetWatchdogFault(void);
CHRG_FALUT_E Dev_BQ25898X_GetChrgFault(void);
uint8_t Dev_BQ25898X_GetOutputVoltageFault(void);
NTC_FAULT_E Dev_BQ25898X_GetNtcFault(void);
uint8_t Dev_BQ25898X_ReadRegBits(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t* data);
uint8_t Dev_BQ25898X_OtgCurrent(uint8_t u8Current);
uint8_t Dev_BQ25898X_ChrgVoltLimit(uint16_t u16Volt);
uint8_t Dev_BQ25898X_Set_SYS_MIN(uint32_t vol);
uint8_t Dev_BQ25898x_getDeviceID(uint8_t *bq25898x_id);

uint8_t Dev_BQ25898X_GetNtcTemp(uint8_t *temp);

uint8_t Check_BQ25898X_RegBits (uint8_t reg, uint8_t mask, uint8_t shift, uint8_t data);

uint8_t Dev_BQ25898X_RstWdgTimer(void);

void Dev_BQ25898x_UpdateCheckinfo_ICHG(uint16_t u16Current);
void Dev_BQ25898x_UpdateCheckinfo_VREG(uint16_t u16Volt);
void Dev_BQ25898X_CheckRegData(void);
uint8_t Dev_BQ25898X_Set_LowCurrent_Charge(void);
uint8_t Dev_BQ25898X_Set_DefaultCurrent_Charge(void);
void Dev_BQ25898X_SetNormalChargeCurrent(void);
void Dev_BQ25898X_SetABNormalChargeCurrent(void);
void Dev_BQ25898X_SetMinChargeCurrent(void);
void Dev_BQ25898X_SetTemporaryChargeCurrent(void);
void Dev_BQ25898X_BATFET_DIS(void);
void Dev_BQ25898X_UpdateVDPM_OS(void);
uint8_t get_eol_step(void);
#endif

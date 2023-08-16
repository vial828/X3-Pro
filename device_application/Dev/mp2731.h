#include "stdint.h"
#include "HWI_Hal.h"
#include "i2c.h"

#define MP2731_ADDR    (0x4B )
#define DEBUG_KEY

#define BAT_ID_PORT     GPIOB
#define BAT_ID_PIN      GPIO_PIN_11
//error num
#define  I2C_READ_ERROR             0x11
#define  I2C_WRITE_ERROR            0x12
#define  ADC_NOT_ENABLED            0x13
#define  MP2731_REG_NUMS  24

 /*reg defines*/
#define MP2731_REG_00  0x00U
#define MP2731_REG_01  0x01U
#define MP2731_REG_02  0x02U
#define MP2731_REG_03  0x03U
#define MP2731_REG_04  0x04U
#define MP2731_REG_05  0x05U
#define MP2731_REG_06  0x06U
#define MP2731_REG_07  0x07U
#define MP2731_REG_08  0x08U
#define MP2731_REG_09  0x09U
#define MP2731_REG_0A  0x0AU
#define MP2731_REG_0B  0x0BU
#define MP2731_REG_0C  0x0CU
#define MP2731_REG_0D  0x0DU
#define MP2731_REG_0E  0x0EU
#define MP2731_REG_0F  0x0FU
#define MP2731_REG_10  0x10U
#define MP2731_REG_11  0x11U
#define MP2731_REG_12  0x12U
#define MP2731_REG_13  0x13U
#define MP2731_REG_14  0x14U
#define MP2731_REG_15  0x15U
#define MP2731_REG_16  0x16U
#define MP2731_REG_17  0x17U
#define MP2731_REG_4B  0x4BU

#define IINLIM_REG          MP2731_REG_00   //input current limit
#define IINLIM_MASK         0x3FU
#define IINLIM_SHIFT        0x00U
#define IINLIM_SU           50      //mA smallest uint
#define IINLIM_OFFSET       100

#define EN_HIZ_REG      MP2731_REG_00
#define EN_HIZ_MASK     0x80U
#define EN_HIZ_SHIFT    0x07U
#define EN_HIZ_EN       1
#define EN_HIZ_DIS      0   //DEFAULT

#define VINDPM_REG          MP2731_REG_01
#define VINDPM_MASK         0x7FU
#define VINDPM_SHIFT        0x00U
#define VINDPM_SU           100      //cnt of (1/20000) of regn    smallest uint
#define VINDPM_OFFSET       3700

#define TREG_REG            MP2731_REG_02  //Thermal Regulation Threshold
#define TREG_MASK           0x0CU
#define TREG_SHIFT          0x02U
#define TREG_60             0
#define TREG_80             1
#define TREG_100            2
#define TREG_120            3

#define NTC_TYPE_REG        MP2731_REG_02  //Thermal Regulation Threshold
#define NTC_TYPE_MASK       0x40U
#define NTC_TYPE_SHIFT      0x06U

#define CONV_RATE_REG       MP2731_REG_03   //ad convert type
#define CONV_RATE_MASK      0x40U
#define CONV_RATE_SHIFT     0x06U
#define CONV_RATE_ONCE      0     //DEFAUT
#define CONV_RATE_CON       1

#define CONV_START_REG      MP2731_REG_03  //ad convert start
#define CONV_START_MASK     0x80U
#define CONV_START_SHIFT    0x07U
#define CONV_START_EN       1
#define CONV_START_DIS      0    //DEFAULT

#define SYS_MIN_REG        MP2731_REG_04
#define SYS_MIN_MASK       0x0EU
#define SYS_MIN_SHIFT      0x01U
#define SYS_MIN_SU         120
#define SYS_MIN_OFFSET     3000

#define CHG_CONFIG_REG      MP2731_REG_04   //charge enable
#define CHG_CONFIG_MASK     0x30U
#define CHG_CONFIG_SHIFT    0x04U
#define CHG_CONFIG_EN       1    //DEFAULT
#define CHG_CONFIG_DIS      0
#define CHG_CONFIG_RES      2
#define CHG_CONFIG_OTG      3

#define ICHG_REG            MP2731_REG_05   //fast charge current limit
#define ICHG_MASK           0x3FU
#define ICHG_SHIFT          0x00U
#define ICHG_SU             40      //mA smallest uint
#define ICHG_OFFSET         320

#define BATLOWV_MASK        0x80U
#define BATLOWV_SHIFT       0x07U
#define BATLOWV_2800MV      0
#define BATLOWV_3000MV      1

#define IPRECHG_REG         MP2731_REG_06   //precharge current limit
#define IPRECHG_MASK        0xF0U
#define IPRECHG_SHIFT       0x04U
#define IPRECHG_SU          40      //mA smallest uint
#define IPRECHG_OFFSET      150      //mA OFFSET

#define ITERM_REG           MP2731_REG_06   //Termination current limit
#define ITERM_MASK          0x0FU
#define ITERM_SHIFT         0x00U
#define ITERM_SU            40      //mA smallest uint
#define ITERM_OFFSET        120      //mA OFFSET

#define VREG_REG            MP2731_REG_07   //Charge Voltage Limit
#define VREG_MASK           0xFEU
#define VREG_SHIFT          0x01U
#define VREG_SU             10        //mv smallest uint
#define VREG_OFFSET         3400      //mv OFFSET

#define VRECHG_MASK         0x01
#define VRECHG_SHIFT        0
#define VRECHG_100MV        0
#define VRECHG_200MV        1

#define EN_TIMER_REG        MP2731_REG_08   //Charging Safety Timer Enable
#define EN_TIMER_MASK       0x01U
#define EN_TIMER_SHIFT      0x00U
#define EN_TIMER_EN         1    //DEFAULT
#define EN_TIMER_DIS        0
#define EN_TIMER_OFFSET     0

#define EN_TERM_REG         MP2731_REG_08
#define EN_TERM_MASK        0x80U
#define EN_TERM_SHIFT       0x07U
#define EN_TERM_EN          1
#define EN_TERM_DIS         0
#define EN_TERM_OFFSET      0


#define CHG_TIMER_REG       MP2731_REG_08  //Fast Charge Timer Setting
#define CHG_TIMER_MASK      0x06U
#define CHG_TIMER_SHIFT     0x01U
#define CHG_TIMER_5h        0
#define CHG_TIMER_8h        1
#define CHG_TIMER_12h       2
#define CHG_TIMER_20h       3

#define WATCHDOG_REG        MP2731_REG_08
#define WATCHDOG_MASK       0x30U
#define WATCHDOG_SHIFT      0x04U
#define WDT_RESET_MASK      0x08U
#define WDT_RESET_SHIFT     0x03U
#define WATCHDOG_DIS        0
#define WATCHDOG_40S        1   //DEFAULT
#define WATCHDOG_80S        2
#define WATCHDOG_160S       3
#define WDT_NORMAL          0
#define WDT_RESET           1

#define BATFET_DIS_REG      MP2731_REG_0A  //Force BATFET off to enable ship mode with tSM_DLY delay time
#define BATFET_DIS_MASK     0x20U
#define BATFET_DIS_SHIFT    0x05U
#define BATFET_DIS_EN       1
#define BATFET_DIS_DIS      0   //DEFAULT
#define SYSRST_SEL_MASK     0x10U
#define SYSRST_SEL_SHIFT    0x04U
#define SYSRST_SEL_HWRST    0
#define SYSRST_SEL_SWRST    1
#define SW_FREQ_MASK        0x80U
#define SW_FREQ_SHIFT       0x07U
#define SW_FREQ_0           0   //1.35MHz
#define SW_FREQ_1           1   //1MHz

#define USB_DET_EN_REG        MP2731_REG_0B
#define USB_DET_EN_MASK       0x20U
#define USB_DET_EN_SHIFT      0x05U

#define MP2731_STAT_REG     MP2731_REG_0C  //charge status
#define VIN_STAT_MASK       0xE0U
#define VIN_STAT_SHIFT      0x05U
#define CHRG_STAT_MASK      0x18U
#define CHRG_STAT_SHIFT     0x03U
#define NTC_FLOAT_STAT_MASK 0x04U
#define NTC_FLOAT_STAT_SHIFT 0x02U
#define THERM_STAT_MASK     0x02U
#define THERM_STAT_SHIFT    0x01U
#define VSYS_STAT_MASK      0x01U
#define VSYS_STAT_SHIFT     0x00U
//STATUS TYPE
#define NO_INPUT_VIN        0x00U
#define NSTD_ADAPTER_VIN    0x01U
#define SDP_VIN             0x02U
#define CDP_VIN             0x03U
#define DCP_VIN             0x04U
#define FCHG_ADAPTER_VIN    0x05U
#define OTG_VIN             0x07U
#define NOT_CHARGING        0x00U
#define PRE_CHARGE          0x01U
#define FAST_CHARGE         0x02U
#define CHARGE_TERM_DONE    0x03U
#define NO_NTC_FLOAT        0
#define EN_NTC_FLOAT        1
#define NORMAL_THERM        0x00U
#define THERMAL_THERM       0x01U
#define IN_VSYS_REG         0X00U
#define NOT_IN_VSYS_REG     0X01U


#define FAULT_REG             MP2731_REG_0D  //fault reg must be read one time and twice
#define WATCHDOG_FAULT_MASK   0x80U
#define WATCHDOG_FAULT_SHIFT  0x07U
#define BOOST_FAULT_MASK      0x40U
#define BOOST_FAULT_SHIFT     0x06U
#define CHRG_INPUT_MASK       0x20U
#define CHRG_INPUT_SHIFT      0x05U
#define CHRG_THERM_MASK       0x10U
#define CHRG_THERM_SHIFT      0x04U
#define BAT_FAULT_MASK        0x08U
#define BAT_FAULT_SHIFT       0x03U
#define NTC_FAULT_MASK        0x07U
#define NTC_FAULT_SHIFT       0x00U
/*FAULT TYPE*/
#define WATCHDOG_TIMEOUT      0x01
#define BATT_OVERVOL_FAULT    0x01
#define TS_WARM               0x02
#define TS_COOL               0x03
#define TS_COLD               0x05
#define TS_HOT                0x06
#define CHARGE_INPUT_FAULT_REG    0x01
#define CHARGE_THERM_SHUTDOWN_REG 0x01

#define BATV_ADC_REG        MP2731_REG_0E
#define BATV_ADC_MASK       0xFFU
#define BATV_ADC_SHIFT      0x00U
#define BATV_ADC_SU         20      //mv smallest uint
#define BATV_ADC_OFFSET     0

#define SYSV_ADC_REG        MP2731_REG_0F
#define SYSV_ADC_MASK       0xFFU
#define SYSV_ADC_SHIFT      0x00U
#define SYSV_ADC_SU         20      //mv smallest uint
#define SYSV_ADC_OFFSET     0

#define TS_ADC_REG          MP2731_REG_10
#define TS_ADC_MASK         0xFFU
#define TS_ADC_SHIFT        0x00U
#define TS_ADC_SU           39
#define TS_ADC_OFFSET       0

#define VBUSV_ADC_REG       MP2731_REG_11
#define VBUSV_ADC_MASK      0xFFU
#define VBUSV_ADC_SHIFT     0x00U
#define VBUSV_ADC_SU        60      //mv smallest uint
#define VBUSV_ADC_OFFSET    0

#define ICHGR_ADC_REG       MP2731_REG_12
#define ICHGR_ADC_MASK      0xFFU
#define ICHGR_ADC_SHIFT     0x00U
#define ICHGR_ADC_SU        17
#define ICHGR_ADC_OFFSET    0

#define IIN_REG        MP2731_REG_13
#define IIN_MASK       0xFFU
#define IIN_SHIFT      0x00U
#define IIN_ADC_SU     13
#define IIN_ADC_OFFSET  0

#define VINPPM_STAT_REG        MP2731_REG_14
#define VINPPM_STAT_MASK       0x80U
#define VINPPM_STAT_SHIFT      0x07U

#define JEITA_VSET_REG      MP2731_REG_16
#define JEITA_VSET_MASK     0x80U
#define JEITA_VSET_SHIFT    0x07U
#define JEITA_VSET_0        0
#define JEITA_VSET_1        1//default

#define JEITA_ISET_REG       MP2731_REG_16
#define JEITA_ISET_MASK      0x40U
#define JEITA_ISET_SHIFT     0x06U
#define JEITA_ISET_0         0     //50% of ICHG
#define JEITA_ISET_1         1     //16.7% of ICHG

#define VTH_HOT_MASK         0x20U
#define VTH_HOT_SHIFT        0x05U
#define VTH_HOT_0            0    //34% of 60 celsius
#define VTH_HOT_1            1    //36% of 55 celsius

#define VTH_WARM_MASK        0x18U
#define VTH_WARM_SHIFT       0x03U
#define VTH_WARM_0           0    //43% of 40 celsius
#define VTH_WARM_1           1    //40% of 45 celsius
#define VTH_WARM_2           2    //38% of 50 celsius
#define VTH_WARM_3           3    //36% of 55 celsius

#define VTH_COOL_MASK        0x06U
#define VTH_COOL_SHIFT       0x01U
#define VTH_COOL_0           0    //72% of 0 celsius
#define VTH_COOL_1           1    //68% of 5 celsius
#define VTH_COOL_2           2    //64% of 10 celsius
#define VTH_COOL_3           3    //60% of 15 celsius

#define VTH_COLD_MASK        0x01U
#define VTH_COLD_SHIFT       0x00U
#define VTH_COLD_0           0    //72% of 0 celsius
#define VTH_COLD_1           1    //68% of 5 celsius

#define SAFTY_TIMER_REG      MP2731_REG_17
#define SAFTY_TIMER_MASK       0x80U
#define SAFTY_TIMER_SHIFT      0x07U
#define SAFTY_TIMER_NORMAL      0
#define SAFTY_TIMER_EXPIRATION  1

#define CHARGE_INPUT_FAULT    0x01
#define CHARGE_THERM_SHUTDOWN 0x02
#define CHARGE_SAFE_TIMEOUT   0x03


//! \brief Defines a macro to read a block of registers.
//!
#define MP2731_READBLOCK(reg, buff, len) \
   hwi_I2C_Mem_Read(1 , MP2731_ADDR , reg , I2C_MEMADD_SIZE_8BIT , buff , len , 0xfff)

 typedef struct
 {
     uint8_t  reg;
     uint8_t  mask;
     uint8_t  shift;
     uint8_t unit;
     uint16_t offset;
 }REG_VALUE_INFO_T;

 typedef struct
 {
     uint8_t  reg;
     uint8_t  mask;
     uint8_t  shift;
 }reg_shift_info;

 enum mps_chip_id {

    MP2731_CHIP_ID = 0x00,
};
extern uint8_t e_adcEnabled;

typedef enum{
    MP2731_CHRG_FLT_NORMAL = 0,
    MP2731_CHRG_FLT_INPUT_OVP,
    MP2731_CHRG_FLT_THERM_OFF,
    MP2731_CHRG_FLT_TIMER_EXPIR,
}MP2731_CHRG_FALUT_E;

typedef enum{
    MP2731_NTC_FLT_NORMAL = 0,
    MP2731_NTC_FLT_WARM = 2,
    MP2731_NTC_FLT_COOL = 3,
    MP2731_NTC_FLT_COLD = 5,
    MP2731_NTC_FLT_HOT = 6,
}MP2731_NTC_FAULT_E;

 typedef enum
 {
     VREG,         //charge current limit
     ITERM,       //Termination current limit
     IPRECHG,     //precharge current limit
     ICHG,       //fast charge current limit
     SYS_MIN,     //min sys voltage
     IINLIM,      //input current limit
     ITEM_NEED_SET,
     VINDPM,

     BATV_ADC,
     SYSV_ADC,
     TS_ADC,
     VBUSV_ADC,
     ICHGR_ADC,
     ENTERM,
     ENTIMER,
     VALUE_ITEM_NUM,

 }VALUE_ITEM_E;

 typedef enum
 {
     WTDG_FAULT,
     BOOST_FAULT,
     CHRG_FAULT,
     BAT_FAULT,
     NTC_FAULT,
     FAULT_FLAG_NUM
 }FAULT_FLAG_E;

//Limits Need Set
#define CHG_VOLT_LIM             4140
#define TERM_CURR_LIM            120
#define PRECHG_CURR_LIM          150
#define ADJ_PRECHG_CURR_LIM      150
#define FAST_CHARGE_CURR_LIM     1000
#define ADJ_FAST_CHARGE_CURR_LIM 1000
#define INPUT_CURR_LIM           3250 //mA
#define SYS_MIN_VOLT_LIM         3500

uint8_t Dev_MP2731_Init(void);
uint8_t Dev_MP2731_EnableCharge(uint8_t enable);
uint8_t Dev_MP2731_EnableADC(uint8_t enable);
uint8_t Dev_MP2731_GetStatus(uint8_t *vinStatus, uint8_t *chgStatus, uint8_t *ntcFloatStatus, uint8_t *thermStatus,uint8_t *vsysStatus);
uint8_t Dev_MP2731_GetAdc(uint16_t *curr, uint16_t *vBusVolt, uint16_t *selfBatVolt, uint16_t  *vsysVolt, uint16_t *tsValue);
uint8_t Dev_MP2731_GetVindpm(uint16_t *vinDpm);
uint8_t Dev_MP2731_ChrgCurrent(uint16_t u16Current);
uint8_t Dev_MP2731_GetWatchdogFault(void);
uint8_t Dev_MP2731_ThermalShutdown(void);
uint8_t Dev_MP2731_GetOutputVoltageFault(void);
MP2731_NTC_FAULT_E Dev_MP2731_GetNtcFault(void);
uint8_t Dev_MP2731_ReadRegBits(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t* data);
uint8_t Dev_MP2731_ChrgVoltLimit(uint16_t u16Volt);
uint8_t Dev_MP2731_Set_SYS_MIN(uint32_t vol);
uint8_t Dev_MP2731_getDeviceID(uint8_t *mp2731_id);

uint8_t Dev_MP2731_GetNtcTemp(uint8_t *temp);
uint8_t Check_MP2731_RegBits (uint8_t reg, uint8_t mask, uint8_t shift, uint8_t data);
uint8_t Dev_MP2731_RstWdgTimer(void);

void Dev_MP2731_UpdateCheckinfo_ICHG(uint16_t u16Current);
void Dev_MP2731_UpdateCheckinfo_VREG(uint16_t u16Volt);
void Dev_MP2731_CheckRegData(void);
uint8_t Dev_MP2731_Set_LowCurrent_Charge(void);
uint8_t Dev_MP2731_Set_DefaultCurrent_Charge(void);
uint8_t Dev_MP2731_boost_back_work_around(void);
void Dev_MP2731_SetNormalChargeCurrent(void);
void Dev_MP2731_SetABNormalChargeCurrent(void);

#define UNKONWN_CHARGER_IC 0xff
#define ID_MP2731 0x0

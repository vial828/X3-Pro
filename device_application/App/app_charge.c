#include "HWI_Hal.h"
#include "stratos_defs.h"
#include "kernel.h"
#include "app_charge.h"
#include "dev_adc.h"
#include "log.h"
#include "dev_temperature.h"
#include "manager.h"
#include "rtc.h"
#include "dev_gauge_bq27z561r2.h"
#include "dev_bq25898x.h"
#include "mp2731.h"
#include "usr_cmd.h"
#include "power.h"
#include "error_code_led_output.h"
#include "math.h"
#include "stdlib.h"
#include "stdbool.h"
#include "HWI_gpio.h"
#include "batTimer.h"
#include "self_flash.h"
#include "error_code.h"


#define MAX_VOLT    4.40f
#define PERCENT96_V 4.10f
#define PERCENT96_A -0.100f
#define PERCENT95_V 4.10f
#define PERCENT95_A -0.180f
#define PERCENT93_V 4.10f
#define PERCENT93_A -0.250f

#define BAT_TOTAL_JOUELS  38000
#define JITTER          100

#define JOUELS_PERCENT_95  (BAT_TOTAL_JOUELS*0.95f)
#define JOUELS_PERCENT_75  (BAT_TOTAL_JOUELS*0.75f)
#define JOUELS_PERCENT_50  (BAT_TOTAL_JOUELS*0.50f)
#define JOUELS_PERCENT_25  (BAT_TOTAL_JOUELS*0.25f)
#define JOUELS_PERCENT_15  (BAT_TOTAL_JOUELS*0.15f)

#define PCT95_VOLT 4.364f
#define PCT75_VOLT 4.220f
#define PCT50_VOLT 4.040f
#define PCT25_VOLT 3.860f


#define SLOPE1 (BAT_TOTAL_JOUELS*0.05f)/(MAX_VOLT - PCT95_VOLT)
#define SLOPE2 (BAT_TOTAL_JOUELS*0.20f)/(PCT95_VOLT - PCT75_VOLT)
#define SLOPE3 (BAT_TOTAL_JOUELS*0.25f)/(PCT75_VOLT - PCT50_VOLT)
#define SLOPE4 (BAT_TOTAL_JOUELS*0.25f)/(PCT50_VOLT - PCT25_VOLT)
#define SLOPE5 (BAT_TOTAL_JOUELS*0.24f)/(PCT25_VOLT - MIN_VOLT)


#define CHARGE_TYPE_TIME   1000

#define RECHG_THRESHOLD    96
#define RECHG_FULL    100


chrg_driver_e current_chrg_drv;
//static timer_t * bat_damage_timer;
//static timer_t * get_batv_timer;
static ptimer_t bat_damage_timer;
static ptimer_t get_batv_timer;
uint32_t record_bat_v = 0;
uint8_t rcord_cnt=0;
uint8_t boot_flag=0;
uint8_t socbufL[2];
uint8_t socbufM[2];
uint8_t voltbufL[2];
uint8_t voltbufM[2];
uint8_t currentL[2];
uint8_t currentM[2];
uint8_t charging_mode;
static uint8_t Dev_InputFault = 0;
extern int16_t charge_timeout;
extern int16_t bat_volt_damage_protect;

void SetInputFault(uint8_t status);

typedef enum{
    discharge_s,
    chg_s,
    chg_full_s,
    chg_suspend_s,
}charge_state_e;

chrg_driver_e app_get_current_chrg_drv(void)
{
    return current_chrg_drv;
}

typedef struct{
    uint8_t charge_allow;
    float joules;
    float i_sense;
    float volt;
    uint8_t bat_left;
    uint8_t usb_plug;
    uint8_t error_time_over;
  charge_state_e charge_state;
  BATTERY_TYPE_E batType;
    uint8_t log_chg_state;
    uint8_t log_chg_partab;
    uint8_t suspend_bit_map;
    uint16_t rsoc;    
    int16_t current;
}charge_context_t;

static charge_context_t cc;

typedef struct
{
    uint8_t Regnums;
    uint8_t au8Reg[BQ25898E_REG_NUMS];
    uint8_t u8OtgStat;
    uint8_t u8VbusStat;
    uint8_t u8ChrgStat;
    uint8_t u8pgStat;
    uint8_t u8VsysStat;
    uint8_t i8ChrgicNtcTemp;
    uint16_t u16Current;
    uint16_t u16VbusVolt;
//    uint8_t u8ntcFloatStat;
//    uint8_t u8thermStat;
    uint16_t u16BatVolt;
    uint16_t u16VsysVolt;
    uint16_t u16tsVal;
}charge_ic_t;




static charge_ic_t c_ic;

const static cic_ntc_map_t ntc_map[] = {
   {55, 3.535  },{54,   3.6615 },{53,   3.7793 },{52,   3.9016 },{51,   4.0288 },
   {50, 4.1609 },{49,   4.3115 },{48,   4.4541 },{47,   4.6023 },{46,   4.7565 },
   {45, 4.9169 },{44,   5.0964 },{43,   5.2698 },{42,   5.4502 },{41,   5.6381 },
   {40, 5.8336 },{39,   6.0455 },{38,   6.2574 },{37,   6.4781 },{36,   6.7081 },
   {35, 6.9479 },{34,   7.2042 },{33,   7.4644 },{32,   7.7359 },{31,   8.019  },
   {30, 8.3145 },{29,   8.6292 },{28,   8.9499 },{27,   9.2848 },{26,   9.6346 },
   {25, 10     },{24,   10.38  },{23,   10.7771},{22,   11.1923},{21,   11.6264},
   {20, 12.0805},{19,   12.5474},{18,   13.0429},{17,   13.5615},{16,   14.1047},
   {15, 14.6571},{14,   15.2507},{13,   15.8727},{12,   16.5247},{11,   17.2084},
   {10, 17.8891},{9,18.6373 },{8    ,19.4224 },{7   ,20.2466 },{6   ,21.112  },
   {5   ,21.9603 },{4   ,22.9095 },{3   ,23.907  },{2   ,24.9556 },{1   ,26.0585 },
   {0   ,27.1433 },{-1, 28.3574},{-2,   29.6354},
};

/*************************************************************************************************
 * @brief    : read charge driver id
 * @return   : chrg_driver_e
************************************************************************************************/
chrg_driver_e app_charge_read_driver_id(void)
{
    uint8_t chipID = 0;
    chrg_driver_e ret = no_DRIVER;
    //Reg 0x14
    if(HWI_OK == Dev_BQ25898x_getDeviceID(&chipID)){
        if(((chipID>>3)&0x07) == BQ25898E_CHIP_ID)
        {
            LOGD("chg ic bq25898E");
            ret = BQ25898E;
        }
        else{
            LOGE("chg ic bq25898x");
            ret =  no_DRIVER;
        }
    }else{
        //Reg 0x4B
        if(HWI_OK == (Dev_MP2731_getDeviceID(&chipID)))
        {
            if(((chipID>>3)&0x07) == MP2731_CHIP_ID){
                LOGD("chg ic mp2731");
                ret =  MP2731;
            }else{
                LOGE("chg ic mps");
                ret =  no_DRIVER;
            }
        }
        else
        {
            LOGE("i2C communication error");
            ret =  no_DRIVER;
        }
    }
    return ret;
}

/*************************************************************************************************
  * @brief    : send current cic register value to pc_tool
  * @return   : void
*************************************************************************************************/
void app_SendChargeICParameter(void)
{
    switch(current_chrg_drv)
    {
        case(BQ25898E):
            c_ic.Regnums = BQ25898E_REG_NUMS;
            /*read all register value*/
            for (uint8_t u8Loop = 0; u8Loop < BQ25898E_REG_NUMS; u8Loop++)
            {
                Dev_BQ25898X_ReadRegBits(BQ25898X_REG_00 + u8Loop,0xFF,0,c_ic.au8Reg + u8Loop);
            }
            /*read specific bit*/
            Dev_BQ25898X_GetStatus(&c_ic.u8VbusStat,&c_ic.u8ChrgStat,&c_ic.u8pgStat,&c_ic.u8VsysStat);
            Dev_BQ25898X_GetAdc(&c_ic.u16Current,&c_ic.u16VbusVolt,&c_ic.u16BatVolt,&c_ic.u16VsysVolt,&c_ic.u16tsVal);
            Dev_BQ25898X_ReadRegBits(BQ25898X_REG_03,BQ25898X_OTG_CONFIG_MASK,BQ25898X_OTG_CONFIG_SHIFT,&c_ic.u8OtgStat);
            Dev_BQ25898X_GetNtcTemp(&c_ic.i8ChrgicNtcTemp);

            /*send the result to pc tool*/
            respond_usr_cmd(USR_CMD_GET_CHARGE, (uint8_t*)&c_ic, sizeof(c_ic));
            break;

        case(MP2731):
            /*read all register value*/
            /*c_ic.Regnums = MP2731_REG_NUMS;
            for (uint8_t u8LoopS = 0; u8LoopS < MP2731_REG_NUMS; u8LoopS++)
            {
                Dev_MP2731_ReadRegBits(MP2731_REG_00 + u8LoopS,0xFF,0,c_ic.au8Reg + u8LoopS);
            }

            //read specific bit
            Dev_MP2731_GetStatus(&c_ic.u8VbusStat,&c_ic.u8ChrgStat,0,0,&c_ic.u8VsysStat);
            Dev_MP2731_GetAdc(&c_ic.u16Current,&c_ic.u16VbusVolt,&c_ic.u16BatVolt,&c_ic.u16VsysVolt,&c_ic.u16tsVal);
//            Dev_MP2731_ReadRegBits(BQ25898X_REG_03,BQ25898X_OTG_CONFIG_MASK,BQ25898X_OTG_CONFIG_SHIFT,&c_ic.u8OtgStat);
            Dev_MP2731_GetNtcTemp(&c_ic.i8ChrgicNtcTemp);

            //send the result to pc tool//
            respond_usr_cmd(USR_CMD_GET_CHARGE, (uint8_t*)&c_ic, sizeof(c_ic));*/
            break;

        default:
            //LOGE("No charge driver IC detected");
            break;
    }
}
/*************************************************************************************************
  * @brief    : read battery id : BAK or SUMSUNG
  * @return   : void
*************************************************************************************************/
void app_bat_id_init(void)
{
#if 0
  GPIO_InitTypeDef  GPIO_InitStruct;
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = BAT_ID_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  HAL_GPIO_Init(BAT_ID_PORT, &GPIO_InitStruct);
  #endif
}

BATTERY_TYPE_E app_read_bat_id(void)
{
    BATTERY_TYPE_E batType = BATTERY_GP;

    if (HWI_PIN_RESET == hwi_GPIO_ReadPin(GET_BAT_ID_E))
    {
        batType = BATTERY_GP;
    }
    else
    {
        batType = BATTERY_BYD;
    }

    //BY ADC
    #if 0
    if(0 <= get_bat_ID() <= 256)
    {
        batType = BATTERY_GP;
    }
    else
    {
        batType = BATTERY_BYD;
    }
    #endif
    return batType;
}

/*************************************************************************************************
  * @brief    : get bat soc
  * @return   : battery soc
*************************************************************************************************/
uint8_t app_get_bat_left(void)
{
    return cc.bat_left;
}

/*************************************************************************************************
  * @brief    : set charge current to fast mode
  * @return   : void
*************************************************************************************************/
void app_enableUsbChrgBat(void)
{
      uint8_t u8RegVal = 0;
    //HAL_GPIO_WritePin(SW1_EN_PORT,SW1_EN_PIN,GPIO_PIN_SET);
    switch(current_chrg_drv)
  {
    case(BQ25898E):
      Dev_BQ25898X_ReadRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, &u8RegVal);
      if (((uint16_t)u8RegVal*BQ25898X_ICHG_LSB) != FAST_CHARGE_CURR_LIMIT)
      {
          Dev_BQ25898X_ChrgCurrent(FAST_CHARGE_CURR_LIMIT);
      }
         break;
    case(MP2731):
      Dev_MP2731_ReadRegBits(MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, &u8RegVal);
      if (((uint16_t)u8RegVal*ICHG_SU) != FAST_CHARGE_CURR_LIM)
      {
        Dev_MP2731_ChrgCurrent(FAST_CHARGE_CURR_LIM);
      }
        break;
    default:
    //LOGE("No charge driver IC detected");
      break;
  }
}

/*************************************************************************************************
  * @brief    : set charge current to 0
  * @return   : void
*************************************************************************************************/
void app_disableUsbChrgBat(void)
{
     uint8_t u8RegVal = 0;
  switch(current_chrg_drv)
  {
    case(BQ25898E):
      Dev_BQ25898X_ReadRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, &u8RegVal);
      if (u8RegVal != 0)
      {
          Dev_BQ25898X_ChrgCurrent(0);
      }
        break;
    case(MP2731):
      Dev_MP2731_ReadRegBits(MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, &u8RegVal);
      if (u8RegVal != 0)
      {
          Dev_MP2731_ChrgCurrent(0);
      }
        break;
    default:
    //LOGE("No charge driver IC detected");
        break;
  }
}

/*************************************************************************************************
  * @brief    : enable/disable cic charge
  * @param    : true or false
  * @return   : void
*************************************************************************************************/
void app_charge_enable(uint8_t en)
{

    switch(current_chrg_drv)
    {
        case(BQ25898E):
            /*set cic enable register bit*/
            Dev_BQ25898X_EnableCharge(en);
            /*set cic enable pin*/
            if(en)
            {
                //HAL_GPIO_WritePin((CHARGE_ENABLE_PORT, CHARGE_ENABLE_PIN, GPIO_PIN_RESET);
                hwi_GPIO_WritePin(CHRG_EN_E, HWI_PIN_RESET);
            }
            else
            {
                //HAL_GPIO_WritePin((CHARGE_ENABLE_PORT, CHARGE_ENABLE_PIN, GPIO_PIN_SET);
                hwi_GPIO_WritePin(CHRG_EN_E, HWI_PIN_SET);
            }
            break;

        case(MP2731):
            /*set cic enable register bit*/
            Dev_MP2731_EnableCharge(en);
            /*set cic enable pin*/
            if(en)
            {
                hwi_GPIO_WritePin(CHRG_EN_E, HWI_PIN_RESET);
            }
            else
            {
                hwi_GPIO_WritePin(CHRG_EN_E, HWI_PIN_SET);
            }
            break;

        default:
            //LOGE("No charge driver IC detected");
            break;
    }
}

/*************************************************************************************************
  * @brief    : set cic charge current
  * @param    : current
  * @return   : void
*************************************************************************************************/
void app_SetChargeCurrentLimit(uint16_t u16Current)
{
    switch(current_chrg_drv)
    {
        case(BQ25898E):
            if (Dev_BQ25898X_ChrgCurrent(u16Current) != 0) /*set cic charge current and check the result*/
            {
                post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
            }
            else
            {   /*update period check info (register value)*/
                Dev_BQ25898x_UpdateCheckinfo_ICHG(u16Current);
            }
            break;
        case(MP2731):
            if (Dev_MP2731_ChrgCurrent(u16Current) != 0) /*set cic charge current and check the result*/
            {
                post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
            }
            else
            {   /*update period check info (register value)*/
                Dev_MP2731_UpdateCheckinfo_ICHG(u16Current);
           }
            break;
        default:
             //LOGE("No charge driver IC detected");
            break;
  }
}

/*************************************************************************************************
  * @brief    : set cic charge voltage
  * @param    : voltage
  * @return   : void
*************************************************************************************************/
void app_SetChargeVoltLimit(uint16_t u16Volt)
{
    switch(current_chrg_drv)
    {
        case(BQ25898E):
            if (Dev_BQ25898X_ChrgVoltLimit(u16Volt) != 0)/*set cic charge voltage and check the result*/
            {
                post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
            }
            else
            {   /*update period check info (register value)*/
                Dev_BQ25898x_UpdateCheckinfo_VREG(u16Volt);
            }
            break;
       case(MP2731):
            if (Dev_MP2731_ChrgVoltLimit(u16Volt) != 0)/*set cic charge voltage and check the result*/
            {
                post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
            }
            else
            {   /*update period check info (register value)*/
                Dev_MP2731_UpdateCheckinfo_VREG(u16Volt);
            }
            break;
       default:
          //LOGE("No charge driver IC detected");
            break;
  }
}

/*************************************************************************************************
  * @brief    : calculate joules through voltage
  * @return   : joules
*************************************************************************************************/
static float get_joules_from_voltage(void)
{
    float joules;

    float vbat = dev_get_adc_result()->vbat;
    //LOGD("%s vbat=%0.3f\r\n", __func__, vbat);

    /*linear calculation for specific sections*/
    if (vbat > MAX_VOLT)
    {
        joules = BAT_TOTAL_JOUELS+JITTER;
    }
    else if (vbat > PCT95_VOLT)
    {
        joules = (vbat-PCT95_VOLT)*SLOPE1 + JOUELS_PERCENT_95;
    }
    else if (vbat > PCT75_VOLT)
    {
        joules = (vbat-PCT75_VOLT)*SLOPE2 + JOUELS_PERCENT_75;
    }
    else if (vbat > PCT50_VOLT)
    {
        joules = (vbat-PCT50_VOLT)*SLOPE3 + JOUELS_PERCENT_50;
    }
    else if (vbat > PCT25_VOLT)
    {
        joules = (vbat-PCT25_VOLT)*SLOPE4 + JOUELS_PERCENT_25;
    }
    else if (vbat > MIN_VOLT)
    {
        joules = (vbat-MIN_VOLT)*SLOPE5;
    }
    else
    {
        joules = 0.0f;
    }
    //LOGD("%s joules=%0.3f\r\n", __func__, joules);
    return joules;
}

/*************************************************************************************************
  * @brief    : calculate soc percentage
  * @return   : soc percentage
*************************************************************************************************/
uint8_t app_get_bat_left_percentage(void)
{
    uint32_t left;
    uint32_t total;
    //uint8_t u8ChrgStat;
    uint16_t systemState;
    static uint16_t empty_cnt = 0;

    float vbat = dev_get_adc_result()->vbat;
    float i = dev_get_adc_result()->i_sense;
    cc.i_sense = i;
    cc.volt = vbat;
    left = cc.bat_left;
    systemState = app_get_state();
   /*if(app_get_current_chrg_drv() == BQ25898E)
    {
      Dev_BQ25898X_GetStatus(0,&u8ChrgStat,0,0);
     //LOGD("app_get_bat_left_percentage:u8ChrgStat is %d\n", u8ChrgStat);
    }
    else if(app_get_current_chrg_drv() == MP2731)
    {
      Dev_MP2731_GetStatus(0,&u8ChrgStat,0,0,0);
    }
    if(BQ25898X_CHG_STAT_CHGDONE == u8ChrgStat || CHARGE_TERM_DONE == u8ChrgStat)//full power
    {
        LOGD("app_get_bat_left_percentage:BQ25898X_CHG_STAT_CHGDONE,100 \n");
        cc.joules = BAT_TOTAL_JOUELS + JITTER;
        left = 100;
    }*/

    if((vbat < MIN_VOLT) && (systemState != STATE_NORMAL_HEAT) && (systemState != STATE_BOOST_HEAT))//empty power
    {
        //LOGD("app_get_bat_left_percentage:0\n");
        empty_cnt++;
        //debounce soc
        //the duration of vbat lower than MIN_VOLT must be greater than 15 seconds
        //15*1000/16 ms   16 is the period of app_charge_task
        if(empty_cnt >= 15*1000/16)
        {
            left = 0;
            cc.joules = 0;
            empty_cnt = 0;
        }
    }
    else
    {
        empty_cnt = 0; //clear empty cnt
        total = BAT_TOTAL_JOUELS;
        left = cc.joules;
        left = (left/100)*100;
        left = left*100/total;
        //Do not show left 0 if joules > 0 (accuracy = 1%)
        if ((0 == left) && (cc.joules > 0))
        {
            left = 1;
        }
        //LOGD("app_get_bat_left_percentage: cc.joules=%f\r\n",cc.joules);
    }

    /*avoid display soc = 0 when vbat>=MIN_VOLT*/
    if ((0 == left) && (vbat >= MIN_VOLT) && (systemState != STATE_NORMAL_HEAT) && (systemState != STATE_BOOST_HEAT))
    {
        left = 1;
        if (0 == cc.joules)
        {
            cc.joules = 50;
        }
    }

    WriteBackupRegister(BKP_DR0, *(uint32_t *)&cc.joules); //store in rtc register
    return (left > 100 ? 100:left);
}

void app_setChargerState(uint8_t state)
{
    cc.log_chg_state =state;
}

/*************************************************************************************************
  * @brief    : get charge state
  * @return   : void
*************************************************************************************************/
uint8_t app_GetChargerState(void)
{
    return cc.log_chg_state;
}


/*************************************************************************************************
  * @brief    : set charge partab
  * @return   : void
*************************************************************************************************/
void setChargerPartab(uint8_t value)
{
    cc.log_chg_partab = value;
}

/*************************************************************************************************
  * @brief    : get charge partab
  * @return   : partab
*************************************************************************************************/
uint8_t app_GetChargerPartab(void)
{
    return cc.log_chg_partab;
}


/*************************************************************************************************
  * @brief    : update charge custom log item fuel
  * @return   : fuel
*************************************************************************************************/
uint8_t app_updateFue(void)
{
      uint8_t fuel;
    if(cc.bat_left <= 9)
    {
        fuel = 0;
    }
    else if(cc.bat_left <= 34)
    {
        fuel = 1;
    }
    else if(cc.bat_left <= 69)
    {
        fuel = 2;
    }
    else if(cc.bat_left <=94)
    {
        fuel = 3;
    }
    else if(cc.bat_left<=99)
    {
        fuel = 4;
    }
    else
    {
        fuel = 5;
    }
    return fuel;
}

/*************************************************************************************************
  * @brief    : send cic current register value to pc_tool for hardware debug
  * @return   : void
*************************************************************************************************/
void app_send_charge_IC_reg(void)
{
   switch(current_chrg_drv)
      {
        case(BQ25898E):
          for (uint8_t u8Loop = 0; u8Loop < BQ25898E_REG_NUMS; u8Loop++)
          {
              Dev_BQ25898X_ReadRegBits(BQ25898X_REG_00 + u8Loop,0xFF,0,c_ic.au8Reg + u8Loop);
          }
          break;
        case(MP2731):
          /*for (uint8_t u8Loop = 0; u8Loop < MP2731_REG_NUMS; u8Loop++)
          {
              Dev_MP2731_ReadRegBits(MP2731_REG_00 + u8Loop,0xFF,0,c_ic.au8Reg + u8Loop);
          }*/
          break;
        default:
         //LOGE("No charge driver IC detected");
          break;
  }
      respond_usr_cmd(CHARGE_REG_LOG, (uint8_t*)&c_ic, sizeof(c_ic.au8Reg));
}

/*************************************************************************************************
  * @brief    : record in/out joules to history data every minute
  * @return   : void
*************************************************************************************************/
static void record_in_out_joules(void)
{
    float i = dev_get_i_sense();
    float v = dev_get_vbat_volt();
    static float inJoules = 0;
    static float outJoules = 0;
    static uint32_t tick = 0;
    static uint8_t timer = 0;
    data_change_frequent_t* pDataChangeFreq = NULL;
    float w = i*v;

    if (0 == timer)
    {
        tick = GetTick();
        timer = 1;
    }
    w = w*0.016f; //16ms=0.016sec
    if (w > 0)
    {
        outJoules += w;
    }
    else
    {
        inJoules -= w;
    }
    if ((1 == timer) && (TicsSince(tick) > 60000))
    {
        pDataChangeFreq = get_data_change_frequent_from_ram();
        pDataChangeFreq->lifeCycleData[in_WH] += (uint32_t)inJoules;
        pDataChangeFreq->lifeCycleData[out_WH] += (uint32_t)outJoules;
        inJoules = 0;
        outJoules = 0;
        timer = 0;
    }
}

/*************************************************************************************************
  * @brief    : integrate joule by voltage and current
  * @param1   : None
  * @return   : None
*************************************************************************************************/
static void add_joules(void)
{
    float i = dev_get_i_sense();
    float v = dev_get_vbat_volt();

    float w = i*v;
    float joules_from_voltage = 0;
    uint16_t systemState;
    static uint32_t timer = 0;
    static uint32_t tick = 0;
    cc.i_sense = i;

    w = w*0.016f; //16ms=0.016sec

    cc.joules -= w;

    if(cc.joules < 0)
    {
        cc.joules = 0;
    }

    //joules must be smaller than 100% if not really charge full
    if ((cc.bat_left < 100) && (cc.joules >= (BAT_TOTAL_JOUELS - 100)))
    {
        cc.joules = BAT_TOTAL_JOUELS - 100;
    }
    /*joules must be smaller than BAT_TOTAL_JOUELS + JITTER*/
    if (cc.joules > BAT_TOTAL_JOUELS + JITTER)
    {
        cc.joules = BAT_TOTAL_JOUELS + JITTER;
    }

    /*correct integrate jouls with voltage*/
    systemState = app_get_state();
    if (systemState == STATE_IDLE)
    {
        joules_from_voltage = get_joules_from_voltage();
        if (fabs(joules_from_voltage - cc.joules) > JOUELS_PERCENT_25)
        {
            if (0 == timer)
            {
                tick = GetTick();
                timer = 1;
            }
        }
        else
        {
            timer = 0;
        }
        if ((1 == timer) && (TicsSince(tick) > 15*1000))/*debounce voltage*/
        {
            timer = 0;
            LOGD("add_joules: joules_from_voltage=%f, cc.joules=%f\r\n",joules_from_voltage, cc.joules);
            cc.joules = joules_from_voltage;
        }
    }
    //LOGD("add_joules: cc.joules=%f\r\n",cc.joules);
}

/*************************************************************************************************
  * @brief    : get battery joules
  * @param1   : None
  * @return   : joules
*************************************************************************************************/
float app_GetBatJoules(void)
{
#ifdef ENABLE_FUEL_GAUGE
    return 0;
#else
    return cc.joules;
#endif
}

void app_chg_entry_suspend(uint8_t reason)
{
    LOGD("chg from %d to suspend_s reason:%d", cc.charge_state, reason);

    /*set the corresponding bit*/
    cc.charge_state = chg_suspend_s;

    app_charge_enable(false);
    app_setChargerState(CHG_LOCKOUT);
    setChargerPartab(CHG_PARTAB_SUSPEND);

    cc.suspend_bit_map |= reason;
}

void app_chg_try_to_exit_suspend(uint8_t reason)
{
  if(reason == CHG_CIT_SUSPEND){
        app_charge_enable(false);
    }
    /*clear the corresponding bit*/
    cc.suspend_bit_map &= (~reason);
    if(cc.suspend_bit_map == 0){
        LOGD("chg from suspend_s to dischg_s");
        app_setChargerState(CHG_INVALID);
        setChargerPartab(CHG_PARTAB_INVALID);
        cc.charge_state = discharge_s;
    }
}
uint8_t app_get_suspend_state(uint8_t reason)
{
    if(cc.suspend_bit_map & reason)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
/*************************************************************************************************
  * @brief    : app_chg_entry_cit
  * @return   : none
*************************************************************************************************/
void app_chg_entry_cit()
{
    LOGD("chg: from %d to chg_cit_s\r\n", cc.charge_state);
    app_chg_entry_suspend(CHG_CIT_SUSPEND);
}

void app_chg_exit_cit()
{
    LOGD("chg: from cit_s to dischg_s\r\n");
    app_chg_try_to_exit_suspend(CHG_CIT_SUSPEND);
}

void app_chg_entry_error()
{
    LOGD("chg: from %d to chg_error_s\r\n", cc.charge_state);
    app_chg_entry_suspend(CHG_ERROR_SUSPEND);
}

void app_chg_exit_error()
{
    LOGD("chg: from %d to dischg_s\r\n", cc.charge_state);
    app_chg_try_to_exit_suspend(CHG_ERROR_SUSPEND);
}

void app_set_charging_mode(void)
{
    int16_t v_bat = (int16_t) (dev_get_adc_result()->vbat*1000);
    int16_t i_sense = (int16_t) (dev_get_adc_result()->i_sense *1000);
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    int16_t  slow_batv_h= pfrt->slow_batv_h ;
    int16_t  slow_batv_l= pfrt->slow_batv_l ;
    int16_t  slow_chg_isense= pfrt->slow_chg_isense;

    if(i_sense<=-2500){
        charging_mode = FAST_CHARGE_MODE;
    }else if((i_sense > (-1*slow_chg_isense)) &&(v_bat>slow_batv_l && v_bat<slow_batv_h)){
        charging_mode = SLOW_CHARGE_MODE;
    }else{
        charging_mode = NORMAL_CHARGE_MODE;
    }
    LOGD("i_sense:%dmA,v_bat:%dmV,CHG_mode:%d",i_sense,v_bat,charging_mode);
}
uint8_t app_get_charging_mode(void)
{
    return charging_mode;
}

static void get_batv(const ptimer_t tm)
{
    read_error_parameter();
    record_bat_v+=dev_get_adc_result()->vbat*1000;
    rcord_cnt++;
    LOGD("batv:%d mv,i:%.3f", record_bat_v,dev_get_adc_result()->i_sense);
    if(rcord_cnt==10)
    {
        if(record_bat_v < bat_volt_damage_protect*10)
        {
            //#ifdef SSCOM
            post_msg_to_manager_with_arg(op_error_occur, flt_de_bat_damage);
            //#endif
            //LOGD("avg_batv: %.3f", record_bat_v);
        }else{
            if(boot_flag==0)
            {
                app_charge_enable(false);
               if(app_get_current_chrg_drv() == BQ25898E)
                 {
                    uint8_t ret=Dev_BQ25898X_Set_DefaultCurrent_Charge();
                    LOGD("BQ25898X Set DefaultCurrent chg ret is [%d]",ret);
                 }
                else if(app_get_current_chrg_drv() == MP2731)
                 {
                    uint8_t ret=Dev_MP2731_Set_DefaultCurrent_Charge();
                    LOGD("MP2731 Set DefaultCurrent chg ret is [%d]",ret);
                 }
             }
            boot_flag=1;
            app_charge_enable(true);
        }
        record_bat_v=0.0;
        rcord_cnt=0;
        //TIMER_SAFE_DELETE(get_batv_timer);
        if(get_batv_timer){
            if(bat_timer_delete(get_batv_timer, portMAX_DELAY)==pdPASS){
                get_batv_timer = NULL;
            }
        }
    }
}
static void judge_batv(const ptimer_t tm)
{
    if(bat_damage_timer){
        if(bat_timer_delete(bat_damage_timer, portMAX_DELAY)==pdPASS){
            bat_damage_timer = NULL;
        }
    }
    //TIMER_SAFE_RESET(get_batv_timer, 1, TIMER_OPT_PERIOD, get_batv, NULL);
    get_batv_timer = bat_timer_reset_ext(get_batv_timer, "get_batv_timer", 5, TIMER_OPT_PERIOD, get_batv);
    bat_timer_start(get_batv_timer, portMAX_DELAY);
}
static void bat_damage_check_before_charge()
{
    if(boot_flag==0)
    {
        if(app_get_current_chrg_drv() == BQ25898E)
          {
             uint8_t ret=Dev_BQ25898X_Set_LowCurrent_Charge();
             LOGD("BQ25898X Set LowCurrent chg ret is [%d]",ret);
          }
         else if(app_get_current_chrg_drv() == MP2731)
          {
             uint8_t ret=Dev_MP2731_Set_LowCurrent_Charge();
             LOGD("MP2731 Set LowCurrent chg is [%d]",ret);
          }

        app_charge_enable(true);
        //TIMER_SAFE_RESET(bat_damage_timer, 50, TIMER_OPT_ONESHOT, judge_batv, NULL);
        bat_damage_timer = bat_timer_reset_ext(bat_damage_timer, "bat_damage_timer", 50, TIMER_OPT_ONESHOT, judge_batv);
        bat_timer_start(bat_damage_timer, portMAX_DELAY);
    }else{
        //TIMER_SAFE_RESET(get_batv_timer, 1, TIMER_OPT_PERIOD, get_batv, NULL);
        get_batv_timer = bat_timer_reset_ext(get_batv_timer, "get_batv_timer", 5, TIMER_OPT_PERIOD, get_batv);
        bat_timer_start(get_batv_timer, portMAX_DELAY);
    }

}

uint8_t read_time = 0;
/*************************************************************************************************
  * @brief    : read soc from gauge
  * @param1   : none
  * @return   : none
*************************************************************************************************/
void read_soc_from_gauge(void)
{
    static uint8_t soc[2];

    if(read_time<2)
    {
       soc[read_time] = dev_bq27z561r2_getGasSoc();
       //LOGD("soc[%d] = %d",read_time, soc[read_time]);
       read_time++;
    }

     if(read_time % 2 == 0){
          if((soc[0]==soc[1]))
          {
             cc.rsoc = soc[1];
             //LOGD("cc.rsoc = %d",cc.rsoc);
          }
        read_time = 0;
     }
}
/*************************************************************************************************
  * @brief    : charge process
  * @param1   : none
  * @return   : none
*************************************************************************************************/
static void charge_proc(void)
{
    uint8_t bat_left;
    uint8_t usb_plug;
    uint8_t chg_done;
    uint8_t temp_Section;

    static uint8_t usb_switch_flag = 0;
    static uint32_t chg_start_tick = 0;
    float batt = dev_get_adc_result()->bat_temp;
    temp_Section=app_get_battery_temp_section(batt);
#ifdef AUTO_TEST
    uint16_t batv = dev_get_adc_result()->vbat*1000;
#endif

#ifdef ENABLE_FUEL_GAUGE
    bat_left = cc.rsoc;
    if (cc.charge_state ==chg_s && bat_left==100){
        bat_left=99;
    }
#else
    bat_left = app_get_bat_left_percentage();
    add_joules();
    record_in_out_joules();
#endif
    usb_plug = app_check_usb_plug_status();
    if(usb_plug == WELL_USB_PLUG && usb_switch_flag != WELL_USB_PLUG)
    {
        usb_switch_flag = WELL_USB_PLUG;
        post_msg_to_manager_with_arg(op_charge, cable_connect);
    }
    else if(usb_plug == NO_USB_PLUG && usb_switch_flag != NO_USB_PLUG)
    {
        usb_switch_flag = NO_USB_PLUG;
        post_msg_to_manager_with_arg(op_charge, cable_disconnected);
    }
    else if(usb_plug == WRONG_USB_PLUG && usb_switch_flag != WRONG_USB_PLUG)
    {
        usb_switch_flag = WRONG_USB_PLUG;
        post_msg_to_manager_with_arg(op_charge, wrong_charge);
    }

    /*charge fsm*/
    switch(cc.charge_state)
    {
        case discharge_s:
            //if(usb_plug && (batt<45) && (batt > 0.6)){
           if(usb_plug==WELL_USB_PLUG){
#ifdef AUTO_TEST
                if(batv < PRE_HEAT_CUTOFF_VOLT){
                    if(batt <= 35){
#else
                if(bat_left<RECHG_FULL){
#endif
                        cc.charge_state = chg_s;
                        chg_start_tick = GetTick(); //record chg start tick
                        app_setChargerState(CHG_CHARGE);
                        //setChargerPartab(CHG_PARTAB_NORMAL);//put the function into app_set_charge_part_ab
                        app_set_charge_part_ab(temp_Section);
                        LOGD("chg: begin chg,termination I:%d mA", TERM_CURR_LIMIT);
                        //post_msg_to_manager_with_arg(op_charge, charge_begin); /*notice manager to entry charge state*/
                        //post_msg_to_manager_with_arg(op_bat_left, bat_left);/*notice manager to display led*/
                        session_count_check(temp_Section);
                        //Dev_BQ25898X_UpdateVDPM_OS();//This feature needs to be reset after charging starts
                        bat_damage_check_before_charge();
                        vTaskDelay(125);
                        app_set_charging_mode();
                        post_msg_to_manager_with_arg(op_charge, charge_begin);
                        //app_charge_enable(true);
#ifdef AUTO_TEST
                    }
#endif
                }else{
                    cc.charge_state = chg_full_s;
                    app_setChargerState(CHG_FULL);
                    setChargerPartab(CHG_PARTAB_INVALID);
#ifndef AUTO_TEST
                    //post_msg_to_manager_with_arg(op_bat_left, 100); /*notice manager to turn on all led 10s*/
                    post_msg_to_manager_with_arg(op_charge, charge_full); /*notice manager to entry charge full state*/
#endif
                    LOGD("chg: from dischg_s to full_s");
                }
            }
            break;
        case chg_s:
           app_set_charge_part_ab(temp_Section);
           app_set_charge_profile(temp_Section);
           if(app_get_current_chrg_drv() == BQ25898E)
            {
               Dev_BQ25898X_GetStatus(0,&chg_done,0,0); /*check chg full*/
                 if(usb_plug == NO_USB_PLUG || usb_plug == WRONG_USB_PLUG){
                    cc.charge_state = discharge_s;
                    LOGD("chg: from chg_s to dischg_s");
                    app_charge_enable(false);
                    app_setChargerState(CHG_INVALID);
                    setChargerPartab(CHG_PARTAB_INVALID);
                  }else if(BQ25898X_CHG_STAT_CHGDONE == chg_done){
#ifndef ENABLE_FUEL_GAUGE
                    cc.joules = BAT_TOTAL_JOUELS + JITTER;
#endif
                    bat_left=100;
                    cc.bat_left = 100;/*bat_left and joules need change to full when charge done at here*/
                    cc.charge_state = chg_full_s;
                    app_setChargerState(CHG_FULL);
                    setChargerPartab(CHG_PARTAB_INVALID);
                    LOGD("chg: from chg_s to full_s");
                    //post_msg_to_manager_with_arg(op_bat_left, 100); /*notic manager to entry discharge state*/
                    post_msg_to_manager_with_arg(op_charge, charge_full);
                    app_charge_enable(false);
              }
            }
           else if(app_get_current_chrg_drv() == MP2731)
            {
               Dev_MP2731_GetStatus(0,&chg_done,0,0,0); /*check chg full*/
                 if(usb_plug == NO_USB_PLUG || usb_plug == WRONG_USB_PLUG){
                   cc.charge_state = discharge_s;
                   LOGD("chg: from chg_s to dischg_s");
                   app_charge_enable(false);
                   app_setChargerState(CHG_INVALID);
                   setChargerPartab(CHG_PARTAB_INVALID);
                 }else if(CHARGE_TERM_DONE == chg_done){
#ifndef ENABLE_FUEL_GAUGE
                   cc.joules = BAT_TOTAL_JOUELS + JITTER;
#endif
                   bat_left=100;
                   cc.bat_left = 100;/*bat_left and joules need change to full when charge done at here*/
                   cc.charge_state = chg_full_s;
                   app_setChargerState(CHG_FULL);
                   setChargerPartab(CHG_PARTAB_INVALID);
                   LOGD("chg: from chg_s to full_s");
                   //post_msg_to_manager_with_arg(op_bat_left, 100); /*notic manager to entry discharge state*/
                   post_msg_to_manager_with_arg(op_charge, charge_full);
                   app_charge_enable(false);
                 }
            }
            /*send soc to manager if soc changed
            *bat_left =100 has been sended before, can not sended again it will make STATE_IDLE confused
            */
            /*if(cc.bat_left != bat_left && bat_left != 100)
            {
                post_msg_to_manager_with_arg(op_bat_left, bat_left);
                LOGD("battery left:%d joules:%f chg_done:%d\r\n", bat_left, cc.joules, chg_done);
            }*/

            /*check chg timeout  and manager will call app_chg_entry_error */
            if(TicsSince(chg_start_tick) > charge_timeout*60*1000){
                post_msg_to_manager_with_arg(op_charge_timeout,chg_timeout_occur);
             }
          break;
        case chg_full_s:
#ifdef AUTO_TEST
            if(usb_plug == NO_USB_PLUG || usb_plug == WRONG_USB_PLUG || batv <PRE_HEAT_CUTOFF_VOLT){
#else
            if(usb_plug == NO_USB_PLUG || usb_plug == WRONG_USB_PLUG || bat_left<RECHG_THRESHOLD){
#endif
                cc.charge_state = discharge_s;
                LOGD("chg: from full_s to dischg_s");
                app_setChargerState(CHG_INVALID);
                setChargerPartab(CHG_PARTAB_INVALID);
             }
            break;
        case chg_suspend_s:
            /*do nothing*/
              if(hwi_GPIO_ReadPin(CHRG_EN_E)==HWI_PIN_RESET){
                  LOGD("chg: suspend dischg");
                  app_charge_enable(false);
                  LOGD("chg: suspend dischg");
              }
              if(usb_plug == NO_USB_PLUG && (app_get_suspend_state(CHG_TIMEOUT_SUSPEND)==1)){
                  post_msg_to_manager_with_arg(op_charge_timeout, chg_timeout_clear);
            }
            break;
        default:
            break;
    }

    cc.bat_left = bat_left;
}

/*************************************************************************************************
  * @brief    : charge init
  * @param    : None
  * @return   : None
*************************************************************************************************/
void app_charge_init(void)
{
    uint32_t tmp;
    uint32_t gauge_verion;

//    GPIO_InitTypeDef  GPIO_InitStruct;
//
//    /* charge enable GPIO pin configuration */
//    GPIO_InitStruct.Pin       = CHARGE_ENABLE_PIN;
//    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull      = GPIO_NOPULL;
//
//    hwi_GPIO_Init(CHARGE_ENABLE_PORT, &GPIO_InitStruct);
//    //HAL_GPIO_WritePin(CHARGE_ENABLE_PORT, CHARGE_ENABLE_PIN, GPIO_PIN_RESET);
//
//    GPIO_InitStruct.Pin       = USB_INT_PIN;
//    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Pull      = GPIO_NOPULL;
//
//    hwi_GPIO_Init(USB_INT_PORT, &GPIO_InitStruct);
	//hwi_GPIO_Init();
    current_chrg_drv = app_charge_read_driver_id();
    /*init cic register*/
    if(current_chrg_drv == BQ25898E)
    {
        hwi_GPIO_WritePin(CHRG_EN_E, HWI_PIN_RESET);
      if (0 != Dev_BQ25898X_Init())
      {
        //post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
      }
    }
    else if(current_chrg_drv == MP2731)
     {
      //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      hwi_GPIO_WritePin(CHRG_EN_E,HWI_PIN_SET);   //charge_dis
        if ( 0 != Dev_MP2731_Init())
      {
         //post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
      }
     }
    app_GetChargeICParameter();
    cc.charge_state = discharge_s;

    cc.batType = app_read_bat_id();
    LOGD("batType %d", cc.batType);

#ifdef ENABLE_FUEL_GAUGE
        flash_record_t * frt = get_self_flash_record_from_ram();
        gauge_verion= frt->gauge_version;
        LOGD("gauge_verion:%d",gauge_verion);

        if(BATTERY_GP == cc.batType)
        {
            if((gauge_verion != GAUGE_GP_DATA_VERSION)){
                LOGD("gp gauge update");
                Dev_BQ27Z561R2_UpdateDataBlock(BATTERY_GP);
            }
        }
        else
        {
            if((gauge_verion != GAUGE_BYD_DATA_VERSION)){
                LOGD("byd gauge update");
                Dev_BQ27Z561R2_UpdateDataBlock(BATTERY_BYD);
            }
        }
        //LOGD("app_charge_init soc: %d\r\n",cc.rsoc);
#endif
    /*read joules from RTC backup register*/
#ifndef ENABLE_FUEL_GAUGE
    tmp = ReadBackupRegister(BKP_DR0);
    cc.joules = *(float *)&tmp;
    if(cc.joules < 10)
    {
        if (BATTERY_GP == cc.batType)
        {
            cc.joules = get_joules_from_voltage();
        }
        else
        {
            LOGD("non BATTERY_GP\r\n");
        }
        WriteBackupRegister(BKP_DR0, *(uint32_t *)&cc.joules); //store in rtc register
    }
    LOGD("joules is %f\n", cc.joules);
#endif

    /*init some global varient*/
    cc.error_time_over = 0;
    LOGD("init chg to chg_disable_c");
}
void dev_first_get_soc(void)
{
    cc.rsoc = dev_bq27z561r2_getGasSoc();
    LOGD("soc = %d",cc.rsoc);
#ifdef ENABLE_FUEL_GAUGE
        cc.bat_left = cc.rsoc;
#else
        cc.bat_left = app_get_bat_left_percentage();
#endif
}

/*************************************************************************************************
  * @brief    : charge task
  * @param    : None
  * @return   : None
*************************************************************************************************/
void app_charge_task(void)
{
    //process charge fsm
    charge_proc();
}

uint8_t usb_plug_time = 0;
uint8_t usb_plug_status=0;
/*************************************************************************************************
  * @brief    : app_set_usb_status
  * @param    : None
  * @return   : usb status
*************************************************************************************************/
void app_set_usb_status(uint8_t status)
{
   usb_plug_status= status;
}
/*************************************************************************************************
  * @brief    : app_check_usb_plug_status
  * @param    : None
  * @return   : usb status
*************************************************************************************************/
uint8_t app_check_usb_plug_status(void)
{
    return usb_plug_status;
}

/*************************************************************************************************
  * @brief    : app_check_usb_plug
  * @param    : None
  * @return   : None
*************************************************************************************************/
void app_check_usb_Volt(uint16_t vbusVolt)
{
    static uint8_t volt[2];
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    int16_t  wrong_chg_h_mv= pfrt->wrong_chg_h_mv;
    if(usb_plug_time<2)
    {
       volt[usb_plug_time] = vbusVolt;
       usb_plug_time++;
    }

     if(usb_plug_time % 2 == 0){
          if((volt[0]==volt[1]))
          {
             if(vbusVolt >= wrong_chg_h_mv)
             {
                 app_set_usb_status(WRONG_USB_PLUG);
             }else if(vbusVolt < 4000)
             {
                app_set_usb_status(NO_USB_PLUG);
             }else
             {
                 app_set_usb_status(WELL_USB_PLUG);
             }
              usb_plug_time = 0;
          }else{
              volt[0]=volt[1];
              usb_plug_time = 1;
          }
     }
}
/*************************************************************************************************
  * @brief    : get some cic status (usb voltage)
  * @param    : None
  * @return   : None
*************************************************************************************************/
uint16_t G_VbusVolt = 0;
uint16_t app_GetChargeICParameter()
{
    uint8_t u8VbusStat = 0,u8ChrgStat = 0,u8pgStat = 0,u8VsysStat = 0,u8ntcFloatStat = 0,u8thermStat = 0;
    uint16_t u16Current = 0,u16VbusVolt = 0,u16BatVolt = 0,u16VsysVolt = 0,u16tsVal = 0;
    uint8_t u8OtgStat = 0;
    uint8_t au8Reg1[BQ25898E_REG_NUMS] = {0};
    uint8_t au8Reg2[MP2731_REG_NUMS] = {0};
    static uint32_t sys_min = 3000;
    //int8_t i8ChrgicNtcTemp = 0;

    switch(current_chrg_drv)
    {
        case(BQ25898E):
            //LOGD("Charge IC Reg: \r\n");
            for (uint8_t u8Loop = 0; u8Loop < BQ25898E_REG_NUMS; u8Loop++)
            {
                Dev_BQ25898X_ReadRegBits(BQ25898X_REG_00 + u8Loop,0xFF,0,&au8Reg1[u8Loop]);
            }
            /*LOGD("Reg0-Reg9:  0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x ",au8Reg[0],au8Reg[1],
                au8Reg[2],au8Reg[3],au8Reg[4],au8Reg[5],au8Reg[6],au8Reg[7],au8Reg[8],au8Reg[9]);
            LOGD("Reg10-Reg20: 0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x ,0x%x", au8Reg[10],au8Reg[11],
                au8Reg[12],au8Reg[13],au8Reg[14],au8Reg[15],au8Reg[16],au8Reg[17],au8Reg[18],au8Reg[19],au8Reg[20]);*/
            Dev_BQ25898X_GetStatus(&u8VbusStat,&u8ChrgStat,&u8pgStat,&u8VsysStat);
            Dev_BQ25898X_GetAdc(&u16Current,&u16VbusVolt,&u16BatVolt,&u16VsysVolt,&u16tsVal);
            //LOGD("ChargIC VbusStat = %d ChargStat = %d pgStat = %d VsysStat = %d \r\n", u8VbusStat,u8ChrgStat,u8pgStat,u8VsysStat);
            //LOGD("ChargIC Current = %d VbusV = %d BatV = %d VsysV = %d TsVal = %d \r\n", u16Current,u16VbusVolt,u16BatVolt,u16VsysVolt,u16tsVal);

            Dev_BQ25898X_ReadRegBits(BQ25898X_REG_03,BQ25898X_OTG_CONFIG_MASK,BQ25898X_OTG_CONFIG_SHIFT,&u8OtgStat);
            //LOGD("ChargIC otg is %s \r\n",u8OtgStat ? "Enable" : "Disable" );
            //Dev_BQ25898X_GetNtcTemp(&i8ChrgicNtcTemp);
            //LOGD("chargIC ntc temp is %d\r\n",i8ChrgicNtcTemp);
            /* dynamic adjust SYS_MIN voltage for the bug 
            * that when the batterry voltage almost equal SYS_MIN
            * the charge current will be exceed the normal value
            */

            if (app_get_state() != STATE_BOOST_HEAT && app_get_state() != STATE_NORMAL_HEAT)
            {
                if (u16BatVolt< 3100 && sys_min==3000) {
                    LOGD("BatV:%dmV,set cic SYS_MIN:3500mV", u16BatVolt);
                    sys_min = 3500;
                    Dev_BQ25898X_Set_SYS_MIN(3500);
                } else if (u16BatVolt > 3200 && sys_min==3500) {
                    LOGD("BatV:%dmV, set cic SYS_MIN:3000mV", u16BatVolt);
                    sys_min = 3000;
                    Dev_BQ25898X_Set_SYS_MIN(3000);
                }
            }

            app_check_usb_Volt(u16VbusVolt);
            break;

        case(MP2731):
            for (uint8_t u8Loop = 0; u8Loop < MP2731_REG_NUMS; u8Loop++)
            {
                Dev_MP2731_ReadRegBits(MP2731_REG_00 + u8Loop,0xFF,0,&au8Reg2[u8Loop]);
            }
            Dev_MP2731_GetStatus(&u8VbusStat,&u8ChrgStat,&u8ntcFloatStat,&u8thermStat,&u8VsysStat);
            //LOGE("u8VbusStat=%d,u8ChrgStat=%d,u8ntcFloatStat=%d,u8thermStat=%d,u8VsysStat=%d\n", u8VbusStat,u8ChrgStat,u8ntcFloatStat,u8thermStat,u8VsysStat);
            Dev_MP2731_GetAdc(&u16Current,&u16VbusVolt,&u16BatVolt,&u16VsysVolt,&u16tsVal);
            //LOGE("u16Current=%d,u16VbusVolt=%d,u16BatVolt=%d,u16VsysVolt=%d,u16tsVal=%d\n", u16Current,u16VbusVolt,u16BatVolt,u16VsysVolt,u16tsVal);

            if (app_get_state() != STATE_BOOST_HEAT && app_get_state() != STATE_NORMAL_HEAT)
            {
                if (u16BatVolt< 3100 && sys_min==3000) {
                    LOGD("BatV:%dmV,set cic SYS_MIN:3600mV", u16BatVolt);
                    sys_min = 3600;
                    Dev_MP2731_Set_SYS_MIN(3600);
                } else if (u16BatVolt > 3200 && sys_min==3600) {
                    LOGD("BatV:%dmV,set cic SYS_MIN:3000mV", u16BatVolt);
                    sys_min = 3000;
                    Dev_MP2731_Set_SYS_MIN(3000);
                }
            }
            /* USB over voltage */
            if(u16VbusVolt >= 15300)
            {
                SetInputFault(1);
            }
            else
            {
                if(u16VbusVolt <= 13700)
                {
                SetInputFault(0);
                }
                /* plug out USB */
                if (u8VbusStat == 0)
                {
                    u16VbusVolt = 0;
                }
            }
            break;

        default:
            LOGE("No chg IC detected");
            break;
    }

    G_VbusVolt = u16VbusVolt;
    return u16VbusVolt;
}

void app_charge_ic_task(void)
{
    app_GetChargeICParameter();
}

void app_SetNormalChargeCurrent(void)
{
    switch(current_chrg_drv)
    {
        case(BQ25898E):
            Dev_BQ25898X_SetNormalChargeCurrent();
            break;
       case(MP2731):
            Dev_MP2731_SetNormalChargeCurrent();
            break;
       default:
          //LOGE("No charge driver IC detected");
            break;
  }
}

void app_SetABNormalChargeCurrent(void)
{
    switch(current_chrg_drv)
    {
        case(BQ25898E):
            Dev_BQ25898X_SetABNormalChargeCurrent();
            break;
       case(MP2731):
            Dev_MP2731_SetABNormalChargeCurrent();
            break;
       default:
          //LOGE("No charge driver IC detected");
            break;
  }
}

void app_SetMinChargeCurrent(void)
{
    switch(current_chrg_drv)
    {
       case(BQ25898E):
            Dev_BQ25898X_SetMinChargeCurrent();
            break;
       case(MP2731):
            //Dev_MP2731_SetMinChargeCurrent();
            break;
       default:
          //LOGE("No charge driver IC detected");
            break;
  }
}

void app_SetTemporaryChargeCurrent(void)
{
    switch(current_chrg_drv)
    {
       case(BQ25898E):
            Dev_BQ25898X_SetTemporaryChargeCurrent();
            break;
       case(MP2731):
            //Dev_MP2731_SetTemporaryChargeCurrent();
            break;
       default:
          //LOGE("No charge driver IC detected");
            break;
    }
}

void app_SetLowPowerChargerCurrent(void)
{
    float usbv = app_GetVbusVolt()/1000.0;
    float ubat = dev_get_adc_result()->vbat;
    float batTemp = dev_get_adc_result()->bat_temp;
    uint8_t temp_section=app_get_battery_temp_section(batTemp);
    if(ubat > 3.9 && usbv < 5.3 && temp_section==temp_Section_normal){
        LOGD("set chg I to 896mA.\r\n");
        app_SetMinChargeCurrent();
    }
}
/*************************************************
  * @brief    :get battery temp section
  * @param    :temp of battery
  * @return   :the battery temperature section
**************************************************/
uint8_t app_get_battery_temp_section(float temp)
{
    uint8_t ts;
    static uint8_t high_flag = 0;
    //LOGD("get battery temp is : %.2f", temp);
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    int16_t charge_temp_protect ;
    int16_t charge_temp_protect_release = pfrt->charge_temp_protect_relesae;
    int32_t all_heat_time = 0;
    //int32_t all_heat_time = pDataChangeFreq->lifeCycleData[total_heat_time];
    int32_t step2_time = pfrt->step2_session_nums*290;
    if(all_heat_time<step2_time)
    {
        charge_temp_protect = pfrt->charge_temp_protect;
    }
    else{
        charge_temp_protect= pfrt->step2_chg_hot_protect;
    }

    if(TEMP_15<=temp && temp < charge_temp_protect && high_flag == 0){
        ts = temp_Section_normal;
    }
    else if(temp >= charge_temp_protect){
        high_flag = 1;
        ts = temp_Section_protect;
    }
    else if(temp < charge_temp_protect_release && high_flag == 1){
        high_flag = 0;
        ts = temp_Section_normal;
    }
    else if(temp >= charge_temp_protect_release && high_flag == 1){
        ts = temp_Section_protect;
    }
    else
    {
        ts = temp_Section_abnormal;
    }

    return ts;
}

/*****************************************************
  * @brief    :set battery charging current or volt
  * @param    :battery temp section
  * @return   :void
********************************************************/
void  app_set_charge_profile(uint8_t arg)
{
    static uint8_t s_ts = 0;
    uint8_t eol_step = 0;

    eol_step=get_eol_step();
    if(eol_step ==STEP_5){
       return;
    }

    if(arg != s_ts){
        s_ts = arg;
        LOGD("temp sec:%d\r\n",arg);
        switch(s_ts){
            case temp_Section_normal:
                app_SetNormalChargeCurrent();
                break;
            case temp_Section_protect:
                app_SetTemporaryChargeCurrent();
                break;
            case temp_Section_abnormal:
                app_SetABNormalChargeCurrent();
                break;
            default:
                break;
        }
    }
}

/*****************************************************
  * @brief    :set charge part ab
  * @param    :battery temp section
  * @return   :void
********************************************************/
void  app_set_charge_part_ab(uint8_t arg)
{
    switch(arg){
        case temp_Section_normal:
            setChargerPartab(CHG_PARTAB_NORMAL);
            break;
        case temp_Section_protect:
            setChargerPartab(CHG_PARTAB_SLOW_CHG_41);
            break;
        case temp_Section_abnormal:
            setChargerPartab(CHG_PARTAB_SLOW_CHG_BAT_COLD);
        default:
            break;
        }
}

/*************************************************************************************************
  * @brief    : get usb voltage
  * @param    : None
  * @return   : usb voltage
*************************************************************************************************/
uint16_t app_GetVbusVolt(void)
{
    return G_VbusVolt;
}



/*************************************************************************************************
  * @brief    : get usb plug status
  * @param1   : None
  * @return   : usb status
*************************************************************************************************/
/*uint8_t app_check_usb_plug_status(void)
{
    uint8_t usb_plug = 0;
    uint16_t vbusmv = 0;
    vbusmv = app_GetVbusVolt();  

    if((vbusmv >= 14600)||((vbusmv < 4400)&&(vbusmv >= 4000)))
    {
        usb_plug = WRONG_USB_PLUG;
    }
    else if(vbusmv < 4000)
    {
        usb_plug = NO_USB_PLUG;
    }
    else
    {
        if(app_get_current_chrg_drv() == MP2731)
        {
            Dev_MP2731_boost_back_work_around();
            usb_plug = WELL_USB_PLUG;
        }
        else if(app_get_current_chrg_drv() == BQ25898E)
        {
            usb_plug = WELL_USB_PLUG;
        }
    }

//   LOGD("usb=%d",usb_plug);
      return usb_plug;
}*/

/*************************************************************************************************
  * @brief    : get MP2731 Input Fault
  * @param    : None
  * @return   : MP2731 Input Fault
*************************************************************************************************/
uint8_t GetInputFault(void)
{
   return Dev_InputFault;
}
void SetInputFault(uint8_t status)
{
//#ifdef SSCOM
    Dev_InputFault = status;
//#endif
}
void app_RecalculateBat(void)
{
    /*get joules according to battery voltage*/
    cc.joules = get_joules_from_voltage();
}

/*************************************************************************************************
  * @brief    : check cic inter error
  * @return   : none
*************************************************************************************************/
void app_CheckChrgFault(uint8_t *pInputVolFault, uint8_t *pTempOutRangeFault, uint8_t *pChargTimeOutFault)
{
   if(app_get_current_chrg_drv() == BQ25898E)
    {
        uint8_t vinvalue;
        vinvalue = GetInputFault();
        CHRG_FALUT_E value = Dev_BQ25898X_GetChrgFault();
        *pInputVolFault = 0;
        *pTempOutRangeFault = 0;
        *pChargTimeOutFault = 0;
        if(vinvalue == 1)
        {
            *pInputVolFault = 1;
        }
        switch (value)
         {
           case CHRG_FLT_THERM_OFF:
             *pTempOutRangeFault = 1;
              break;
           case CHRG_FLT_TIMER_EXPIR:
             *pChargTimeOutFault = 1;
              break;
           default:
              break;
         }
    }
     else if(app_get_current_chrg_drv() == MP2731)
    {
      uint8_t vinvalue;
      uint8_t tsvalue;
       vinvalue = GetInputFault();
       tsvalue = Dev_MP2731_ThermalShutdown();
        *pInputVolFault = 0;
        *pTempOutRangeFault = 0;
        *pChargTimeOutFault = 0;
              if(vinvalue == 1)
               {
            *pInputVolFault = 1;
           }
              if(tsvalue == 1)
               {
            *pTempOutRangeFault = 1;
           }
     }
}

/*************************************************************************************************
  * @brief    : check cic watchdog
  * @return   : none
*************************************************************************************************/
uint8_t app_CheckWatchdogFault(void)
{
    uint8_t retVal = 0;
   if(app_get_current_chrg_drv() == BQ25898E)
    {
      if (Dev_BQ25898X_GetWatchdogFault())
        {
          retVal = 1;
        }
     }
      else if(app_get_current_chrg_drv() == MP2731)
     {
       if (Dev_MP2731_GetWatchdogFault())
        {
           retVal = 1;
        }
      }
        return retVal;
}

/*************************************************************************************************
  * @brief    : check cic output voltage error
  * @return   : none
*************************************************************************************************/
uint8_t app_CheckOutputVoltage(void)
{
    uint8_t retVal = 0;
    if(app_get_current_chrg_drv() == BQ25898E)
     {
      if (Dev_BQ25898X_GetOutputVoltageFault())
        {
          retVal = 1;
        }
     }
     else if(app_get_current_chrg_drv() == MP2731)
      {
       if (Dev_MP2731_GetOutputVoltageFault())
        {
          retVal = 1;
        }
      }
     return retVal;
}

/*************************************************************************************************
  * @brief    : check cic ntc error
  * @return   : none
*************************************************************************************************/
uint8_t app_CheckNTCFault(void)
{
      uint8_t retVal = 0;
   if(app_get_current_chrg_drv() == BQ25898E)
      {
          NTC_FAULT_E ntcFault =  Dev_BQ25898X_GetNtcFault();
       if (NTC_FLT_COLD == ntcFault)
          {
            retVal = 1;
          }
       else if (NTC_FLT_HOT == ntcFault)
          {
            retVal = 2;
          }
       }
    else if(app_get_current_chrg_drv() == MP2731)
     {
       MP2731_NTC_FAULT_E ntcFault =  Dev_MP2731_GetNtcFault();
        if (NTC_FLT_COLD == ntcFault)
          {
             retVal = 1;
          }
         else if (NTC_FLT_HOT == ntcFault)
          {
             retVal = 2;
          }
      }
    return retVal;
}

/*************************************************************************************************
  * @brief    : check cic adc and mcu adc difference error
  * @return   : none
*************************************************************************************************/
void app_CheckVoltageSelfCheck(uint8_t *pVoltageSelfCheckFault, uint8_t *pCurrentSelfCheckFault)
{
    uint16_t u16Current = 0;
    uint16_t u16BatVolt = 0;
    uint16_t i_sense = 0;
    uint16_t bat_v = 0;
    if ((NULL == pVoltageSelfCheckFault) || (NULL == pCurrentSelfCheckFault))
    {
        return;
    }
    *pVoltageSelfCheckFault = 0;
    *pCurrentSelfCheckFault = 0;
    i_sense = (uint16_t)fabs(dev_get_adc_result()->i_sense * 1000);
    bat_v = (uint16_t)(dev_get_adc_result()->vbat * 1000);
      if(app_get_current_chrg_drv() == BQ25898E)
      {
         Dev_BQ25898X_GetAdc(&u16Current,NULL,&u16BatVolt,NULL,NULL);
      }
      else if(app_get_current_chrg_drv() == MP2731)
      {
         Dev_MP2731_GetAdc(&u16Current,NULL,&u16BatVolt,NULL,NULL);
      }

    if (abs(u16Current - i_sense) > 500)
      {
         *pCurrentSelfCheckFault = 1;
      }
    if (abs(u16BatVolt - bat_v) > 500)
    {
        *pVoltageSelfCheckFault = 1;
    }
}

/*************************************************************************************************
  * @brief    : config cic register
  * @return   : none
*************************************************************************************************/
void app_ConfigChargIC(void)
{
   if(app_get_current_chrg_drv() == BQ25898E)
    {
       if (0 != Dev_BQ25898X_Init())
      {
        post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
      }
    }
       else if(app_get_current_chrg_drv() == MP2731)
      {
         if (0 != Dev_MP2731_Init())
           {
              post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
           }
      }
}

/*************************************************************************************************
  * @brief    : reset cic wdg timer
  * @return   : none
*************************************************************************************************/
void app_RstChargeICWdgTimer(void)
{
    if(app_get_current_chrg_drv() == BQ25898E)
       {
         if (Dev_BQ25898X_RstWdgTimer() != 0)
           {
             post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
           }
       }
     else if(app_get_current_chrg_drv() == MP2731)
        {
          if (Dev_MP2731_RstWdgTimer() != 0)
            {
              post_msg_to_manager_with_arg(op_error_occur, flt_de_cic_config_error);
            }
        }
}

/*************************************************************************************************
  * @brief    : check cic register value periodly in charge mode
  * @return   : none
*************************************************************************************************/
void app_CheckChgICRegData(void)
{
    if(app_get_state() != STATE_CHARGE)
    {
        return ;
    }

    if(app_get_current_chrg_drv() == BQ25898E)
    {
        Dev_BQ25898X_CheckRegData();
    }
    else if(app_get_current_chrg_drv() == MP2731)
    {
        Dev_MP2731_CheckRegData();
    }
    else
    {
        LOGD("unknown IC");
    }
}

void app_GetTSPCT(uint8_t *pVal)
{
    if(app_get_current_chrg_drv() == BQ25898E)
    {
       Dev_BQ25898X_GetNtcTemp(pVal);
    }
    else if(app_get_current_chrg_drv() == MP2731)
      {
        Dev_MP2731_GetNtcTemp(pVal);
      }
}

/*************************************************************************************************
  * @brief    : get ntc temperature
  * @return   : temperature
*************************************************************************************************/
float app_GetNTCTemp(void)
{
//    uint8_t TSPCTVal = 0;
//    float ohm = 0;
//    float RT1 = 9.31;
//    float RT2 = 620;
//    float TS = 0;
//    app_GetTSPCT(&TSPCTVal);

    /*TSPCTval to ohm formula*/
//    TS = TSPCTVal*0.00465 + 0.21;
//    if(TS!=1){
//        ohm =  TS*RT1*RT2/(1-TS)/(RT2-(TS*RT1/(1-TS)));
//    }
 //   return dev_convert_ntc_ohm(ntc_map, sizeof(ntc_map)/sizeof(cic_ntc_map_t), ohm);
      return 0;
}
/*************************************************************************************************
  * @brief    : check gauge data flash
  * @return   : null
*************************************************************************************************/
void app_CheckGaugeData(void)
{
    uint8_t step;

    if(cc.charge_state != chg_s)
    {
        return ;
    }

    step =get_eol_step();
    Dev_BQ27Z561_CheckFlashData(step);
}


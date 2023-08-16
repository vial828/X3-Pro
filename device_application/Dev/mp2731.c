

#include "HWI_Hal.h"
#include "app_charge.h"
#include "dev_temperature.h"
#include "mp2731.h"
#include "i2c.h"
#include "log.h"
uint8_t e_adcEnabled = 0;
#define MP2731_REG_CNT 0x19
#define REG_CONFIG_MAX_NUM 12
#define MP2731_VREG_NUM   7
#define MP2731_ICHG_NUM   3
#define MP2731_IPRECHG_NUM   2

//#define  NTC_RESISTANCE_MAX_NUM     80
//static uint8_t MP2731REGCacheData[MP2731_REG_CNT];
//static uint8_t MP2731REGCacheFlag[MP2731_REG_CNT]={0};

REG_VALUE_INFO_T CheckValueInfo[VALUE_ITEM_NUM] =
{
  {VREG_REG, VREG_MASK, VREG_SHIFT, VREG_SU, VREG_OFFSET},
  {ITERM_REG, ITERM_MASK, ITERM_SHIFT, ITERM_SU, ITERM_OFFSET},
  {IPRECHG_REG, IPRECHG_MASK, IPRECHG_SHIFT, IPRECHG_SU, IPRECHG_OFFSET},
  {ICHG_REG, ICHG_MASK, ICHG_SHIFT, ICHG_SU, ICHG_OFFSET},
  {SYS_MIN_REG, SYS_MIN_MASK, SYS_MIN_SHIFT, SYS_MIN_SU, SYS_MIN_OFFSET},
  {IINLIM_REG, IINLIM_MASK, IINLIM_SHIFT, IINLIM_SU, IINLIM_OFFSET},
  {VINDPM_REG, VINDPM_MASK, VINDPM_SHIFT, VINDPM_SU, VINDPM_OFFSET},
  {0},
  {BATV_ADC_REG, BATV_ADC_MASK, BATV_ADC_SHIFT, BATV_ADC_SU, BATV_ADC_OFFSET},
  {SYSV_ADC_REG, SYSV_ADC_MASK, SYSV_ADC_SHIFT, SYSV_ADC_SU, SYSV_ADC_OFFSET},
  {TS_ADC_REG, TS_ADC_MASK, TS_ADC_SHIFT, TS_ADC_SU, TS_ADC_OFFSET},
  {VBUSV_ADC_REG, VBUSV_ADC_MASK, VBUSV_ADC_SHIFT, VBUSV_ADC_SU, VBUSV_ADC_OFFSET},
  {ICHGR_ADC_REG, ICHGR_ADC_MASK, ICHGR_ADC_SHIFT, ICHGR_ADC_SU, ICHGR_ADC_OFFSET},
  {EN_TERM_REG,   EN_TERM_MASK,   EN_TERM_SHIFT,   EN_TERM_EN,   EN_TERM_OFFSET},
  {EN_TIMER_REG,  EN_TIMER_MASK,  EN_TIMER_SHIFT,  EN_TIMER_EN,  EN_TIMER_OFFSET}
};

uint16_t l_u16ConfigValue[ITEM_NEED_SET] =
{
  CHG_VOLT_LIM,           //charge voltage limit      ori 4144 ,now 4140 or 4150
  TERM_CURR_LIM,          //Termination current limit
  PRECHG_CURR_LIM,        //precharge current limit
  FAST_CHARGE_CURR_LIM,   //fast charge current limit
  SYS_MIN_VOLT_LIM,       //min sys voltage
  INPUT_CURR_LIM          //input current limit
//  4800,
//  2000
};

const static cic_ntc_map_t MP2731_NTC_RESISTANCE_ARRAY[] = {
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

//const uint16_t MP2731_NTC_RESISTANCE_ARRAY[NTC_RESISTANCE_MAX_NUM] =
//{
//    42510,40600,38790,37070,35440,33890,32420,31020,29690,28420,           /* -10ï½?1 */
//    27220,26080,24990,23950,22960,22020,21120,20270,19450,18670,           /*  0ï½?9 */
//    17930,17210,16530,15890,15270,14670,14110,13570,13050,12550,           /*  10ï½?9 */
//    12080,11630,11190,10780,10380,10000,9630,9280,8950,8620,               /*  20ï½?9 */
//    8310,8020,7730,7460,7200,6950,6710,6480,6250,6040,                     /*  30ï½?9 */
//    5830,5640,5450,5260,5090,4920,4750,4600,4450,4300,                     /*  40ï½?9 */
//    4160,4030,3900,3770,3650,3540,3420,3320,3210,3110,                     /*  50ï½?9 */
//    3010,2920,2830,2750,2670,2590,2510,2440,2360,2290,                     /*  60ï½?9 */
//};
char* l_rEnumName[VALUE_ITEM_NUM] =
{
  "VREG",         //charge voltage limit
  "ITERM",       //Termination current limit
  "IPRECHG",     //precharge current limit
  "ICHG",       //fast charge current limit
  "SYS_MIN",     //min sys voltage
  "IINLIM",      //input current limit
  "ITEM_NEED_SET",
  "VINDPM",

  "BATV_ADC",
  "SYSV_ADC",
  "TS_ADC",
  "VBUSV_ADC",
  "ICHGR_ADC",
  "EN_TERM",
  "EN_TIMER",
};

/*reg options*/
uint8_t Dev_MP2731_ReadRegBits(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t* data)
{
      uint8_t ret = 0;
    uint8_t u8ReadTemp = 0;

    ret = hwi_I2C_Mem_Read(1,MP2731_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,0xfff);
    if (ret)
        return I2C_READ_ERROR;

    u8ReadTemp &= mask;
    u8ReadTemp >>= shift;

    *data = u8ReadTemp;
    return ret;
}

static uint8_t Dev_MP2731_UpdateRegBits(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t data)
{
    uint8_t ret = 0;
    uint8_t tmp = 0;

    ret = hwi_I2C_Mem_Read(1,MP2731_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&tmp,1,0xfff);

    if (ret)
    {
        return I2C_WRITE_ERROR;
    }

    tmp &= ~mask;
    data <<= shift;
    tmp |= data & mask;

    ret = hwi_I2C_Mem_Write(1,MP2731_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&tmp,1,0xfff);
    return ret;
}

uint8_t Check_MP2731_RegBits (uint8_t reg, uint8_t mask, uint8_t shift, uint8_t data)
{
    uint8_t readData = 0;
    uint8_t retVal = 0;
    Dev_MP2731_ReadRegBits(reg, mask, shift, &readData);
    if (readData != data)
    {
        retVal = 1;
    }
    return retVal;
}

uint8_t MP2731_getReg(uint8_t reg, uint8_t *dest)
{
    uint8_t ret;
    ret = MP2731_READBLOCK(reg, dest, 1);
    return ret;
}

uint8_t Dev_MP2731_getDeviceID(uint8_t *chip_ID)
{
    uint8_t ret = 0;
    ret = MP2731_getReg(MP2731_REG_17 , chip_ID);
    return ret;
}

/*set cc cv limits*/
uint8_t GetValue(VALUE_ITEM_E valitem, uint16_t* value)
{
  uint8_t ret=0;
  uint8_t u8ReadTemp=0;

  ret = Dev_MP2731_ReadRegBits(CheckValueInfo[valitem].reg, CheckValueInfo[valitem].mask, CheckValueInfo[valitem].shift, &u8ReadTemp);
  if(ret)
     return ret;
  *value = u8ReadTemp * CheckValueInfo[valitem].unit + CheckValueInfo[valitem].offset;
  return ret;
}

uint8_t SetValue(VALUE_ITEM_E valitem)
{
  uint8_t ret=0;
  uint8_t u8Val;

  if(l_u16ConfigValue[valitem] < CheckValueInfo[valitem].offset)
  {
    u8Val = 0x00;
  }
  else
  {
    u8Val = (l_u16ConfigValue[valitem]-CheckValueInfo[valitem].offset)/CheckValueInfo[valitem].unit;
  }
    //Set JEITA Requirement
  Dev_MP2731_UpdateRegBits(MP2731_REG_16 , JEITA_ISET_MASK, JEITA_ISET_SHIFT, JEITA_ISET_0);
  Dev_MP2731_UpdateRegBits(MP2731_REG_16 , JEITA_VSET_MASK, JEITA_VSET_SHIFT, JEITA_VSET_0);
  Dev_MP2731_UpdateRegBits(MP2731_REG_16 , VTH_HOT_MASK, VTH_HOT_SHIFT, VTH_HOT_0);
  Dev_MP2731_UpdateRegBits(MP2731_REG_16 , VTH_COOL_MASK, VTH_COOL_SHIFT, VTH_COOL_1);
  Dev_MP2731_UpdateRegBits(MP2731_REG_16 , VTH_COLD_MASK, VTH_COLD_SHIFT, VTH_COLD_0);
  Dev_MP2731_UpdateRegBits(MP2731_REG_16 , VTH_WARM_MASK, VTH_WARM_SHIFT, VTH_WARM_3);
  Dev_MP2731_UpdateRegBits(EN_TIMER_REG , EN_TIMER_MASK, EN_TIMER_SHIFT, EN_TIMER_EN);

  ret = Dev_MP2731_UpdateRegBits(CheckValueInfo[valitem].reg, CheckValueInfo[valitem].mask, CheckValueInfo[valitem].shift, u8Val);

  return ret;
}
uint8_t CheckValue(VALUE_ITEM_E valitem)
{
  uint8_t ret=0;
  uint16_t u16ValRead = 0;

  ret = GetValue(valitem, &u16ValRead);
  //LOGI("get config value after set: %s is : %d	\r\n", l_rEnumName[valitem] , u16ValRead);
  if(ret)
    return ret;

  if((l_u16ConfigValue[valitem] - u16ValRead < 0) || (l_u16ConfigValue[valitem] - u16ValRead) >= CheckValueInfo[valitem].unit)  //may not exactlly equal
  {
    LOGD("config check error : %s ,set :%d, read: %d \r\n", l_rEnumName[valitem] , l_u16ConfigValue[valitem], u16ValRead);
    return 1;
  }
  return ret;
}

/*******************************************************************************
* @brief
* @param
* @param
* @retval
*      0: no error
*      other: error
*******************************************************************************/
uint8_t Dev_MP2731_DisableWDG(void)
{
    uint8_t ret = 0;
    uint8_t dis_wdg = 0;

    dis_wdg = WATCHDOG_DIS;
    ret = Dev_MP2731_UpdateRegBits(MP2731_REG_08, WATCHDOG_MASK, WATCHDOG_SHIFT, dis_wdg);
    return ret;
}

uint8_t Dev_MP2731_EnableCharge(uint8_t enable)
{
    uint8_t ret = 0;
    uint8_t val = 0;
    if (enable)
    {
        val = CHG_CONFIG_EN;
    }
    else
    {
        val = CHG_CONFIG_DIS;
    }

    ret = Dev_MP2731_UpdateRegBits(MP2731_REG_04, CHG_CONFIG_MASK, CHG_CONFIG_SHIFT, val);

    return ret;
}

uint8_t Dev_MP2731_Set_SYS_MIN(uint32_t vol)
{
    uint8_t ret = 0;

    ret = Dev_MP2731_UpdateRegBits(MP2731_REG_04, SYS_MIN_MASK, SYS_MIN_SHIFT,
     (vol - SYS_MIN_OFFSET)/SYS_MIN_SU);

    return ret;
}

uint8_t Dev_MP2731_EnableADC(uint8_t enable)
{
    uint8_t ret = 0;
    uint8_t val = 0;

    if (enable)
    {
        val = CONV_START_EN;
         Dev_MP2731_UpdateRegBits(MP2731_REG_03, CONV_RATE_MASK, CONV_RATE_SHIFT, CONV_RATE_CON);
    }
    else
    {
        val = CONV_START_DIS;
    }
    ret = Dev_MP2731_UpdateRegBits(MP2731_REG_03, CONV_START_MASK, CONV_START_SHIFT, val);

    if (!ret)
    {
        e_adcEnabled = enable;
    }
    return ret;
}

/*******************************************************************************
* @brief
* @param
* @param
* @retval
*      0: no error
*      other: error
*******************************************************************************/
uint8_t Dev_MP2731_ChrgCurrent(uint16_t u16Current)
{
    uint8_t u8Icharg = (u16Current - ICHG_OFFSET) / ICHG_SU;
    uint8_t ret = 0;
    ret = Dev_MP2731_UpdateRegBits(MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, u8Icharg);
    if (0 == ret)
    {
        if (1 == Check_MP2731_RegBits (MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, u8Icharg))
        {
            return 0xFF;
        }
    }
    return ret;
}

uint8_t Dev_MP2731_ChrgVoltLimit(uint16_t u16Volt)
{
    uint8_t u8Icharg = (u16Volt - VREG_OFFSET) / VREG_SU;
    uint8_t ret = 0;
    ret = Dev_MP2731_UpdateRegBits(MP2731_REG_07, VREG_MASK, VREG_SHIFT, u8Icharg);
    if (0 == ret)
    {
        if (1 == Check_MP2731_RegBits (MP2731_REG_07, VREG_MASK, VREG_SHIFT, u8Icharg))
        {
            return 0xFF;
        }
    }
    return ret;
}
/*******************************************************************************
* @brief
* @param
* @param
* @retval
*      0: no error
*      other: error
*******************************************************************************/
uint8_t Dev_MP2731_GetStatus(uint8_t *vinStatus, uint8_t *chgStatus, uint8_t *ntcFloatStatus, uint8_t *thermStatus,uint8_t *vsysStatus)
{
    uint8_t ret = 0;
    uint8_t u8ReadTemp = 0;

    ret = hwi_I2C_Mem_Read(1,MP2731_ADDR,MP2731_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,0xfff);
    if (ret)
        return I2C_READ_ERROR;
    if (vinStatus)
    {
        *vinStatus = (u8ReadTemp & VIN_STAT_MASK) >> VIN_STAT_SHIFT;
    }
    if (chgStatus)
    {
        *chgStatus = (u8ReadTemp & CHRG_STAT_MASK) >> CHRG_STAT_SHIFT;
    }
    if (ntcFloatStatus)
    {
        *ntcFloatStatus = (u8ReadTemp & NTC_FLOAT_STAT_MASK) >> NTC_FLOAT_STAT_SHIFT;
    }
    if (thermStatus)
    {
        *thermStatus = (u8ReadTemp & THERM_STAT_MASK) >> THERM_STAT_SHIFT;
    }
    if (vsysStatus)
    {
        *vsysStatus = (u8ReadTemp & VSYS_STAT_MASK) >> VSYS_STAT_SHIFT;
    }
    return ret;
}

uint8_t Dev_MP2731_GetWatchdogFault(void)
{
    uint8_t u8RegVal = 0;
    hwi_I2C_Mem_Read(1,MP2731_ADDR,MP2731_REG_0D,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,0xfff);
    u8RegVal = (u8RegVal & WATCHDOG_FAULT_MASK) >> WATCHDOG_FAULT_SHIFT;
    return u8RegVal;
}

//uint8_t Dev_MP2731_GetInputFault(void)
//{
//    uint16_t vbusmv;
//    uint8_t value;
//    uint8_t u8RegVal = 0;
//     Dev_MP2731_ReadRegBits(MP2731_REG_11, VBUSV_ADC_MASK, VBUSV_ADC_SHIFT, &value);
//     vbusmv = value* VBUSV_ADC_SU + VBUSV_ADC_OFFSET;
//     Dev_MP2731_boost_back_work_around();
//       if(vbusmv > 4000)
//        {
//          Dev_MP2731_ReadRegBits(MP2731_REG_0D, CHRG_INPUT_MASK, CHRG_INPUT_SHIFT, &u8RegVal);
//        }
//    return u8RegVal;
//}


uint8_t Dev_MP2731_ThermalShutdown(void)
{
    uint8_t ret;
    uint8_t u8RegVal = 0;
    ret = Dev_MP2731_ReadRegBits(MP2731_REG_0D, CHRG_THERM_MASK, CHRG_THERM_SHIFT, &u8RegVal);
    if(ret)
    {
       return ret;
    }
    return u8RegVal;
}

uint8_t Dev_MP2731_GetOutputVoltageFault(void)
{
    uint8_t u8RegVal = 0;
    hwi_I2C_Mem_Read(1,MP2731_ADDR,MP2731_REG_0D,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,0xfff);
    u8RegVal = (u8RegVal & BAT_FAULT_MASK) >> BAT_FAULT_SHIFT;
    return u8RegVal;
}

MP2731_NTC_FAULT_E Dev_MP2731_GetNtcFault(void)
{
    uint8_t u8RegVal = 0;
    hwi_I2C_Mem_Read(1,MP2731_ADDR,MP2731_REG_0D,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,0xfff);
    u8RegVal = (u8RegVal & NTC_FAULT_MASK) >> NTC_FAULT_SHIFT;
    return (MP2731_NTC_FAULT_E)u8RegVal;
}
/*******************************************************************************
* @brief
* @param
* @param ts: ts value (21%-80%)*100
* @retval
*      0: no error
*      other: error
*******************************************************************************/
uint8_t Dev_MP2731_GetAdc(uint16_t *curr, uint16_t *vBusVolt, uint16_t *selfBatVolt, uint16_t  *vsysVolt, uint16_t *tsValue)
{
    uint8_t value = 0;
    uint8_t ret = 0;

    if (!e_adcEnabled)
    {
        return ADC_NOT_ENABLED;
    }

    if (curr)
    {
        ret = Dev_MP2731_ReadRegBits(MP2731_REG_12, ICHGR_ADC_MASK, ICHGR_ADC_SHIFT, &value);
        //ret = Dev_MP2731_ReadRegBits(MP2731_REG_13, IIN_MASK, IIN_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *curr = (uint16_t)(value* ICHGR_ADC_SU + ICHGR_ADC_OFFSET);
       // *curr = (uint16_t)(value* IIN_ADC_SU + IIN_ADC_OFFSET);
    }
    if (vBusVolt)
    {
        ret = Dev_MP2731_ReadRegBits(MP2731_REG_11, VBUSV_ADC_MASK, VBUSV_ADC_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *vBusVolt = value* VBUSV_ADC_SU + VBUSV_ADC_OFFSET;
    }
    if (selfBatVolt)
    {
        ret = Dev_MP2731_ReadRegBits(MP2731_REG_0E, BATV_ADC_MASK, BATV_ADC_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *selfBatVolt = value* BATV_ADC_SU + BATV_ADC_OFFSET;
    }
    if (vsysVolt)
    {
        ret = Dev_MP2731_ReadRegBits(MP2731_REG_0F, SYSV_ADC_MASK, SYSV_ADC_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *vsysVolt = value* SYSV_ADC_SU + SYSV_ADC_OFFSET;
    }
    if (tsValue)
    {
        ret = Dev_MP2731_ReadRegBits(MP2731_REG_10, TS_ADC_MASK, TS_ADC_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *tsValue = (value* TS_ADC_SU) / 10 + TS_ADC_OFFSET * 100;
    }
    return ret;
}

/*******************************************************************************
* @brief
* @param
* @param
* @retval
*      0: no error
*      other: error
*******************************************************************************/
uint8_t Dev_MP2731_GetVindpm(uint16_t *vinDpm)
{
    uint8_t ret = 0;
    uint8_t u8ReadTemp = 0;

    ret = hwi_I2C_Mem_Read(1,MP2731_ADDR,MP2731_REG_01,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,0xfff);
    if (ret)
        return I2C_READ_ERROR;

    if (vinDpm)
    {
        *vinDpm = ((u8ReadTemp & VINDPM_MASK) >> VINDPM_SHIFT)* VINDPM_SU + VINDPM_OFFSET;
    }
    return ret;
}

uint8_t Dev_MP2731_GetNtcTemp(uint8_t *temp)
{
    float fTs = 0;
    uint8_t value = 0;
    uint8_t ret = 0;
    float RT1 = 11;
    float RT2 = 1.4;
    float ntcRes = 0;

    ret = Dev_MP2731_ReadRegBits(MP2731_REG_10, TS_ADC_MASK, TS_ADC_SHIFT, &value);
    if (ret)
    {
        return ret;
    }
    fTs = value* 0.00392;
    ntcRes = RT1* fTs / (1 - fTs) - RT2;
//    LOGD("value=%d,tsValue=%f,ntcRes=%f\n",value,tsValue,ntcRes);
    *temp = dev_convert_ntc_ohm(MP2731_NTC_RESISTANCE_ARRAY, sizeof(MP2731_NTC_RESISTANCE_ARRAY)/sizeof(cic_ntc_map_t), ntcRes);
    return ret;
}

/*******************************************************************************
* @brief
* @param
* @param Implement the following workaround to eliminate boost back
* @retval
*      0: no error
*      other: error
*******************************************************************************/
uint8_t Dev_MP2731_boost_back_work_around(void)
{
    uint8_t ret = 0;
    uint8_t value;
//    uint8_t u8ValueOld;
     ret = Dev_MP2731_ReadRegBits(VINPPM_STAT_REG, VINPPM_STAT_MASK, VINPPM_STAT_SHIFT, &value);
      if(value == 1)
       {
        //Dev_MP2731_UpdateRegBits(MP2731_REG_03, CONV_RATE_MASK, CONV_RATE_SHIFT, CONV_RATE_CON);
        ret = Dev_MP2731_ReadRegBits(IIN_REG, IIN_MASK, IIN_SHIFT, &value);
          if(value < 10)  //133mA
          {
//           value=1;
//          ret = Dev_MP2731_ReadRegBits(USB_DET_EN_REG, USB_DET_EN_MASK, USB_DET_EN_SHIFT, &u8ValueOld);
//          u8ValueOld &= (~USB_DET_EN_MASK);
//          value <<= USB_DET_EN_SHIFT;
//          u8ValueOld |= (value & USB_DET_EN_MASK);
          Dev_MP2731_UpdateRegBits(USB_DET_EN_REG, USB_DET_EN_MASK, USB_DET_EN_SHIFT, 1);
          Dev_MP2731_UpdateRegBits(USB_DET_EN_REG, USB_DET_EN_MASK, USB_DET_EN_SHIFT, 0);
          Dev_MP2731_UpdateRegBits(USB_DET_EN_REG, USB_DET_EN_MASK, USB_DET_EN_SHIFT, 1);
          //Dev_MP2731_UpdateRegBits(MP2731_REG_03, CONV_RATE_MASK, CONV_RATE_SHIFT, CONV_RATE_ONCE);
            ret = Dev_MP2731_ReadRegBits(MP2731_REG_0C, VIN_STAT_MASK, VIN_STAT_SHIFT, &value);
            LOGD("met boost back vin 0x%x ret=%d\n",value,ret);
          }
       }
        return ret;
}

/*******************************************************************************
* @brief
* @param
* @retval None
*******************************************************************************/
uint8_t Dev_MP2731_Init(void)
{
    uint8_t ret = 0;
    uint8_t u8i;

     Dev_MP2731_DisableWDG();
      Dev_MP2731_EnableADC(1);

    Dev_MP2731_UpdateRegBits(EN_HIZ_REG, EN_HIZ_MASK, EN_HIZ_SHIFT, EN_HIZ_DIS);
    Dev_MP2731_UpdateRegBits(VINDPM_REG, VINDPM_MASK, VINDPM_SHIFT, 8);
    Dev_MP2731_UpdateRegBits(NTC_TYPE_REG, NTC_TYPE_MASK, NTC_TYPE_SHIFT, 1);
    Dev_MP2731_UpdateRegBits(BATFET_DIS_REG, BATFET_DIS_MASK, BATFET_DIS_SHIFT, BATFET_DIS_DIS);
//    Dev_MP2731_UpdateRegBits(MP2731_REG_0A, SW_FREQ_MASK, SW_FREQ_SHIFT, SW_FREQ_1);
      for(u8i = 0; u8i < ITEM_NEED_SET; u8i++)
   {
    SetValue((VALUE_ITEM_E)u8i);
    ret = CheckValue((VALUE_ITEM_E)u8i);
    if(ret)
      break;
   }
    Dev_MP2731_UpdateRegBits(CHG_TIMER_REG , CHG_TIMER_MASK, CHG_TIMER_SHIFT , CHG_TIMER_5h);
    Dev_MP2731_UpdateRegBits(TREG_REG , TREG_MASK, TREG_SHIFT , TREG_100);
       LOGD("MP2731 init success!\n");
         return ret;
}

uint8_t Dev_MP2731_RstWdgTimer(void)
{
    uint8_t ret = Dev_MP2731_UpdateRegBits(MP2731_REG_08, WDT_RESET_MASK, WDT_RESET_SHIFT, WDT_RESET);
    return ret;
}


void Dev_MP2731_UpdateCheckinfo_ICHG(uint16_t u16Current)
{
    CheckValueInfo[MP2731_ICHG_NUM].unit = (u16Current - ICHG_OFFSET) / ICHG_SU;
}
void Dev_MP2731_UpdateCheckinfo_IPRECHG(uint16_t u16Current)
{
    CheckValueInfo[MP2731_IPRECHG_NUM].unit = (u16Current - IPRECHG_OFFSET) / IPRECHG_SU;
}
void Dev_MP2731_UpdateCheckinfo_VREG(uint16_t u16Volt)
{
    CheckValueInfo[MP2731_VREG_NUM].unit = (u16Volt - VREG_OFFSET) / VREG_SU;
}

void Dev_MP2731_CheckRegData(void)
{
    uint8_t i = 0;
    uint8_t data = 0;
    uint8_t error = 0;
    for (i = 0; i < (sizeof(CheckValueInfo) / sizeof(REG_VALUE_INFO_T)); i++)
    {
        Dev_MP2731_ReadRegBits(CheckValueInfo[i].reg, CheckValueInfo[i].mask, CheckValueInfo[i].shift, &data);
        if (data != CheckValueInfo[i].unit)
        {
            error = 1;
           // LOGE("Dev_MP2731_CheckRegData error:reg%x,mask=0x%x, shift=%d,data=0x%x, value=0x%x\r\n", CheckValueInfo[i].reg, CheckValueInfo[i].mask, CheckValueInfo[i].shift, data, CheckValueInfo[i].value);
           // Dev_MP2731_UpdateRegBits(CheckValueInfo[i].reg, CheckValueInfo[i].mask, CheckValueInfo[i].shift, CheckValueInfo[i].unit);
        }
    }
    if (1 == error)
    {
        for (i = 0; i < (sizeof(CheckValueInfo) / sizeof(REG_VALUE_INFO_T)); i++)
        {
            Dev_MP2731_ReadRegBits(CheckValueInfo[i].reg, CheckValueInfo[i].mask, CheckValueInfo[i].shift, &data);
            //LOGE("Dev_MP2731_CheckRegData error:reg%x,mask=0x%x, shift=%d,data=0x%x, value=0x%x\r\n", CheckValueInfo[i].reg, CheckValueInfo[i].mask, CheckValueInfo[i].shift, data, CheckValueInfo[i].value);
        }
    }
}

uint8_t Dev_MP2731_Set_LowCurrent_Charge()
{
    //Reg00 as 50mA (0x01)
    uint8_t ret = Dev_MP2731_UpdateRegBits(MP2731_REG_00, IINLIM_MASK, IINLIM_SHIFT, (150 - IINLIM_OFFSET) / IINLIM_SU);
    //Reg04 as 64mA (0x01)
    ret+=Dev_MP2731_UpdateRegBits(MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, (360 - ICHG_OFFSET) / ICHG_SU);
    //Reg05 as 64mA (0x00)
    ret+=Dev_MP2731_UpdateRegBits(MP2731_REG_06, ITERM_MASK, ITERM_SHIFT, (TERM_CURR_LIM - ITERM_OFFSET) / ITERM_SU);
    return ret;
}
uint8_t Dev_MP2731_Set_DefaultCurrent_Charge()
{
    uint8_t ret = Dev_MP2731_UpdateRegBits(MP2731_REG_00, IINLIM_MASK, IINLIM_SHIFT, (INPUT_CURR_LIM - IINLIM_OFFSET) / IINLIM_SU);
    ret+=Dev_MP2731_UpdateRegBits(MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, (FAST_CHARGE_CURR_LIM - ICHG_OFFSET) / ICHG_SU);
    ret+=Dev_MP2731_UpdateRegBits(MP2731_REG_06, ITERM_MASK, ITERM_SHIFT, (TERM_CURR_LIM - ITERM_OFFSET) / ITERM_SU);
    return ret;
}

void Dev_MP2731_SetNormalChargeCurrent(){
    uint8_t ret=Dev_MP2731_UpdateRegBits(MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, (FAST_CHARGE_CURR_LIM - ICHG_OFFSET) / ICHG_SU);
    //ret+=Dev_MP2731_UpdateRegBits(MP2731_REG_06, IPRECHG_MASK, IPRECHG_SHIFT, (PRECHG_CURR_LIM - IPRECHG_OFFSET) / IPRECHG_SU);
    if(ret==0){
        Dev_MP2731_UpdateCheckinfo_ICHG(FAST_CHARGE_CURR_LIM);
        //Dev_MP2731_UpdateCheckinfo_IPRECHG(PRECHG_CURR_LIM);
    }
}

void Dev_MP2731_SetABNormalChargeCurrent(){
    uint8_t ret=Dev_MP2731_UpdateRegBits(MP2731_REG_05, ICHG_MASK, ICHG_SHIFT, (ADJ_FAST_CHARGE_CURR_LIM - ICHG_OFFSET) / ICHG_SU);
    //ret+=Dev_MP2731_UpdateRegBits(MP2731_REG_06, IPRECHG_MASK, IPRECHG_SHIFT, (ADJ_PRECHG_CURR_LIM - IPRECHG_OFFSET) / IPRECHG_SU);
    if(ret==0){
        Dev_MP2731_UpdateCheckinfo_ICHG(ADJ_FAST_CHARGE_CURR_LIM);
        //Dev_MP2731_UpdateCheckinfo_IPRECHG(ADJ_PRECHG_CURR_LIM);
    }
}

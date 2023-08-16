#include "HWI_Hal.h"
//#include "Stm32g0xx_hal_i2c.h"
#include "app_charge.h"
#include "dev_bq25898x.h"
#include "i2c.h"
#include "log.h"
#include "dev_temperature.h"
#include "dev_gauge_bq27z561r2.h"
uint8_t g_adcEnabled = 0;

int16_t default_charge_current;
#define REG_CONFIG_MAX_NUM 20

/*charge IC init sequence*/
const REG_CONFIG_INFO_T s_intRegInfo[REG_CONFIG_MAX_NUM] =
{
    //IPRECHG, REG05 bit4-bit7 Precharge Current Limit
    {BQ25898X_REG_05, BQ25898X_IPRECHG_MASK, BQ25898X_IPRECHG_SHIFT, (PRECHARGE_CURR_LIMIT - BQ25898X_IPRECHG_BASE) / BQ25898X_IPRECHG_LSB},
    //ICHG, REG04 bit0-bit6 Fast Charge Current Limit 
    {BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (FAST_CHARGE_CURR_LIMIT - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB},
    //ITERM, REG05 bit0-bit3 Termination Current Limit
    {BQ25898X_REG_05, BQ25898X_ITERM_MASK, BQ25898X_ITERM_SHIFT, (TERM_CURR_LIMIT - BQ25898X_ITERM_BASE) / BQ25898X_ITERM_LSB},
    //VREG, REG06 bit2-bit7 Charge Voltage Limit
    {BQ25898X_REG_06, BQ25898X_VREG_MASK, BQ25898X_VREG_SHIFT, (CHARGE_VOLT_LIMIT - BQ25898X_VREG_BASE) / BQ25898X_VREG_LSB},
    //VRECHG, REG06 bit0 Battery Recharge Threshold Offset
    {BQ25898X_REG_06, BQ25898X_VRECHG_MASK, BQ25898X_VRECHG_SHIFT, RECHARGE_THRESHOLD_OFFSET},
    //BATLOWV,REG06 bit1 Battery Precharge to Fast Charge Threshold
    {BQ25898X_REG_06, BQ25898X_BATLOWV_MASK, BQ25898X_BATLOWV_SHIFT, PRECHARGE_TO_FAST_CHARGE_THRESHOLD},
    //CHG_TIMER, REG07 bit1-bit2 Fast Charge Timer Setting
    {BQ25898X_REG_07, BQ25898X_CHG_TIMER_MASK, BQ25898X_CHG_TIMER_SHIFT, FAST_CHARGE_TIMER},
    //JEITA_ISET, REG07 bit0 JEITA Low Temperature Current Setting
    {BQ25898X_REG_07, BQ25898X_JEITA_ISET_MASK, BQ25898X_JEITA_ISET_SHIFT, LOW_TEMP_CURR_LIMIT},
    //TREG, REG08 bit0-bit1 Thermal Regulation Threshold
    {BQ25898X_REG_08, BQ25898X_TREG_MASK, BQ25898X_TREG_SHIFT, THERMAL_REGULATION_THRESHOLD},
    //{BQ25898X_REG_08, BQ25898X_RES_COMP_MASK, BQ25898X_RES_COMP_SHIFT, (BAT_COMP_LIMIT - BQ25898X_RES_COMP_BASE)/BQ25898X_RES_COMP_LSB},
    {BQ25898X_REG_08, BQ25898X_VCLAMP_MASK, BQ25898X_VCLAMP_SHIFT, (VCLAMP_LIMIT - BQ25898X_VCLAMP_COMP_BASE)/BQ25898X_VCLAMP_COMP_LSB},
    //BOOST_LIM, REG0A bit0-bit1 Boost Mode Current Limit
	{BQ25898X_REG_0A, BQ25898X_BOOST_LIM_MASK, BQ25898X_BOOST_LIM_SHIFT, BOOST_MODE_CURR_LIMIT},
	//ICO_EN, REG02 bit4 Input Current Optimizer (ICO) Enable
	{BQ25898X_REG_02, BQ25898X_ICOEN_MASK, BQ25898X_ICOEN_SHIFT, BQ25898X_ICO_ENABLE},
	//HVDCP_EN, REG02 bit3 High Voltage DCP Enable (bq25898D only)
    {BQ25898X_REG_02, BQ25898X_HVDCPEN_MASK, BQ25898X_HVDCPEN_SHIFT, BQ25898X_HVDCP_DISABLE},
    //MAXC_EN, REG02 bit2 MaxCharge Adapter Enable (bq25898D only)
    {BQ25898X_REG_02, BQ25898X_MAXCEN_MASK, BQ25898X_MAXCEN_SHIFT, BQ25898X_MAXC_DISABLE},
    //AUTO_DPDM_EN, REG02 bit0 Automatic D+/D- Detection Enable
    {BQ25898X_REG_02, BQ25898X_AUTO_DPDM_EN_MASK, BQ25898X_AUTO_DPDM_EN_SHIFT, BQ25898X_AUTO_DPDM_DISABLE},
    //REG00 bit0-bit5 Input Current Limit bq25898D
    {BQ25898X_REG_00, BQ25898X_IINLIM_MASK, BQ25898X_IINLIM_SHIFT, (INPUT_CURR_LIMIT - BQ25898X_IINLIM_BASE) / BQ25898X_IINLIM_LSB},
    {BQ25898X_REG_00, BQ25898X_ENILIM_MASK, BQ25898X_ENILIM_SHIFT, BQ25898X_ENILIM_DISABLE},
	//REG0A bit7 VINDPM Threshold Setting Method
    //{BQ25898X_REG_0D, BQ25898X_FORCE_VINDPM_MASK, BQ25898X_FORCE_VINDPM_SHIFT, BQ25898X_FORCE_VINDPM_ENABLE},
    //REG0A bit0-bit6 Absolute VINDPM Threshold
    //{BQ25898X_REG_0D, BQ25898X_VINDPM_MASK, BQ25898X_VINDPM_SHIFT, (INPUT_VOLT_LIMIT - BQ25898X_VINDPM_BASE) / BQ25898X_VINDPM_LSB},
    //JEITA_VSET,REG09 bit4 JEITA High Temperature Voltage Setting  
    {BQ25898X_REG_09, BQ25898X_JEITA_VSET_MASK, BQ25898X_JEITA_VSET_SHIFT, BQ25898X_JEITA_VSET_VREG},
    //SYS_MIN, REG03 bit1-bit3 Minimum System Voltage Limit
    {BQ25898X_REG_03, BQ25898X_SYS_MINV_MASK, BQ25898X_SYS_MINV_SHIFT, (SYS_MIN_VOLT_LIMIT - BQ25898X_SYS_MINV_BASE)/BQ25898X_SYS_MINV_LSB},
    {BQ25898X_REG_03, BQ25898X_MIN_VBAT_SEL_MASK, BQ25898X_MIN_VBAT_SEL_SHIFT, BQ25898X_MIN_VBAT_2900},
};

#define BQ25898X_VREG_NUM        10
#define BQ25898X_ICHG_NUM        7
#define BQ25898X_PREICHG_NUM     8
#define BQ25898X_TERM_ICHG_NUM   9

REG_CONFIG_INFO_T CheckRegInfo[] =
{
    //REG00 bit0-bit5 Input Current Limit bq25898D
    {BQ25898X_REG_00, BQ25898X_ENILIM_MASK, BQ25898X_ENILIM_SHIFT, BQ25898X_ENILIM_DISABLE},

    {BQ25898X_REG_02, BQ25898X_ICOEN_MASK, BQ25898X_ICOEN_SHIFT, BQ25898X_ICO_ENABLE},
    //HVDCP_EN, REG02 bit3 High Voltage DCP Enable (bq25898D only)
    {BQ25898X_REG_02, BQ25898X_HVDCPEN_MASK, BQ25898X_HVDCPEN_SHIFT, BQ25898X_HVDCP_DISABLE},
    //MAXC_EN, REG02 bit2 MaxCharge Adapter Enable (bq25898D only)
    {BQ25898X_REG_02, BQ25898X_MAXCEN_MASK, BQ25898X_MAXCEN_SHIFT, BQ25898X_MAXC_DISABLE},
    //AUTO_DPDM_EN, REG02 bit0 Automatic D+/D- Detection Enable
    {BQ25898X_REG_02, BQ25898X_AUTO_DPDM_EN_MASK, BQ25898X_AUTO_DPDM_EN_SHIFT, BQ25898X_AUTO_DPDM_DISABLE},

    {BQ25898X_REG_03, BQ25898X_SYS_MINV_MASK, BQ25898X_SYS_MINV_SHIFT, (SYS_MIN_VOLT_LIMIT - BQ25898X_SYS_MINV_BASE)/BQ25898X_SYS_MINV_LSB},
    {BQ25898X_REG_03, BQ25898X_MIN_VBAT_SEL_MASK, BQ25898X_MIN_VBAT_SEL_SHIFT, BQ25898X_MIN_VBAT_2900},

    //ICHG, REG04 bit0-bit6 Fast Charge Current Limit 
    {BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (FAST_CHARGE_CURR_LIMIT - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB},
    
    //IPRECHG, REG05 bit4-bit7 Precharge Current Limit
    {BQ25898X_REG_05, BQ25898X_IPRECHG_MASK, BQ25898X_IPRECHG_SHIFT, (PRECHARGE_CURR_LIMIT - BQ25898X_IPRECHG_BASE) / BQ25898X_IPRECHG_LSB},
    
    //ITERM, REG05 bit0-bit3 Termination Current Limit
    {BQ25898X_REG_05, BQ25898X_ITERM_MASK, BQ25898X_ITERM_SHIFT, (TERM_CURR_LIMIT - BQ25898X_ITERM_BASE) / BQ25898X_ITERM_LSB},
    //VREG, REG06 bit2-bit7 Charge Voltage Limit
    {BQ25898X_REG_06, BQ25898X_VREG_MASK, BQ25898X_VREG_SHIFT, (CHARGE_VOLT_LIMIT - BQ25898X_VREG_BASE) / BQ25898X_VREG_LSB},
    //VRECHG, REG06 bit0 Battery Recharge Threshold Offset
    {BQ25898X_REG_06, BQ25898X_VRECHG_MASK, BQ25898X_VRECHG_SHIFT, RECHARGE_THRESHOLD_OFFSET},
    //BATLOWV,REG06 bit1 Battery Precharge to Fast Charge Threshold
    {BQ25898X_REG_06, BQ25898X_BATLOWV_MASK, BQ25898X_BATLOWV_SHIFT, PRECHARGE_TO_FAST_CHARGE_THRESHOLD},
    //CHG_TIMER, REG07 bit1-bit2 Fast Charge Timer Setting
    {BQ25898X_REG_07, BQ25898X_CHG_TIMER_MASK, BQ25898X_CHG_TIMER_SHIFT, FAST_CHARGE_TIMER},
    //JEITA_ISET, REG07 bit0 JEITA Low Temperature Current Setting
    {BQ25898X_REG_07, BQ25898X_JEITA_ISET_MASK, BQ25898X_JEITA_ISET_SHIFT, LOW_TEMP_CURR_LIMIT},
    //TREG, REG08 bit0-bit1 Thermal Regulation Threshold
    {BQ25898X_REG_08, BQ25898X_TREG_MASK, BQ25898X_TREG_SHIFT, THERMAL_REGULATION_THRESHOLD},
    
    //BOOST_LIM, REG0A bit0-bit1 Boost Mode Current Limit
    {BQ25898X_REG_0A, BQ25898X_BOOST_LIM_MASK, BQ25898X_BOOST_LIM_SHIFT, BOOST_MODE_CURR_LIMIT},
    {BQ25898X_REG_07, BQ25898X_EN_TERM_MASK,   BQ25898X_EN_TERM_SHIFT,   BQ25898X_TERM_ENABLE},
    {BQ25898X_REG_07, BQ25898X_EN_TIMER_MASK,  BQ25898X_EN_TIMER_SHIFT, BQ25898X_EN_TIMER_ENABLE},
};



#define TEMP_OFFSET    (50)
//#define  NTC_RESISTANCE_MAX_NUM     80

const static cic_ntc_map_t BQ25898X_NTC_RESISTANCE_ARRAY[] = {
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

//const uint16_t NTC_RESISTANCE_ARRAY[NTC_RESISTANCE_MAX_NUM] =
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

uint8_t eol_step = 0;
void set_eol_step(uint8_t step)
{
    eol_step=step;
}
uint8_t get_eol_step(void)
{
   return eol_step;
}

void session_count_check(uint8_t temp_status)
{
    data_change_frequent_t* pDataChangeFreq = get_data_change_frequent_from_ram();
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    int32_t all_heat_time = 0;
    //int32_t all_heat_time = pDataChangeFreq->lifeCycleData[total_heat_time];
    int32_t step1_nums_time = pfrt->step1_session_nums*290;
    int32_t step2_nums_time = pfrt->step2_session_nums*290;
    int32_t step3_nums_time = pfrt->step3_session_nums*290;
    int32_t step4_nums_time = pfrt->step4_session_nums*290;

    if(all_heat_time >= step4_nums_time)
    {
        set_eol_step(STEP_4);
        default_charge_current = pfrt->step4_chg_curr;
        LOGD("session > %d,set step4 curr volt",pfrt->step4_session_nums);
        if(temp_status == temp_Section_normal)
        {
            Dev_BQ25898X_ChrgCurrent(pfrt->step4_chg_curr);
            Dev_BQ25898x_UpdateCheckinfo_ICHG(pfrt->step4_chg_curr);
        }
        Dev_BQ25898X_ChrgVoltLimit(pfrt->step4_chg_volt);
        Dev_BQ25898x_UpdateCheckinfo_VREG(pfrt->step4_chg_volt);
    }
    else if(all_heat_time >= step3_nums_time)
    {
        set_eol_step(STEP_3);
        default_charge_current = pfrt->step3_chg_curr;
        LOGD("session > %d,set step3 curr volt",pfrt->step3_session_nums);
        if(temp_status == temp_Section_normal)
        {
            Dev_BQ25898X_ChrgCurrent(pfrt->step3_chg_curr);
            Dev_BQ25898x_UpdateCheckinfo_ICHG(pfrt->step3_chg_curr);
        }
        Dev_BQ25898X_ChrgVoltLimit(pfrt->step3_chg_volt);
        Dev_BQ25898x_UpdateCheckinfo_VREG(pfrt->step3_chg_volt);
    }
    else if(all_heat_time >= step2_nums_time)
    {
        set_eol_step(STEP_2);
        default_charge_current = pfrt->step2_chg_curr;
        LOGD("session > %d,set step2 curr volt",pfrt->step2_session_nums);
        if(temp_status == temp_Section_normal)
        {
            Dev_BQ25898X_ChrgCurrent(pfrt->step2_chg_curr);
            Dev_BQ25898x_UpdateCheckinfo_ICHG(pfrt->step2_chg_curr);
        }
        Dev_BQ25898X_ChrgVoltLimit(pfrt->step2_chg_volt);
        Dev_BQ25898x_UpdateCheckinfo_VREG(pfrt->step2_chg_volt);
    }
    else if(all_heat_time >= step1_nums_time)
    {
        set_eol_step(STEP_1);
        default_charge_current = pfrt->step1_chg_curr;
        LOGD("session > %d,set step1 curr volt",pfrt->step1_session_nums);
        if(temp_status == temp_Section_normal)
        {
            Dev_BQ25898X_ChrgCurrent(pfrt->step1_chg_curr);
            Dev_BQ25898x_UpdateCheckinfo_ICHG(pfrt->step1_chg_curr);
        }
        Dev_BQ25898X_ChrgVoltLimit(pfrt->step1_chg_volt);
        Dev_BQ25898x_UpdateCheckinfo_VREG(pfrt->step1_chg_volt);
    }
    else
    {
        LOGD("session set step0");
        set_eol_step(STEP_0);
        default_charge_current = FAST_CHARGE_CURR_LIMIT;
    }
}

/*******************************************************************************
* @brief
* @param
* @param
* @retval
*      0: no error
*      other: error
*******************************************************************************/
static uint8_t Dev_BQ25898X_UpdateRegBits(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t data)
{
    uint8_t ret = 0;
    uint8_t tmp = 0;
	//uint8_t chnnl = 0;
    //ret = HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&tmp,1,100);
	  ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&tmp,1,0xfff);

    if (ret)
    {
        return I2C_WRITE_ERROR;
    }

    tmp &= ~mask;
    data <<= shift;
    tmp |= data & mask;

//    ret = HAL_I2C_Mem_Write(&hi2c1,BQ25898X_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&tmp,1,100);
		ret = hwi_I2C_Mem_Write(GPIO_CHANNEL,BQ25898X_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&tmp,1,0xfff);
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
uint8_t Dev_BQ25898X_ReadRegBits(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t* data)
{

    uint8_t ret = 0;
    uint8_t u8ReadTemp = 0;

    //ret = HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,100);
		ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,0xfff);
    if (ret)
        return I2C_READ_ERROR;

    u8ReadTemp &= mask;
    u8ReadTemp >>= shift;

    *data = u8ReadTemp;
    return ret;
}

uint8_t BQ25898x_getReg(uint8_t reg, uint8_t *dest)
{
    uint8_t ret;
    ret = BQ25898x_READBLOCK(reg, dest, 1);
    return ret;
}

uint8_t Dev_BQ25898x_getDeviceID(uint8_t *chip_ID)
{
    uint8_t ret = 0;
    ret = BQ25898x_getReg(BQ25898X_REG_14 , chip_ID);
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
static uint8_t Dev_BQ25898X_DisableWDG(void)
{
    uint8_t ret = 0;
    uint8_t dis_wdg = 0;

    dis_wdg = BQ25898X_WDT_DISABLE;
    ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_07, BQ25898X_WDT_MASK, BQ25898X_WDT_SHIFT, dis_wdg);
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
uint8_t Dev_BQ25898X_EnableCharge(uint8_t enable)
{
    uint8_t ret = 0;
    uint8_t val = 0;
    if (enable)
    {
        val = BQ25898X_CHG_ENABLE;
    }
    else
    {
        val = BQ25898X_CHG_DISABLE;
    }

    ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_03, BQ25898X_CHG_CONFIG_MASK, BQ25898X_CHG_CONFIG_SHIFT, val);

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
uint8_t Dev_BQ25898X_Set_SYS_MIN(uint32_t vol)
{
    uint8_t ret = 0;

    ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_03, BQ25898X_SYS_MINV_MASK, BQ25898X_SYS_MINV_SHIFT, 
	     (vol - BQ25898X_SYS_MINV_BASE)/BQ25898X_SYS_MINV_LSB);

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
uint8_t Dev_BQ25898X_EnableADC(uint8_t enable)
{
    uint8_t ret = 0;
    uint8_t val = 0;
    if (enable)
    {
        val = BQ25898X_CONV_RATE_CONTINUE;
    }
    else
    {
        val = BQ25898X_CONV_RATE_ONE_SHOT;
    }
    ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_02, BQ25898X_CONV_RATE_MASK, BQ25898X_CONV_RATE_SHIFT, val);
    if (!ret)
    {
        g_adcEnabled = enable;
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
uint8_t Dev_BQ25898X_EnableOtg(uint8_t enable)
{
    uint8_t ret = 0;
    uint8_t val = 0;
    if (enable)
    {
        val = BQ25898X_OTG_ENABLE;
    }
    else
    {
        val = BQ25898X_OTG_DISABLE;
    }
    ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_03, BQ25898X_OTG_CONFIG_MASK, BQ25898X_OTG_CONFIG_SHIFT, val);
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
uint8_t Dev_BQ25898X_ChrgCurrent(uint16_t u16Current)
{
    uint8_t u8Icharg = (u16Current - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB;
    uint8_t ret = 0;
    ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, u8Icharg);
    if (0 == ret)
    {
        if (1 == Check_BQ25898X_RegBits (BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, u8Icharg))
        {
            return 0xFF;
        }
    }
    return ret;
}

uint8_t Dev_BQ25898X_OtgCurrent(uint8_t u8Current)
{
    uint8_t ret = 0;
    uint8_t u8RegLast = 0;
    ret = Dev_BQ25898X_ReadRegBits(BQ25898X_REG_0A, BQ25898X_BOOST_LIM_MASK, BQ25898X_BOOST_LIM_SHIFT, &u8RegLast);
    if (ret)
    {
        return ret;
    }
    if (u8RegLast != u8Current)
    {
        ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_0A, BQ25898X_BOOST_LIM_MASK, BQ25898X_BOOST_LIM_SHIFT, u8Current);
    }
    return ret;
}

uint8_t Dev_BQ25898X_ChrgVoltLimit(uint16_t u16Volt)
{
    uint8_t u8Icharg = (u16Volt - BQ25898X_VREG_BASE) / BQ25898X_VREG_LSB;
    uint8_t ret = 0;
    ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_06, BQ25898X_VREG_MASK, BQ25898X_VREG_SHIFT, u8Icharg);
    if (0 == ret)
    {
        if (1 == Check_BQ25898X_RegBits (BQ25898X_REG_06, BQ25898X_VREG_MASK, BQ25898X_VREG_SHIFT, u8Icharg))
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
uint8_t Dev_BQ25898X_GetStatus(uint8_t *vbusStatus, uint8_t *chgStatus, uint8_t *pgStatus, uint8_t *vsysStatus)
{
    uint8_t ret = 0;
    uint8_t u8ReadTemp = 0;

	  //ret = HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,BQ25898X_REG_0B,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,100);
    ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,BQ25898X_REG_0B,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,0xfff);
    if (ret)
        return I2C_READ_ERROR;
    if (vbusStatus)
    {
        *vbusStatus = (u8ReadTemp & BQ25898X_VBUS_STAT_MASK) >> BQ25898X_VBUS_STAT_SHIFT;
    }
    if (chgStatus)
    {
        *chgStatus = (u8ReadTemp & BQ25898X_CHG_STAT_MASK) >> BQ25898X_CHG_STAT_SHIFT;
    }
    if (pgStatus)
    {
        *pgStatus = (u8ReadTemp & BQ25898X_PG_STAT_MASK) >> BQ25898X_PG_STAT_SHIFT;
    }
    if (vsysStatus)
    {
        *vsysStatus = (u8ReadTemp & BQ25898X_VSYS_STAT_MASK) >> BQ25898X_VSYS_STAT_SHIFT;
    }
    return ret;
}

/*******************************************************************************
* @brief
* @param
* @param 
* @retval
*******************************************************************************/
uint8_t Dev_BQ25898X_GetWatchdogFault(void)
{
    uint8_t u8RegVal = 0;
    //HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,100);
		hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,0xfff);
    u8RegVal = (u8RegVal & BQ25898X_FAULT_WATCHDOG_MASK) >> BQ25898X_FAULT_WATCHDOG_SHIFT;
    return u8RegVal;
}

/*******************************************************************************
* @brief
* @param
* @param 
* @retval
*******************************************************************************/
CHRG_FALUT_E Dev_BQ25898X_GetChrgFault(void)
{
    uint8_t u8RegVal = 0;
    //HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,100);
	  hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,0xfff);
    u8RegVal = (u8RegVal & BQ25898X_FAULT_CHRG_MASK) >> BQ25898X_FAULT_CHRG_SHIFT;
    return (CHRG_FALUT_E)u8RegVal;
}

/*******************************************************************************
* @brief
* @param
* @param 
* @retval
*******************************************************************************/
uint8_t Dev_BQ25898X_GetOutputVoltageFault(void)
{
    uint8_t u8RegVal = 0;
    uint8_t ret = 0;
    //HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,100);
    ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,0xfff);
    if(ret)
    {
        return I2C_READ_ERROR;
    }
    u8RegVal = (u8RegVal & BQ25898X_FAULT_OUTPUTVOLTAGE_MASK) >> BQ25898X_FAULT_OUTPUTVOLTAGE_SHIFT;
    return u8RegVal;
}


/*******************************************************************************
* @brief
* @param
* @param 
* @retval
*******************************************************************************/
NTC_FAULT_E Dev_BQ25898X_GetNtcFault(void)
{
    uint8_t u8RegVal = 0;
    //HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,100);
	  hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,BQ25898X_REG_0C,I2C_MEMADD_SIZE_8BIT,&u8RegVal,1,0xfff);
    u8RegVal = (u8RegVal & BQ25898X_FAULT_NTC_MASK) >> BQ25898X_FAULT_NTC_SHIFT;
    return (NTC_FAULT_E)u8RegVal;
}

/*******************************************************************************
* @brief
* @param
* @param ts: ts value (21%-80%)*100
* @retval
*      0: no error
*      other: error
*******************************************************************************/
uint8_t Dev_BQ25898X_GetAdc(uint16_t *curr, uint16_t *vBusVolt, uint16_t *selfBatVolt, uint16_t  *vsysVolt, uint16_t *tsValue)
{
    uint8_t value = 0;
    uint8_t ret = 0;

    if (!g_adcEnabled)
    {
        return ADC_NOT_ENABLED;
    }

    if (curr)
    {
        ret = Dev_BQ25898X_ReadRegBits(BQ25898X_REG_12, BQ25898X_ICHGR_MASK, BQ25898X_ICHGR_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *curr = (uint16_t)(value* BQ25898X_ICHGR_LSB + BQ25898X_ICHGR_BASE);
    }
    if (vBusVolt)
    {
        ret = Dev_BQ25898X_ReadRegBits(BQ25898X_REG_11, BQ25898X_VBUSV_MASK, BQ25898X_VBUSV_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *vBusVolt = value* BQ25898X_VBUSV_LSB + BQ25898X_VBUSV_BASE;
    }
    if (selfBatVolt)
    {
        ret = Dev_BQ25898X_ReadRegBits(BQ25898X_REG_0E, BQ25898X_BATV_MASK, BQ25898X_BATV_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *selfBatVolt = value* BQ25898X_BATV_LSB + BQ25898X_BATV_BASE;
    }
    if (vsysVolt)
    {
        ret = Dev_BQ25898X_ReadRegBits(BQ25898X_REG_0F, BQ25898X_SYSV_MASK, BQ25898X_SYSV_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *vsysVolt = value* BQ25898X_SYSV_LSB + BQ25898X_SYSV_BASE;
    }
    if (tsValue)
    {
        ret = Dev_BQ25898X_ReadRegBits(BQ25898X_REG_10, BQ25898X_TSPCT_MASK, BQ25898X_TSPCT_SHIFT, &value);
        if (ret)
        {
            return ret;
        }
        *tsValue = (value* BQ25898X_TSPCT_LSB) / 10 + BQ25898X_TSPCT_BASE * 100;
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
uint8_t Dev_BQ25898X_GetVindpm(uint8_t *forceVIndpm, uint16_t *vinDpm)
{
    uint8_t ret = 0;
    uint8_t u8ReadTemp = 0;

    //ret = HAL_I2C_Mem_Read(&hi2c1,BQ25898X_ADDR,BQ25898X_REG_0D,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,100);
	  ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ25898X_ADDR,BQ25898X_REG_0D,I2C_MEMADD_SIZE_8BIT,&u8ReadTemp,1,0xfff);
    if (ret)
        return I2C_READ_ERROR;
    if (forceVIndpm)
    {
        *forceVIndpm = (u8ReadTemp & BQ25898X_FORCE_VINDPM_MASK) >> BQ25898X_FORCE_VINDPM_SHIFT;
    }
    if (vinDpm)
    {
        *vinDpm = ((u8ReadTemp & BQ25898X_VINDPM_MASK) >> BQ25898X_VINDPM_SHIFT)* BQ25898X_VINDPM_LSB + BQ25898X_VINDPM_BASE;
    }
    return ret;
}


/*******************************************************************************
* @brief
* @param
* @retval
*******************************************************************************/
uint8_t Dev_BQ25898X_GetNtcTemp(uint8_t *temp)
{
    float fTs = 0;
    uint8_t value = 0;
    uint8_t ret = 0;
    float RT1 = 9.31;
    float RT2 = 620;
//    uint16_t low = 0;
//    uint16_t high = NTC_RESISTANCE_MAX_NUM - 1;
//    uint16_t mid = 0;
    float ntcRes = 0;

    ret = Dev_BQ25898X_ReadRegBits(BQ25898X_REG_10, BQ25898X_TSPCT_MASK, BQ25898X_TSPCT_SHIFT, &value);
    if (ret)
    {
        return ret;
    }
      fTs = value*0.00465 + 0.21;
//    ntcRes = 1.0/(1.0/(RT1_VALUE*fTs) - 1.0/RT1_VALUE - 1.0/RT2_VALUE);
      ntcRes =  fTs*RT1*RT2/(1-fTs)/(RT2-(fTs*RT1/(1-fTs)));
//    LOGD("value=%d,fTs=%f,ntcRes=%f\n",value,fTs,ntcRes);
    *temp = dev_convert_ntc_ohm(BQ25898X_NTC_RESISTANCE_ARRAY, sizeof(BQ25898X_NTC_RESISTANCE_ARRAY)/sizeof(cic_ntc_map_t), ntcRes);
//    LOGD("##temp=%d\n",*temp);

//    if (ntcRes > NTC_RESISTANCE_ARRAY[low])
//    {
//        *temp = -11;
//    }
//    else if (ntcRes <= NTC_RESISTANCE_ARRAY[high])
//    {
//        *temp = 70;
//    }
//    else
//    {
//        mid = (low + high) / 2;
//        while (low != mid)
//        {
//            if (ntcRes > NTC_RESISTANCE_ARRAY[mid])
//            {
//                high = mid;
//            }
//            else
//            {
//                low = mid;
//            }
//            mid = (high + low) / 2;
//        }
//        *temp = (int8_t)mid - 10;
//    }
    return ret;
}



/*******************************************************************************
* @brief
* @param
* @retval None
*******************************************************************************/
uint8_t Dev_BQ25898X_Init(void)
{
    uint8_t ret = 0;
    uint8_t index = 0;

    
    Dev_BQ25898X_DisableWDG();
 
  
    for (index = 0; index < REG_CONFIG_MAX_NUM; index++)
    {
        ret = Dev_BQ25898X_UpdateRegBits(s_intRegInfo[index].reg, s_intRegInfo[index].mask, s_intRegInfo[index].shift, s_intRegInfo[index].value);
        if ((ret != 0) || (1 == Check_BQ25898X_RegBits (s_intRegInfo[index].reg, s_intRegInfo[index].mask, s_intRegInfo[index].shift, s_intRegInfo[index].value)))
        {
            return 0xFF;
        }
    }
    Dev_BQ25898X_EnableADC(1);
    //Dev_BQ25898X_UpdateVDPM_OS();
    //Dev_BQ25898X_EnableCharge(1);
    //Dev_BQ25898X_Set_Bat_Compensation();
    //Dev_BQ25898X_Set_Input_Volt(INPUT_VOLT_MIN_VAL);
    return ret;
}

uint8_t Check_BQ25898X_RegBits (uint8_t reg, uint8_t mask, uint8_t shift, uint8_t data)
{
    uint8_t readData = 0;
    uint8_t retVal = 0;
    Dev_BQ25898X_ReadRegBits(reg, mask, shift, &readData);
    if (readData != data)
    {
        retVal = 1;
    }
    return retVal;
}
uint8_t Dev_BQ25898X_RstWdgTimer(void)
{
    uint8_t ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_03, BQ25898X_WDT_RESET_MASK, BQ25898X_WDT_RESET_SHIFT, BQ25898X_WDT_RESET);
    return ret;
}

void Dev_BQ25898x_UpdateCheckinfo_ICHG(uint16_t u16Current)
{
    CheckRegInfo[BQ25898X_ICHG_NUM].value = (u16Current - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB;
}

void Dev_BQ25898x_UpdateCheckinfo_PREICHG(uint16_t u16Current)
{
    CheckRegInfo[BQ25898X_PREICHG_NUM].value = (u16Current - BQ25898X_IPRECHG_BASE) / BQ25898X_IPRECHG_LSB;
}

void Dev_BQ25898x_UpdateCheckinfo_TERMICHG(uint16_t u16Current)
{
    CheckRegInfo[BQ25898X_TERM_ICHG_NUM].value = (u16Current - BQ25898X_ITERM_BASE) / BQ25898X_ITERM_LSB;
}

void Dev_BQ25898x_UpdateCheckinfo_VREG(uint16_t u16Volt)
{
    CheckRegInfo[BQ25898X_VREG_NUM].value = (u16Volt - BQ25898X_VREG_BASE) / BQ25898X_VREG_LSB;
}

void Dev_BQ25898X_CheckRegData(void)
{
    uint8_t i = 0;
    uint8_t data = 0;
    uint8_t error = 0;
    for (i = 0; i < (sizeof(CheckRegInfo) / sizeof(REG_CONFIG_INFO_T)); i++)
    {
        Dev_BQ25898X_ReadRegBits(CheckRegInfo[i].reg, CheckRegInfo[i].mask, CheckRegInfo[i].shift, &data);
        if (data != CheckRegInfo[i].value)
        {
            error = 1;
            LOGE("BQ25898X_CheckREG error:reg%x, mask=0x%x, shift=%d, data=0x%x, value=0x%x\r\n", CheckRegInfo[i].reg, CheckRegInfo[i].mask, CheckRegInfo[i].shift, data, CheckRegInfo[i].value);
            Dev_BQ25898X_UpdateRegBits(CheckRegInfo[i].reg, CheckRegInfo[i].mask, CheckRegInfo[i].shift, CheckRegInfo[i].value);
        }
    }
    if (1 == error)
    {
        for (i = 0; i < (sizeof(CheckRegInfo) / sizeof(REG_CONFIG_INFO_T)); i++)
        {
            Dev_BQ25898X_ReadRegBits(CheckRegInfo[i].reg, CheckRegInfo[i].mask, CheckRegInfo[i].shift, &data);
            LOGE("BQ25898X_CheckREG error:reg%x, mask=0x%x, shift=%d, data=0x%x, value=0x%x\r\n", CheckRegInfo[i].reg, CheckRegInfo[i].mask, CheckRegInfo[i].shift, data, CheckRegInfo[i].value);
        }
    }
}

uint8_t Dev_BQ25898X_Set_LowCurrent_Charge()
{
    //Reg00 as 50mA (0x01)
    uint8_t ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_00, BQ25898X_IINLIM_MASK, BQ25898X_IINLIM_SHIFT, (50 - BQ25898X_IINLIM_BASE) / BQ25898X_IINLIM_LSB);
    //Reg04 as 64mA (0x01)
    ret+=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (64 - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB);
    //Reg05 as 64mA (0x00)
    ret+=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_05, BQ25898X_ITERM_MASK, BQ25898X_ITERM_SHIFT, (64 - BQ25898X_ITERM_BASE) / BQ25898X_ITERM_LSB);
    return ret;
}
uint8_t Dev_BQ25898X_Set_DefaultCurrent_Charge()
{
    uint8_t ret = Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_00, BQ25898X_IINLIM_MASK, BQ25898X_IINLIM_SHIFT, (INPUT_CURR_LIMIT - BQ25898X_IINLIM_BASE) / BQ25898X_IINLIM_LSB);
    ret+=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (default_charge_current - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB);
    ret+=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_05, BQ25898X_ITERM_MASK, BQ25898X_ITERM_SHIFT, (TERM_CURR_LIMIT - BQ25898X_ITERM_BASE) / BQ25898X_ITERM_LSB);
    return ret;
}

void Dev_BQ25898X_SetNormalChargeCurrent(){
    uint8_t ret=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (default_charge_current - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB);
    //ret+=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_05, BQ25898X_IPRECHG_MASK, BQ25898X_IPRECHG_SHIFT, (PRECHARGE_CURR_LIMIT - BQ25898X_IPRECHG_BASE) / BQ25898X_IPRECHG_LSB);
    if(ret==0){
        Dev_BQ25898x_UpdateCheckinfo_ICHG(default_charge_current);
        //Dev_BQ25898x_UpdateCheckinfo_PREICHG(PRECHARGE_CURR_LIMIT);
    }
}

void Dev_BQ25898X_SetABNormalChargeCurrent(){
    uint8_t ret=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (ADJ_FAST_CHARGE_CURR_LIMIT - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB);
    //ret+=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_05, BQ25898X_IPRECHG_MASK, BQ25898X_IPRECHG_SHIFT, (ADJ_PRECHARGE_CURR_LIMIT - BQ25898X_IPRECHG_BASE) / BQ25898X_IPRECHG_LSB);
    if(ret==0){
        Dev_BQ25898x_UpdateCheckinfo_ICHG(ADJ_FAST_CHARGE_CURR_LIMIT);
        //Dev_BQ25898x_UpdateCheckinfo_PREICHG(ADJ_PRECHARGE_CURR_LIMIT);
    }
}

void Dev_BQ25898X_SetMinChargeCurrent(void){
    uint8_t ret=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (MIN_FAST_CHARGE_CURR_LIMIT - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB);
    if(ret==0){
        Dev_BQ25898x_UpdateCheckinfo_ICHG(MIN_FAST_CHARGE_CURR_LIMIT);
    }
}
void Dev_BQ25898X_SetTemporaryChargeCurrent(void){
    uint8_t ret=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_04, BQ25898X_ICHG_MASK, BQ25898X_ICHG_SHIFT, (TEMP_FAST_CHARGE_CURR_LIMIT - BQ25898X_ICHG_BASE) / BQ25898X_ICHG_LSB);
    if(ret==0){
        Dev_BQ25898x_UpdateCheckinfo_ICHG(TEMP_FAST_CHARGE_CURR_LIMIT);
    }
}

void Dev_BQ25898X_BATFET_DIS(void)
{
    Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_09, BQ25898X_BATFET_DLY_MASK, BQ25898X_BATFET_DLY_SHIFT,BQ25898X_BATFET_DLY_OFF);
    Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_09, BQ25898X_PUMPX_UP_MASK, BQ25898X_PUMPX_UP_SHIFT, BQ25898X_PUMPX_UP);
    Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_09, BQ25898X_BATFET_DIS_MASK, BQ25898X_BATFET_DIS_SHIFT,BQ25898X_BATFET_OFF);
}

void Dev_BQ25898X_UpdateVDPM_OS(void){
    uint8_t ret=Dev_BQ25898X_UpdateRegBits(BQ25898X_REG_01, BQ25898X_VDPM_OS_BIT_MASK, BQ25898X_VDPM_OS_BIT_SHIFT, VDPM_OS);
    if(ret){
        LOGD("BQ25898X_UpdateVDPM_OS fail\r\n");
    }
}

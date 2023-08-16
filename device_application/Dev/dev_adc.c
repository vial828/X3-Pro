#include "HWI_Hal.h"
#include "kernel.h"
#include "log.h"
#include "dev_adc.h"
#include "usr_cmd.h"
#include "dev_temperature.h"
#include "HWI_adc.h"
#include "self_flash.h"
#define DELAY_START_ADC_TIME 100  //ms


//static uint8_t adc_ch[] = {8,0,1,2,3,4,5,6,7,9,10};
static uint8_t adc_ch[] = {0,1,2,3,4,5,6,7,10,8};
static uint8_t adcIndex = 0;
//static uint8_t VREFINT_L;
//static uint8_t VREFINT_H;
//static uint16_t VREFINT;
static uint32_t adc_raw[ADC_CH_NUM];
static float g_calu;
//b=927*2.5/(4096*92.25891*0.002)-0.01 =3.056348
static float g_b;
static float g_ipeak = 0;
static adc_context_st adc_c;

#define AVERAGE_CNT 4
static uint16_t adc_value_array[sizeof(adc_ch)][AVERAGE_CNT] ={0};
static uint8_t record_cnt=0;

/*************************************************************************************************
 * @brief   :select adc channel
 * @parm    :channel
 * @return  :none
*************************************************************************************************/
static void AdcFirstSelect( uint8_t channel )
{
    hwi_AdcFirstSelect(channel);
}

/*************************************************************************************************
 * @brief   :read adc value (block)
 * @return  :adc value
*************************************************************************************************/
static uint16_t AdcFirstRead( void )
{
  return hwi_AdcFirstRead();
}

/*************************************************************************************************
 * @brief   :read adc value noblock
 * @parm    :value pointer
 * @return  :adc value
*************************************************************************************************/
static uint16_t AdcReadNoblock( uint32_t *value)
{
    return hwi_AdcReadNoblock(value);
}

/*************************************************************************************************
 * @brief   :calculate the average
 * @parm    :channel
 * @return  :adc value
*************************************************************************************************/
uint16_t get_average_adc(adc_ch_e channel)
{
    uint16_t adc_average_value;
    uint16_t adc_sum_value;
    
    adc_average_value=adc_sum_value=0;
    //while(data_record==1)
    {        
        for(int i=0;i<AVERAGE_CNT;i++)
        {
            adc_sum_value+=adc_value_array[channel][i];
        }
        adc_average_value=adc_sum_value/AVERAGE_CNT;
    }
    
    return adc_average_value;
}

/*************************************************************************************************
 * @brief   :get adc value from buffer 
 * @parm    :channel
 * @return  :adc value that calibrated with g_calu
*************************************************************************************************/
uint16_t dev_get_adc(adc_ch_e channel)
{
    return adc_raw[channel] * g_calu;
    //return adc_raw[adc_ch];
}

/*************************************************************************************************
 * @brief   :get adc channel voltage
 * @parm    :channel
 * @return  :voltage
*************************************************************************************************/
float dev_get_voltage(adc_ch_e channel)
{
    uint32_t adc_value = 0;
    uint32_t ver_value = 0;
    adc_value = adc_raw[channel];
    ver_value = adc_raw[vref];
    if (0 == ver_value)
    {
        return 0;
    }
    /*The actual Vdd = 3V * VERFINT_CAL / VERFINT_DATA 
     Vchannelx = Vdd / FULL_SCALE * ADC_DATAx
    so Vchannelx = (3V * VERFINT_CAL * ADC_DATAx) / (VERFINT_DATA * FULL_SCALE)*/
    return ((float)(VREFINT * 3.3f * adc_value) / (ver_value * 4095));
}

/*************************************************************************************************
 * @brief   :get HWID voltage
 * @return  :voltage
*************************************************************************************************/

#if 0
/* VBUS---32.4K--PA0--24.9K----GND*/
float get_vbus_volt(void)
{
	float v;
	
	v	= dev_get_voltage(VBUS_VOLT);
	return (v/24.9 * (24.9+32.4));
}
#endif

/*************************************************************************************************
 * @brief   :get battery voltage
 * @return  :voltage
*************************************************************************************************/
/* VBUS---20K--PA1--20K----GND*/
float dev_get_vbat_volt(void)
{
    if(dev_get_phy_val_pos() & (1<<BAT_V_E))
    {
        return (uint16_t)dev_get_phy_val(BAT_V_E)/1000.0;
    }
    else
    {
        float v;
        v = dev_get_voltage(VBAT_VOLT);
        return (2*v);
    }
}
#if 0
float get_vregn_volt(void)
{
    float v;
    uint32_t adc_value = 0;
    adc_value = dev_get_adc(VREGN);
    v = (float)((float)adc_value/4096.0*2.5*2.0);

    return v;
}
float get_vntc_volt(void)
{
    float v;
    uint32_t adc_value = 0;
    adc_value = dev_get_adc(VNTC);
    v = (float)((float)adc_value/4096.0*2.5);

    return v;
}
#endif
#if 0

/*************************************************************************************************
 * @brief   :get the ID of the battery
 * @return  :current
*************************************************************************************************/
uint16_t get_bat_ID(void)
{
    uint16_t bat_ID;
  

	/*maybe return the cli setting value*/
    if(dev_get_phy_val_pos() & (1<<BAT_ID_E))
    {
        bat_ID = (int16_t)dev_get_phy_val(BAT_ID_E);
    }
    else
    {
        bat_ID = dev_get_adc(BAT_ID);     
    }
    
    /*add the transfer module that change BAT_ID voltage to ID name*/

    return bat_ID;
    

}
#endif

/*************************************************************************************************
 * @brief   :get the current of system consumption
 * @return  :current
*************************************************************************************************/
float dev_get_i_sense(void)
{
    uint16_t u;
    static float cur;
    static uint16_t u_s = 0xFFFF;

	/*maybe return the cli setting value*/
    if(dev_get_phy_val_pos() & (1<<I_SENSE_E))
    {
        cur = ((int16_t)dev_get_phy_val(I_SENSE_E))/1000.0;
    }
    else
    {
        u = dev_get_adc(I_SENSE);
        if(u != u_s){
            u_s = u;
            // I=ADC/4096*2.5*5.3631608-3.024454 (A)
    //      cur = ((float)u - 885)/292.5;
        //cur = (float)u*2.8f/(4096.0f*102.0256772f*0.002) - g_b;//glimmer
        cur = (float)u*2.8f/(4096.0f*102.2f*0.002) - g_b;
       // cur=(float)u*2.8f/(4096.0f*101.8397677f*0.002f)- g_b;
    //      if(cur>-0.050 && cur<0.010){
    //          cur = 0.010;
    //      }
        }
    }

    return cur;
//  return ((float)u - 885)/292.5;
}

static void set_i_sense_peak(void)
{
    
    static uint32_t u32tick = 0;
    float isense = dev_get_adc_result()->i_sense;

    if (GetTick()- u32tick <= 1000)
    {
        if (isense > g_ipeak)
        {
            g_ipeak = isense;
        }
    }
    else
    {
        u32tick = GetTick();
        g_ipeak = isense;
    }
}

float dev_get_isense_peak(void)
{
    return g_ipeak;
}
/*************************************************************************************************
 * @brief   :get battery current (this is invailible now)
 * @return  :current
*************************************************************************************************/
/*float get_i_bat(void)
{
    uint16_t u;
    static float cur;
    static uint16_t u_s = 0xFFFF;

    u = dev_get_adc(I_BAT);
    if(u != u_s){
        u_s = u;
        // I=ADC/4096*2.5*5.3631608-3.024454 (A)
//      cur = ((float)u - 885)/292.5;
        cur = (float)u*2.5/(4096*92.25891*0.002) - g_b;
//      if(cur>-0.050 && cur<0.010){
//          cur = 0.010;
//      }
    }

    return cur;
//  return ((float)u - 885)/292.5;
}*/

/*************************************************************************************************
 * @brief   :get all channel adc value to buffer sequencely
 * @return  :none
*************************************************************************************************/
static void adc_first_conversion(void)
{
  uint8_t i;

    for(i=0; i< sizeof(adc_ch); i++)
    {
        AdcFirstSelect(adc_ch[i]);
        adc_raw[i] = AdcFirstRead();
    }
    if (0 == adc_raw[vref])
    {
    	g_calu = 0;
    }   
    else
    {
    	g_calu = (float)VREFINT / adc_raw[vref] * 3.3f / 2.8f;
    }
    //AdcSelect(1);
    adcIndex = 0;
    AdcFirstSelect(adc_ch[adcIndex]);
//  LOGD("adc first ok ...\r\n");
}

/*************************************************************************************************
 * @brief   :calibrated the system current
 * @return  :none
*************************************************************************************************/
static void cal_i_sense_b(void)
{
    //b=927*2.5/(4096*92.25891*0.002)-0.01 =3.056348
    uint16_t adc = dev_get_adc(I_SENSE);
    //g_b = adc*2.8/(4096*102.0256772*0.002)-0.017; //glimmer
    g_b=adc*2.8/(4096*102.2*0.002)-0.02494+0.008;
    //g_b=adc*2.8/(4096*101.8397677*0.002)-0.06715;
    LOGD("cal isense  adc=%d b=%f\r\n", adc, g_b);
}
void cal_i_sense_b_RE(void)
{
    //b=927*2.5/(4096*92.25891*0.002)-0.01 =3.056348
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    uint16_t g_b_adc = pfrt->g_b_adc;
    g_b=g_b_adc*2.8/(4096*102.2*0.002)-0.02494+0.008;
    LOGD("re_isense adc=%d b=%f", g_b_adc, g_b);
}
void record_g_b_adc(void)
{
    flash_record_t * pfrt = get_self_flash_record_from_ram();
    pfrt->g_b_adc = dev_get_adc(I_SENSE);
    update_data_flash(USR_DATA,INVALID);
}
/*************************************************************************************************
 * @brief   :adc init
 * @return  :none
*************************************************************************************************/
void dev_adc_init(void)
{
    hwi_HAL_Delay(50);
    adc_first_conversion();
    cal_i_sense_b();
}


/*************************************************************************************************
 * @brief   :adc task (put the adc value to buffer)
 * @return  :none
*************************************************************************************************/
void dev_adc_task(void)
{
    uint16_t adc_value[9];

//  EXECUTE_ONECE_AFTER_TIME(adc_first_conversion, DELAY_START_ADC_TIME);
    hwi_AdcRead(adc_value);
    for(uint8_t i = 0;i < 9;i++)
    {
        adc_raw[i] = adc_value[i];
    }

//    if(AdcReadNoblock(&adc_raw[adcIndex])){
//        if(adc_ch[adcIndex] == 10){ //channel 10 is vref
            if (0 == adc_raw[vref])
            {
                g_calu = 0;
            }   
            else
            {
                g_calu = (float)VREFINT / adc_raw[vref] * 3.3f / 2.8f;
            }
//        }
//        adcIndex++;
//        if(adcIndex==sizeof(adc_ch))
//        {
//            record_cnt++;
//        }
//        adcIndex = adcIndex % sizeof(adc_ch);
//        AdcSelect(adc_ch[adcIndex]);
//    }
    //hwi_Adc_sw_enable();
//    for(uint8_t cnt = 0;cnt < 9;cnt++)
//    {
//        adc_value_array[cnt][record_cnt]=adc_raw[cnt] * g_calu;
//    }
//    record_cnt++;
//    if(record_cnt==AVERAGE_CNT)
//    {
//        record_cnt=0;
//        //data_record=1;
//    }

}

/*************************************************************************************************
 * @brief   :get random seed by XOR-ing adc value of floating pin and current tick
 * @parm    :void
 * @return  :random seed for rand()
*************************************************************************************************/
uint32_t dev_get_random_seed(void)
{
    return hwi_trng_generate();
}

void dev_cal_adc_result(void)
{
    adc_c.bat_temp = dev_get_bat_temp();
    adc_c.usb_temp = dev_get_usb_temp();
    adc_c.coil_temp = dev_get_coil_temp();
    adc_c.cold_junc_temp=dev_get_cold_junc_temp();
    adc_c.vbat = dev_get_vbat_volt();
    adc_c.zone1_temp = dev_get_zone_temp(1);
    adc_c.zone2_temp = dev_get_zone_temp(2);
    adc_c.i_sense = dev_get_i_sense();
    set_i_sense_peak();
}

adc_context_st* dev_get_adc_result(void)
{
    return &adc_c;
}






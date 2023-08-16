#include "HWI_Hal.h"


#define DELAY_START_ADC_TIME 100  //ms


uint16_t VREFINT;

//返回此时adc转换的实际参考电压，即mcu当前的vref供电电压
void hwi_UpdateVref(void)
{
//    //VREFINT = REG32(0x0);  //待确认地址
//	
//    uint32_t timeout=0,vref_calibration_value,temp;
//	//vref_calibration_value=REG32(0X0);//待确认地址
//	vref_calibration_value=1489;//待确认地址
//	adc_software_trigger_enable(ADC_INSERTED_CHANNEL); 
//    /* wait the end of conversion flag */
//    timeout=hwi_get_tick();
//    while(!adc_flag_get(ADC_FLAG_EOIC))
//    {
//        if((hwi_get_tick()-timeout)>0x5U)
//        {
//          //hwi_adc_init();
//          break;
//        }
//    }
//    /* clear the end of conversion flag */
//  adc_flag_clear(ADC_FLAG_EOIC);
//	temp=adc_inserted_data_read(ADC_INSERTED_CHANNEL_0);
//	VREFINT=3300/(temp/vref_calibration_value);	
		
		VREFINT=1489;
	
}
/*************************************************************************************************
 * @brief   :select adc channel
 * @parm    :channel
 * @return  :none
*************************************************************************************************/
void hwi_AdcSelect( uint8_t adc_ch )
{
  #if 0
  ADC1->CHSELR = 1 << adc_ch;
  ADC1->CR |= ADC_CR_ADSTART;
  #endif
  
	/* ADC regular channel config */
	adc_regular_channel_config(0U, adc_ch, ADC_SAMPLETIME_479POINT5); 
	/* ADC software trigger enable */
	adc_software_trigger_enable(ADC_REGULAR_CHANNEL);    
}

/*************************************************************************************************
 * @brief   :read adc value (block)
 * @return  :adc value
*************************************************************************************************/
uint16_t hwi_AdcRead( void )
{
  uint16_t result;
  #if 0
  while( ~ADC1->ISR & ADC_ISR_EOC);
  result = ADC1->DR;
  #endif
  
    uint32_t timeout=0;
    /* wait the end of conversion flag */
    timeout=hwi_get_tick();
    while(!adc_flag_get(ADC_FLAG_EOC))
    {
        if((hwi_get_tick()-timeout)>0x5U)
        {
          hwi_adc_init();
          break;
        }
    }
    /* clear the end of conversion flag */
    adc_flag_clear(ADC_FLAG_EOC);
		result=adc_regular_data_read();
  
  return result;
}

uint16_t hwi_AdcReadNoblock( uint32_t *value)
{
    #if 0
  if( ~ADC1->ISR & ADC_ISR_EOC){
		return 0;
	}else{
		*value = ADC1->DR;
		return 1;
	}
    #endif
	
  if(adc_flag_get(ADC_FLAG_EOC)==RESET){
		return 0;
	}else{
		*value = adc_regular_data_read();
		return 1;
	}	
	
}


/*************************************************************************************************
 * @brief   :adc init
 * @return  :none
*************************************************************************************************/
void hwi_adc_init(void)
{
	
		hwi_UpdateVref();
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
  
    adc_deinit();
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config( ADC_REGULAR_CHANNEL, 1U);	
    /* ADC regular channel config */
    //adc_inserted_channel_config(0U, ADC_CHANNEL_10, ADC_SAMPLETIME_479POINT5);	
	
    /* ADC vbat channel enable */
    //adc_channel_9_to_11(ADC_VBAT_CHANNEL_SWITCH, ENABLE);
    /* ADC temperature and vref enable */
    adc_channel_9_to_11(ADC_TEMP_VREF_CHANNEL_SWITCH, ENABLE);		
	
    
    /* ADC external trigger config */
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
//    adc_external_trigger_config(ADC_INSERTED_CHANNEL, EXTERNAL_TRIGGER_DISABLE);	

		adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_5B,ADC_OVERSAMPLING_RATIO_MUL32);
		adc_oversample_mode_enable();
    /* enable ADC interface */
    adc_enable();
  
    hwi_delay_ms(1U);  	
}

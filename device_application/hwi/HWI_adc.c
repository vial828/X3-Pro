#include "HWI_Hal.h"


#define DELAY_START_ADC_TIME 100  //ms


uint16_t VREFINT = 0;

void hwi_UpdateVref(void)
{
    uint32_t u32Reg = 0;
    uint16_t u16Ret = 0;
    u32Reg = REG32(VREFINT_ADDR); 
    u16Ret = u32Reg>>16;
    if (u16Ret == 0 || u16Ret == 0xFFFF)
    {
        u16Ret = (uint16_t)u32Reg;
        if (u16Ret == 0 || u16Ret == 0xFFFF)
        {
            VREFINT=1489;
        }
        else
        {
            VREFINT = u16Ret;
        }
    }
    else
    {
        VREFINT = u16Ret;
    }
}

/*************************************************************************************************
 * @brief   :select adc channel
 * @parm    :channel
 * @return  :none
*************************************************************************************************/
void hwi_AdcFirstSelect( uint8_t channel )
{
  #if 0
  ADC1->CHSELR = 1 << adc_ch;
  ADC1->CR |= ADC_CR_ADSTART;
  #endif
  
	/* ADC regular channel config */
	adc_regular_channel_config(0U, channel, ADC_SAMPLETIME_479POINT5); 
	/* ADC software trigger enable */
	adc_software_trigger_enable(ADC_REGULAR_CHANNEL);    
}

/*************************************************************************************************
 * @brief   :read adc value (block)
 * @return  :adc value
*************************************************************************************************/
uint16_t hwi_AdcFirstRead( void )
{
  uint16_t result;
  #if 0
  while( ~ADC1->ISR & ADC_ISR_EOC);
  result = ADC1->DR;
  #endif
  
    uint32_t timeout=0;
    /* wait the end of conversion flag */
    timeout=hwi_SysTick_GetTick();
    while(!adc_flag_get(ADC_FLAG_EOC))
    {
        if((hwi_SysTick_GetTick()-timeout)>0x5U)
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

	adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL16);
	adc_oversample_mode_enable();
    /* enable ADC interface */
    adc_enable();
  
    hwi_delay_ms(1U);  	
}
#if 1
uint16_t adc_value[9];

/*************************************************************************************************
 * @brief   :select adc channel
 * @parm    :channel
 * @return  :none
*************************************************************************************************/
void hwi_Adc_sw_enable(void)
{
    /* enable ADC software trigger */
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
}

/*************************************************************************************************
 * @brief   :read adc value (block)
 * @return  :adc value
*************************************************************************************************/
void hwi_AdcRead( uint16_t *result )
{
   uint8_t channel;
    for(channel = 0;channel < 9;channel++)
    {
        result[channel] = adc_value[channel];
    }
}

void hwi_dma_config_init(void)
{
    /* ADC_DMA_channel configuration */
    dma_single_data_parameter_struct dma_single_data_parameter;
    rcu_periph_clock_enable(RCU_DMA1);
    /* ADC DMA_channel configuration */
    dma_deinit(DMA1, DMA_CH4);

    /* initialize DMA single data mode */
    dma_single_data_parameter.periph_addr         = (uint32_t)(&ADC_RDATA);
    dma_single_data_parameter.periph_inc          = DMA_PERIPH_INCREASE_DISABLE;
    dma_single_data_parameter.memory0_addr        = (uint32_t)(&adc_value);
    dma_single_data_parameter.memory_inc          = DMA_MEMORY_INCREASE_ENABLE;
    dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
    dma_single_data_parameter.circular_mode       = DMA_CIRCULAR_MODE_ENABLE;
    dma_single_data_parameter.direction           = DMA_PERIPH_TO_MEMORY;
    dma_single_data_parameter.number              = 9U;
    dma_single_data_parameter.priority            = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA1, DMA_CH4, &dma_single_data_parameter);
    dma_channel_subperipheral_select(DMA1, DMA_CH4, DMA_SUBPERI0);

    /* enable DMA circulation mode */
    dma_circulation_enable(DMA1, DMA_CH4);
    /* enable DMA channel */
    dma_channel_enable(DMA1, DMA_CH4);
}

void hwi_adc_reinit(void)
{
    //hwi_UpdateVref();
    /* enable ADC clock */
    //rcu_periph_clock_enable(RCU_ADC);
    //adc_clock_config(ADC_ADCCK_PCLK2_DIV4);

    adc_deinit();
    /* ADC contineous function disable */
    adc_special_function_config(ADC_CONTINUOUS_MODE, ENABLE);
    /* ADC scan mode disable */
    adc_special_function_config(ADC_SCAN_MODE, ENABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* ADC external trigger config */
    adc_external_trigger_config(ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    /* ADC channel length config */
    adc_channel_length_config( ADC_REGULAR_CHANNEL, 9U);
    /* ADC regular channel config */
    //adc_inserted_channel_config(0U, ADC_CHANNEL_10, ADC_SAMPLETIME_479POINT5);

    /* ADC vbat channel enable */
    //adc_channel_9_to_11(ADC_VBAT_CHANNEL_SWITCH, ENABLE);
    /* ADC temperature and vref enable */
    adc_channel_9_to_11(ADC_TEMP_VREF_CHANNEL_SWITCH, ENABLE);


//    adc_external_trigger_config(ADC_INSERTED_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    adc_regular_channel_config(0U, ADC_CHANNEL_0, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(1U, ADC_CHANNEL_1, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(2U, ADC_CHANNEL_2, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(3U, ADC_CHANNEL_3, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(4U, ADC_CHANNEL_4, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(5U, ADC_CHANNEL_5, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(6U, ADC_CHANNEL_6, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(7U, ADC_CHANNEL_7, ADC_SAMPLETIME_479POINT5);
    //adc_regular_channel_config(8U, ADC_CHANNEL_8, ADC_SAMPLETIME_479POINT5);
  //  adc_regular_channel_config(9U, ADC_CHANNEL_9, ADC_SAMPLETIME_479POINT5);
    adc_regular_channel_config(8U, ADC_CHANNEL_10, ADC_SAMPLETIME_479POINT5);

    adc_oversample_mode_config(ADC_OVERSAMPLING_ALL_CONVERT,ADC_OVERSAMPLING_SHIFT_4B,ADC_OVERSAMPLING_RATIO_MUL16);
    adc_oversample_mode_enable();
    /* enable ADC interface */
    adc_enable();

    hwi_delay_ms(1U);
        /* ADC DMA function enable */
    adc_dma_request_after_last_enable();
    adc_dma_mode_enable();
    /* enable ADC software trigger */
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
}
#endif



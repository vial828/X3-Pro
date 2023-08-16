#ifndef __HWI_ADC_H
#define __HWI_ADC_H

#include "HWI_Hal.h"



extern uint16_t VREFINT;
#define VREFINT_ADDR  (0x040022908)

void hwi_adc_init(void);
void hwi_AdcFirstSelect( uint8_t adc_ch );
uint16_t hwi_AdcFirstRead( void );
void hwi_Adc_sw_enable(void);
void hwi_AdcRead( uint16_t *result );
uint16_t hwi_AdcReadNoblock( uint32_t *value);
void hwi_UpdateVref(void);
void hwi_dma_config_init(void);
void hwi_adc_reinit(void);


#endif


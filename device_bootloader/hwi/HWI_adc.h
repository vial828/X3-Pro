#ifndef __HWI_ADC_H
#define __HWI_ADC_H

#include "HWI_Hal.h"



extern uint16_t VREFINT;

void hwi_adc_init(void);
void hwi_AdcSelect( uint8_t adc_ch );
uint16_t hwi_AdcRead( void );
uint16_t hwi_AdcReadNoblock( uint32_t *value);
void hwi_UpdateVref(void);
//uint32_t hwi_get_random_seed(void);

#endif


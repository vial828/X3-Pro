#ifndef __HWI_SYSTICK_H
#define __HWI_SYSTICK_H

#include "HWI_Hal.h"

#ifdef __cplusplus
 extern "C" {
#endif



void hwi_SysTick_Config(void);
void hwi_SysTick_Handler(void);
uint32_t hwi_SysTick_GetTick(void);

#ifdef __cplusplus
}
#endif

#endif

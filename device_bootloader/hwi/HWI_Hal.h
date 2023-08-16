#ifndef __HWI_HAL_H
#define __HWI_HAL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32w51x.h"



 /***************************************  config - usr  *****************************************/
  /** @addtogroup Exported_types
    * @{
    */

  typedef enum
  {
    HWI_OK       = 0x00U,
    HWI_ERROR    = 0x01U,
    HWI_BUSY     = 0x02U,
    HWI_TIMEOUT  = 0x03U,
  } HWI_StatusTypeDef;


#include "HWI_flash.h"
#include "HWI_power.h"	
#include "HWI_rtc.h"
#include "HWI_systick.h"
#include "HWI_uart.h"
#include "HWI_wdg.h"
#include "HWI_rtc.h"	


/****************function****************/
void hwi_HAL_Init(void);
void hwi_SystemClock_Config(void);
void hwi_HAL_Delay(uint32_t Delay);
void delay_init(void);
void  hwi_delay_us(uint32_t duration);
void hwi_delay_ms(uint32_t duration);


#ifdef __cplusplus
}
#endif

#endif


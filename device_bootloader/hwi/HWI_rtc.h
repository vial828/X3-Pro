#ifndef __HWI_RTC_H
#define __HWI_RTC_H

#include "HWI_Hal.h"


#ifdef __cplusplus
 extern "C" {
#endif



/***************************************  config - usr  *****************************************/
typedef enum
{
    BKP_DR0 = 0x00000070U,
    BKP_DR1 = 0x00000074U,
    BKP_DR2 = 0x00000078U,
    BKP_DR3 = 0x0000007CU,
    BKP_DR4 = 0x00000080U,
		BKP_DR5 = 0x00000084U,
}BKP_DRx_enum;

/****************function****************/
void hwi_RtcEnableDomainRegisterInit(void);
void hwi_WriteBackupRegister(BKP_DRx_enum BKP_DRx,uint32_t data);
uint32_t hwi_ReadBackupRegister(BKP_DRx_enum BKP_DRx);


#ifdef __cplusplus
}
#endif

#endif


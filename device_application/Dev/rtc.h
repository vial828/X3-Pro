#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "HWI_Hal.h"

/***************************************  config - usr  *****************************************/



/****************function****************/
void RtcEnableDomainRegisterInit(void);
void WriteBackupRegister(BKP_DRx_enum BKP_DRx,uint32_t data);
uint32_t ReadBackupRegister(BKP_DRx_enum BKP_DRx);

#ifdef __cplusplus
}
#endif

#endif

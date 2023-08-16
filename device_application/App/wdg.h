#ifndef __WDG_H
#define __WDG_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "HWI_Hal.h"


/***************driver******************/


/****************function****************/
void IWDG_Init(void);
void FeedIwdg(void);
void IWDG_reinit(uint16_t fwdtime);
#ifdef __cplusplus
}
#endif

#endif

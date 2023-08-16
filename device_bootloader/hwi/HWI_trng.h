#ifndef __HWI_TRNG_H
#define __HWI_TRNG_H

#include "HWI_Hal.h"
#ifdef __cplusplus
 extern "C" {
#endif



/***************************************  config - usr  *****************************************/


/****************function****************/

void hwi_TRNG_Init(void);
void hwi_TRNG_Free(void);
uint32_t hwi_trng_generate(void);


#ifdef __cplusplus
}
#endif

#endif


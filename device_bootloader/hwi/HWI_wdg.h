#ifndef __HWI_WDG_H
#define __HWI_WDG_H

#ifdef __cplusplus
 extern "C" {
#endif


#define WDT_TIME_OUT_MS                     2000

/***************************************  config - usr  *****************************************/



/****************function****************/
void hwi_IWDG_Init(void);
void hwi_FeedIwdg(void);


#ifdef __cplusplus
}
#endif

#endif


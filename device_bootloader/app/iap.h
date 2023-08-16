#ifndef __IAP_H__
#define __IAP_H__


/* Exported types ------------------------------------------------------------*/


extern uint32_t IAP_ReadFlag(void);

extern void IAP_WriteFlag(uint32_t flag);

extern void IAP_JumpToApp(void);

void JumpPreparation(void);







#endif

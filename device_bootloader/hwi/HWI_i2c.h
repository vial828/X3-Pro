#ifndef _HWI_I2C_H
#define _HWI_I2C_H

#include "HWI_Hal.h"


#ifdef __cplusplus
 extern "C" {
#endif

#define GPIO_CHANNEL 1

void hwi_I2C_Init(uint8_t channel);
void hwi_I2C_MspInit(uint8_t channel);
HWI_StatusTypeDef hwi_I2C_Mem_Write(uint8_t channel, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HWI_StatusTypeDef hwi_I2C_Mem_Read(uint8_t channel, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

#ifdef __cplusplus
}
#endif

#endif

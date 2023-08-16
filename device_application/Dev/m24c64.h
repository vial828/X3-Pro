#ifndef M24C64_H_
#define M24C64_H_

#include <stdlib.h>
#include <string.h>
#include "HWI_Hal.h"
#include "kernel.h"

void m24c64_Init(void);
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

#endif


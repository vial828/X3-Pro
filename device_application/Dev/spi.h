#ifndef _SPI_H
#define _SPI_H

#include "stdint.h"

void spi_init(void);
void set_spi_cs(uint8_t value);
uint8_t spi_send_byte(uint8_t data);
HAL_StatusTypeDef spi_write(uint8_t *pdata,uint8_t len);
HAL_StatusTypeDef spi_read(uint8_t *pdata,uint8_t len);
HAL_StatusTypeDef spi_write_read(uint8_t *txdata,uint8_t *rxdata,uint8_t len);

#endif

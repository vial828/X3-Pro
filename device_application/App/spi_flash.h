#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "stdint.h"

void disable_spi_flash(void);
void enable_spi_flash(void);
void read_spiflash_id(void);

void read_spiflash_test(void);
void write_spiflash_test(void);

void spiflash_power_down(void);
void spiflash_wake_up(void);
void spiflash_erase_block(uint32_t block);
void spiflash_erase_sector(uint32_t sector);
void spiflash_read_buffer(uint8_t *pbuffer, uint32_t addr, uint16_t num);
void spiflash_write_page(uint8_t *pbuffer, uint32_t addr, uint16_t num);
void spiflash_write_buffer(uint8_t *pbuffer, uint32_t addr, uint16_t num);
uint8_t spiflash_get_byte(uint32_t addr);
uint16_t spiflash_get_halfword(uint32_t addr);
uint32_t spiflash_get_word(uint32_t addr);
void spiflash_set_byte(uint32_t addr, uint8_t val);
void spiflash_set_halfword(uint32_t addr, uint16_t val);
void spiflash_set_word(uint32_t addr, uint32_t val);


#endif



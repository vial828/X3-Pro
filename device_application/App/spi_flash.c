#include "log.h"
#include "kernel.h"
#include "spi.h"
#include "spi_flash.h"
#include <string.h>

#define Dummy_Byte 0x00

#define GD25Q_WriteEnable						0x06
#define GD25Q_WriteDisable					0x04
 
#define GD25Q_ReadStatusReg1				0x05
#define GD25Q_ReadStatusReg2				0x35
#define GD25Q_WriteStatusReg				0x01
 
#define GD25Q_ReadData							0x03
#define GD25Q_FastReadData					0x0B
#define GD25Q_FastReadDual					0x3B
#define GD25Q_PageProgram						0x02
 
#define GD25Q_SectorErase						0x20
#define GD25Q_BlockErase						0xD8
#define GD25Q_ChipErase							0xC7
 
#define GD25Q_PowerDown							0xB9
#define GD25Q_ReleasePowerDown			0xAB
#define GD25Q_DeviceID							0xAB
#define GD25Q_ManufactDeviceID			0x90
#define GD25Q_JedecDeviceID					0x9F

#define GD25Q_PAGE_SIZE             256

void disable_spi_flash(void)
{
	set_spi_cs(0);
	spi_send_byte(GD25Q_PowerDown);
	set_spi_cs(1);
}

void enable_spi_flash(void)
{
	set_spi_cs(0);
	spi_send_byte(GD25Q_ReleasePowerDown);
	set_spi_cs(1);
}

void gd25v_wait_for_busy(void)
{
	uint8_t status;
	
	set_spi_cs(0);
	spi_send_byte(GD25Q_ReadStatusReg1);
	
	do{
		status = spi_send_byte(Dummy_Byte);
	}while((status & 0x01) == 0x01);
	
	set_spi_cs(1);
}

//void Read_Flash_ID(uint8_t * data)
void read_spiflash_id(void)
{
	uint8_t rx_data[16];
	set_spi_cs(0);
	spi_send_byte(GD25Q_JedecDeviceID);
	spi_read(rx_data, 3);
	set_spi_cs(1);
	LOGD("0x%x  0x%x  0x%x\r\n", rx_data[0], rx_data[1], rx_data[2]);
}

void read_spiflash_test(void)
{
	uint8_t tx_data[4] = {GD25Q_ReadData, 0x00, 0x00, 0x00};
	uint8_t rx_data[32];
	
	memset(rx_data, 0, 32);
	
	set_spi_cs(0);
	spi_write(tx_data, 4);
	spi_read(rx_data, 32);
	set_spi_cs(1);
	
	for(uint8_t i=0; i<32; i++){
		LOGD("0x%x\r\n", rx_data[i]);
	}
}

void spiflash_power_down(void)
{
	set_spi_cs(0);
	spi_send_byte(GD25Q_PowerDown);
	set_spi_cs(1);
	
	/* wait for TDP*/
	hwi_HAL_Delay(1);
}

void spiflash_wake_up(void)
{
	set_spi_cs(0);
	spi_send_byte(GD25Q_ReleasePowerDown);
	set_spi_cs(1);
	
	/* wait for TDP*/
	hwi_HAL_Delay(1);
}

void spiflash_write_enable(void)
{
	set_spi_cs(0);
	spi_send_byte(GD25Q_WriteEnable);
	set_spi_cs(1);
}

void spiflash_write_disable(void)
{
	set_spi_cs(0);
	spi_send_byte(GD25Q_WriteDisable);
	set_spi_cs(1);
}

void spiflash_erase_block(uint32_t block)
{
	spiflash_write_enable();
	gd25v_wait_for_busy();
	set_spi_cs(0);
	spi_send_byte(GD25Q_BlockErase);
	spi_send_byte((block & 0xFF0000)>>16);
	spi_send_byte((block & 0xFF00)>>8);
	spi_send_byte(block & 0xFF);
	set_spi_cs(1);
	gd25v_wait_for_busy();
}

void spiflash_erase_sector(uint32_t sector)
{
	spiflash_write_enable();
	gd25v_wait_for_busy();
	set_spi_cs(0);
	spi_send_byte(GD25Q_SectorErase);
	spi_send_byte((sector & 0xFF0000)>>16);
	spi_send_byte((sector & 0xFF00)>>8);
	spi_send_byte(sector & 0xFF);
	set_spi_cs(1);
	gd25v_wait_for_busy();
}

void spiflash_read_buffer(uint8_t *pbuffer, uint32_t addr, uint16_t num)
{
	set_spi_cs(0);
	spi_send_byte(GD25Q_ReadData);
	spi_send_byte((addr & 0xFF0000)>>16);
	spi_send_byte((addr & 0xFF00)>>8);
	spi_send_byte(addr & 0xFF);
	
	while(num--){
		*pbuffer = spi_send_byte(Dummy_Byte);
		pbuffer++;
	}
	
	set_spi_cs(1);
}

void spiflash_write_page(uint8_t *pbuffer, uint32_t addr, uint16_t num)
{
	spiflash_write_enable();
	gd25v_wait_for_busy();
	set_spi_cs(0);
	spi_send_byte(GD25Q_PageProgram);
	spi_send_byte((addr & 0xFF0000)>>16);
	spi_send_byte((addr & 0xFF00)>>8);
	spi_send_byte(addr & 0xFF);
	
	while(num--){
		spi_send_byte(*pbuffer);
		pbuffer++;
	}
	
	set_spi_cs(1);
	gd25v_wait_for_busy();
}

void spiflash_write_buffer(uint8_t *pbuffer, uint32_t addr, uint16_t num)
{
	uint16_t page_remain;

	page_remain = GD25Q_PAGE_SIZE - addr % GD25Q_PAGE_SIZE;
	
	if(num<=page_remain)
		page_remain = num;
	
	while(1){
		spiflash_write_page(pbuffer, addr, page_remain);
		if(num == page_remain){
			break;
		}else{
			pbuffer += page_remain;
			addr += page_remain;
			
			num -= page_remain;
			if(num > GD25Q_PAGE_SIZE){
				page_remain = GD25Q_PAGE_SIZE;
			}else{
				page_remain = num;
			}
		}
	}
}

uint8_t spiflash_get_byte(uint32_t addr)
{
	uint8_t val = 0;
	
	spiflash_read_buffer(&val, addr, 1);
	
	return val;
}

uint16_t spiflash_get_halfword(uint32_t addr)
{
		uint16_t val = 0;
		uint8_t temp[2];
	
		spiflash_read_buffer(temp, addr, 2);
	
		val |= temp[0];
		val |= temp[1]<<8;
	
		return val;
}

uint32_t spiflash_get_word(uint32_t addr)
{
		uint32_t val = 0;
		uint8_t temp[4];
	
		spiflash_read_buffer(temp, addr, 4);
	
		val |= temp[0];
		val |= temp[1]<<8;
		val |= temp[2]<<16;
		val |= temp[3]<<24;
	
		return val;
}

void spiflash_set_byte(uint32_t addr, uint8_t val)
{
	spiflash_write_buffer(&val, addr, 1);
}

void spiflash_set_halfword(uint32_t addr, uint16_t val)
{
	uint8_t temp[2];
	
	temp[0] = val & 0xFF;
	temp[1] = (val>>8)&0xFF;
	
	spiflash_write_buffer(temp, addr, 2);
}

void spiflash_set_word(uint32_t addr, uint32_t val)
{
	uint8_t temp[4];
	
	temp[0] = val & 0xFF;
	temp[1] = (val>>8)&0xFF;
	temp[2] = (val>>16)&0xFF;
	temp[3] = (val>>24)&0xFF;
	
	spiflash_write_buffer(temp, addr, 4);
}




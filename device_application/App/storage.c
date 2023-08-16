#include "kernel.h"
#include "spi_flash.h"
#include "storage.h"
#include "log.h"

void storage_init(void)
{
	enable_spi_flash();
	read_spiflash_id();
}

void storage_task(void)
{

}



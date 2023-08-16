#include "HWI_flash.h"

HWI_StatusTypeDef  HWI_FLASH_Lock(void)
{
  fmc_lock();
    return HWI_OK;
}

HWI_StatusTypeDef  HWI_FLASH_Unlock(void)
{
  fmc_unlock();
    return HWI_OK;
}

HWI_StatusTypeDef HWI_FLASH_Erase(HWI_FLASH_EraseInit *pEraseInit, uint32_t *PageError)
{
  HWI_StatusTypeDef status = HWI_ERROR;
  uint32_t address = 0U;
  fmc_unlock();

  if (pEraseInit->m_typeErase == FLASH_TYPEERASE_MASSERASE)
  {
    /* clear all pending flags */
    fmc_flag_clear (FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);
    fmc_mass_erase();
    /* clear all pending flags */
    fmc_flag_clear (FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);
  }
  else
  {
    /*Initialization of PageError variable*/
    *PageError = 0xFFFFFFFFU;
    /* clear all pending flags */
    fmc_flag_clear (FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);
    /* Erase page by page to be done*/
    for (address = pEraseInit->m_page*FLASH_PAGE_SIZE+FLASH_BASE;
         address < ( (pEraseInit->m_numberPages * FLASH_PAGE_SIZE) + pEraseInit->m_page*FLASH_PAGE_SIZE+FLASH_BASE);
         address += FLASH_PAGE_SIZE)
    {
      if(FMC_READY!=fmc_page_erase (address))
	  {
		  *PageError=address;
		  fmc_lock();
		  /* clear all pending flags */
		  fmc_flag_clear (FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);		  
		  return status;		 
	  }
      /* clear all pending flags */
      fmc_flag_clear (FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);
    }
  }
  fmc_lock();
  return HWI_OK;
}

HWI_StatusTypeDef  HWI_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
{
  HWI_StatusTypeDef status = HWI_ERROR;

  /* unlock the flash program/erase controller */
  fmc_unlock();
	__disable_irq();
	icache_disable();
	
  if(FMC_READY!=fmc_word_program(Address, Data))
  {
		icache_enable();			
		__enable_irq();
	
	  fmc_lock();
	  /* clear all pending flags */
	  fmc_flag_clear (FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);		  
	  return status;		    
  }
	if(FLASH_TYPEPROGRAM_DOUBLEWORD==TypeProgram)
	{
		if(FMC_READY!=fmc_word_program(Address+4, (Data>>32U)))
		{
			icache_enable();			
			__enable_irq();
		
			fmc_lock();
			/* clear all pending flags */
			fmc_flag_clear (FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);		  
			return status;		    
		}		
	}		
	icache_enable();			
	__enable_irq();
  fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);

  /* lock the main FMC after the program operation */
  fmc_lock();

  return HWI_OK;
}

uint32_t HWI_FLASH_GetError (void)
{
  return 0;
}



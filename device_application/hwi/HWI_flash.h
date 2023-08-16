#ifndef __HWI_FLASH_H
#define __HWI_FLASH_H
      
#include "HWI_Hal.h"
#include "gd32w51x.h"


/** @defgroup FLASH_Type_Erase FLASH Erase Type
  * @{
  */
//#define FLASH_TYPEERASE_PAGES           FLASH_CR_PER    /*!< Pages erase only */
//#define FLASH_TYPEERASE_MASS            FLASH_CR_MER1   /*!< Flash mass erase activation */

#define FLASH_TYPEERASE_PAGES     0x00U  /*!<Pages erase only*/
#define FLASH_TYPEERASE_MASSERASE 0x02U  /*!<Flash mass erase activation*/
#define FLASH_PAGE_SIZE           ((uint16_t)0x1000U)
#define FLASH_TYPEPROGRAM_DOUBLEWORD    0   /*!< Program a double-word (64-bit) at a specified address */
#define FLASH_SIZE           	  ((uint32_t)0x200000U)  /*!<GD32W515PIQ6 FLASH size 2 MB */

/**
  * @brief  FLASH Erase structure definition
  */
typedef struct
{
    uint32_t m_typeErase;       /*!< Mass erase or page erase.
                                  This parameter can be a value of @ref FLASH_Type_Erase */
    uint32_t m_page;            /*!< Initial Flash page to erase when page erase is enabled
                                  This parameter must be a value between 0 and (FLASH_PAGE_NB - 1) */
    uint32_t m_numberPages;     /*!< Number of pages to be erased.
                                  This parameter must be a value between 1 and (FLASH_PAGE_NB - value of initial page)*/
} HWI_FLASH_EraseInit;

HWI_StatusTypeDef  HWI_FLASH_Unlock(void);
HWI_StatusTypeDef  HWI_FLASH_Lock(void);

HWI_StatusTypeDef HWI_FLASH_Erase(HWI_FLASH_EraseInit *pEraseInit, uint32_t *PageError);

HWI_StatusTypeDef  HWI_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data);

uint32_t HWI_FLASH_GetError(void);

#endif


/******************************************************************************
*
*    Copyright (C), 2019-2020, xi'an byd Co., Ltd.
*
*******************************************************************************
* file name :
* description:

* author    :
* vertion   :
* data      :

*
*history version:
*      <author>         <date>           <version>      <description>
*
******************************************************************************/


/*******************************--includes--**********************************/
#include "HWI_Hal.h"

#include "dev_flash_ex.h"

#include "iap_config.h"


#include "flash.h"
#include <string.h>
/*******************************--define--************************************/



/*****************************--type define--*********************************/



/***************************--global variable--*******************************/
static uint8_t SaveBuffer[TOTAL_SAVE_LEN] = {0};
static boot_record_t boot_record_s;
/*****************************************************************************/
void Flash_EnableRDP_WRP_Pages(uint32_t wrp_start_addr, uint32_t wrp_end_addr)
{
    uint16_t spage=0,epage=0;

    if((FMC_OBR&0x000000ff)!=0xcc)
    {
        rcu_periph_clock_enable(RCU_PMU);
        /* unlock the flash program/erase controller */
        fmc_unlock();
        fmc_flag_clear(FMC_FLAG_END | FMC_FLAG_WPERR | FMC_FLAG_OBERR);
        ob_unlock();
        spage=(wrp_start_addr-FLASH_BASE)/FLASH_PAGE_SIZE;
        epage=(wrp_end_addr-FLASH_BASE)/FLASH_PAGE_SIZE;
        /* configure write protection pages */
        ob_write_protection_config(spage, epage, OBWRP_INDEX0);
        ob_security_protection_config(FMC_SPC_P1);			

        //ob_write_protection_config(WRP_REGION_SPAGE_DEFAULT_VALUE, WRP_REGION_EPAGE_DEFAULT_VALUE, OBWRP_INDEX1);
        /* start option bytes modification */
        hwi_delay_ms(100);

        __disable_irq();
        pmu_wakeup_pin_enable(PMU_CS0_WUPEN1|PMU_CS0_WUPEN2|PMU_CS0_WUPEN3);
        pmu_flag_clear(PMU_FLAG_RESET_WAKEUP);
        pmu_to_standbymode();
    }
}


/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
int8_t Flash_ErasePages(uint32_t Address, uint32_t size)
{
    uint32_t pageError = 0;
    int8_t ret = 0;

    HWI_FLASH_EraseInit  flashEraseInit;

    flashEraseInit.m_typeErase = FLASH_TYPEERASE_PAGES;
    flashEraseInit.m_page = (Address - FLASH_BASE)/FLASH_PAGE_SIZE;
    flashEraseInit.m_numberPages = (size+FLASH_PAGE_SIZE-1)/FLASH_PAGE_SIZE;
    HWI_FLASH_Unlock();
    if(HWI_FLASH_Erase(&flashEraseInit,&pageError) == HWI_OK)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    HWI_FLASH_Lock();

    return ret;
}

/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
int8_t Flash_Program(uint32_t Address, uint64_t Data)
{
    int8_t ret = 0;

    HWI_FLASH_Unlock();

    if(HWI_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,Address,Data) == HWI_OK)
    {
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    HWI_FLASH_Lock();

    return ret;
}

/******************************************************************************
 * function name:
 * description:
 * input:
 * return:
 * exception:
******************************************************************************/
static uint64_t Convert_64Bits(const uint8_t* dat,uint8_t len)
{
    uint64_t u64data = 0;
    uint8_t temp = 0;
    uint8_t i = 0;

    if(len >= (uint8_t)8)
    {
       len = (uint8_t)8;
    }

    for(i = 0;i < len;i++)
    {
        temp = dat[i];
        u64data |= (((uint64_t)temp) << (8*i));
    }
    return u64data;
}

static void Flash_DataInit(void)
{
    uint8_t i = 0;
    for(i = 0;i< TOTAL_SAVE_LEN;i ++)
    {
        SaveBuffer[i] = *(uint8_t*)(DATA_START_ADDR+i);
    }
}




boot_record_t * get_boot_record_from_flash(void)
{
    /* Read record data from bootloader flash and copy to boot_record_s */
    memcpy((void*)&boot_record_s, (void *)DATA_START_ADDR, sizeof(boot_record_t));
    return &boot_record_s;
}

boot_record_t * get_boot_record_from_ram(void)
{
    return &boot_record_s;
}

void update_boot_flash(void)
{
    uint64_t *flash_temp = (uint64_t*)&boot_record_s;
    uint32_t boot_record_double_word_num = sizeof(boot_record_t)/8;
    uint32_t Address = DATA_START_ADDR;

    /* Unlock the Flash to enable the flash control register access *************/
    HWI_FLASH_Unlock();

    uint32_t pageError = 0;
    HWI_FLASH_EraseInit  flashEraseInit;

    flashEraseInit.m_typeErase = FLASH_TYPEERASE_PAGES;
    flashEraseInit.m_page = (DATA_START_ADDR - FLASH_BASE)/FLASH_PAGE_SIZE;
    flashEraseInit.m_numberPages = 1;

    HWI_FLASH_Erase(&flashEraseInit,&pageError);
    for(int i = 0; i < boot_record_double_word_num; i++)
    {
        if(HWI_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, *flash_temp) == HWI_OK)
        {
            Address+=8;
            flash_temp++;
        }
        else
        {
            //Error_Handler();
        }
    }

    HWI_FLASH_Lock();

}







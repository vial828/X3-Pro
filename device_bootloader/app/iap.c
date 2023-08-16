
#include "HWI_Hal.h"

#include "dev_uart.h"

#include "dev_flash_ex.h"

#include "iap_config.h"

#include "flash.h"

#include "ymodem.h"

#include "iap.h"

#include "version.h"

#include "securityBoot.h"

void IAP_WriteFlag(uint32_t flag)
{
	#if 0
    uint64_t u64Flag = (uint64_t)flag;
    HAL_FLASH_Unlock();
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,IAP_FLAG_ADDR,u64Flag);
    HAL_FLASH_Lock();
	#endif
}


uint32_t IAP_ReadFlag(void)
{
    uint32_t flag = 0;
    flag = *(__IO uint32_t*)(IAP_FLAG_ADDR);
    return flag;
}

/*uint64_t volatile flash_temp[256];*/
void JumpPreparation(void)
{
    boot_record_t* brt = get_boot_record_from_ram();

    /* app update flag */
    brt->app_update_flag = APPRUN_FLAG_DATA;

    /* app file size */
    brt->app_file_size = File_Size;

    /* app file name */
    memset(brt->app_file_name, 0, sizeof(brt->app_file_name));
    strcpy(brt->app_file_name, (char*)file_name);

    /* app signature */
    memcpy(brt->app_signature, signature, SIGNATURE_LENGTH);

    brt->return_err = ERR_CLEAR;
    brt->reset_err = ERR_CLEAR;
    brt->error_pos = 0;
    brt->comm_lock_flag = 0;//serial communication locked

    update_boot_flash();

}


void IAP_JumpToApp(void)
{
    /* a function pointer to hold the address of the reset handler of the user app */
    void (*jump_to_app)(void);
    /* 0x20009000 : RAM address */
    if((*(volatile uint32_t*)APP_ADDRESS) < (uint32_t)0x20060000)
    {
        /* Close global interrupt */
//        __set_PRIMASK(1);
        /* Clear intterupt Flag */
//        NVIC_ClearPendingIRQ(SysTick_IRQn);
//        /* Reset the RCC clock configuration to the default reset state.*/
//        LL_RCC_DeInit();
//        /* Reset peripheral to the default reset state */
//        Dev_UART_DeInit(DEV_UART1);
        /* 1. reading the MSP value */
        uint32_t msp_value = *(volatile uint32_t *)APP_ADDRESS;
        /* 2. set the MSP value */
        __set_MSP(msp_value);
        /* 3. set the reset address of the user application */
        uint32_t reset_address = *(volatile uint32_t *) (APP_ADDRESS + 4);
        /* 4. jump to app */
        jump_to_app = (void (*)(void))reset_address;
        jump_to_app();
    }
}

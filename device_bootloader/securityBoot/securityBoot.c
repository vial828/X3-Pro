#include "securityBoot.h"
#include "crypto.h"
#include "iap_config.h"
#include "ymodem.h"
#include "flash.h"
#include "HWI_Hal.h"
#include "dev_uart.h"

const uint8_t public_key[17] = {0x03,0xAA,0x77,0xF7,0xBA,0x81,0x1B,0x9C,0xD9,0x50,0xA7,0xD4,0x9C,0x19,0x00,0x25,0xCF};

void do_hash(char * hash_code)
{
    hashalg_CTX ctx;
    char block_buf[9];

    hashalgInit(&ctx);

    uint32_t addr_temp = APP_ADDRESS;

    while(addr_temp < APP_ADDRESS + File_Size)
    {
        memcpy(block_buf,(uint32_t*)(addr_temp),HASH_BLOCK);
        hashalgUpdate(&ctx, (const unsigned char*)block_buf, HASH_BLOCK);
        addr_temp += 4;
    }

   hashalgFinal((unsigned char *)hash_code, &ctx);
   hash_code[20] = '\0';

}

void delay_ms(uint32_t delay)
{
    for(uint32_t i = 0;i< 32;i++)
    {
        for(uint32_t j =0;j< delay;j++)
        {
            __NOP();
        }
    }
}

int securityboot(void)
{
    int state = 0;
    char hash_code[21];

    do_hash(hash_code);

    state = cryalgsa_verify(public_key,(uint8_t *)hash_code,signature);

    if(state != 1)
    {
        Flash_ErasePages(DATA_START_ADDR, (APP_ADDRESS - DATA_START_ADDR));
        Flash_ErasePages(APP_ADDRESS, File_Size);
        delay_ms(300);
        //Flash_ErasePages(APP_ADDRESS, File_Size);
        NVIC_SystemReset();
    }
    return state;
}

int securitybootAfterUpdate(void)
{
    int state = 0;
    char hash_code[21];

    do_hash(hash_code);

    state = cryalgsa_verify(public_key,(uint8_t *)hash_code,signature);

    if(state != 1)
    {
        vd_g_CommandExp_PackSendData(0xF3);
        Flash_ErasePages(APP_ADDRESS, File_Size);
        hwi_HAL_Delay(300);
        //Flash_ErasePages(APP_ADDRESS, File_Size);
        NVIC_SystemReset();
    }
    vd_g_CommandExp_PackSendData(0x00);
    return state;
}



#ifndef   __FLASH__H
#define   __FLASH__H

#include "stdint.h"
#include "ymodem.h"

#define ERR_EXIST   0x6789
#define ERR_CLEAR   0x0000

#pragma pack(1)
typedef struct
{
    uint8_t bootloader_version[32];
    uint64_t app_update_flag;
    uint64_t app_file_size;
    char app_file_name[FILE_NAME_LENGTH];
    uint8_t app_signature[32];

    /* Serial Comm lock flag*/
    uint32_t comm_lock_flag;

    /* error record */
    uint16_t return_err;
    uint16_t reset_err;
    uint32_t error_pos;
}boot_record_t;
#pragma pack()

int8_t Flash_ErasePages(uint32_t Address, uint32_t size);

int8_t Flash_Program(uint32_t Address, uint64_t Data);


void Flash_EnableRDP_WRP_Pages(uint32_t wrp_start_addr, uint32_t wrp_end_addr);

boot_record_t * get_boot_record_from_flash(void);
boot_record_t * get_boot_record_from_ram(void);
void update_boot_flash(void);
#endif










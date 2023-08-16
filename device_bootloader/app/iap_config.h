#ifndef IAP_CONFIG_H
#define IAP_CONFIG_H

#include "ymodem.h"

/* Define the APP start address */
#define  APP_ADDRESS     0x08011000

/* IAP command */
/* flash last page */
#define DATA_START_ADDR      (uint32_t)(APP_ADDRESS - 0x1000)
#define BOOT_VERSION_ADDR    DATA_START_ADDR
#define IAP_FLAG_ADDR        (BOOT_VERSION_ADDR + 32)
#define FILE_SIZE_ADDR       (IAP_FLAG_ADDR + 8)
#define FILE_NAME_ADDR       (FILE_SIZE_ADDR + 8)
#define SIGNATURE_ADDR       (FILE_NAME_ADDR + FILE_NAME_LENGTH)


#define TOTAL_SAVE_LEN       24

#define INIT_FLAG_DATA        0xFFFF   //默认标志的数据(空片子的情况)

#define UPDATE_FLAG_DATA      0x12345   //下载标志的数据

#define APPRUN_FLAG_DATA      0x5A5A   //update over

/* Compute the FLASH upload image size --------------------------*/
#define FLASH_IMAGE_SIZE    (uint32_t) (0x200000UL - (APP_ADDRESS - 0x08000000UL))
/* The maximum length of the command string */
#define  CMD_STRING_SIZE       128

#endif

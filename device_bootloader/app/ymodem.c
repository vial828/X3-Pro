/******************************************************************************
*
*    Copyright (C), 2019-2020, xi'an byd Co., Ltd.
*
*******************************************************************************
* file name :
* description: This file provides all the software functions related to the ymodem
  *          protocol.

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

#include "dev_uart.h"
#include "dev_flash_ex.h"

#include "flash.h"

#include "iap_config.h"

#include "ymodem.h"

#include "securityBoot.h"
#include "AES.h"

/*******************************--define--************************************/
#define APP_VER_CHECK

#define DATA_MAX_LEN  150
#define KEYLEN 16

/*****************************--type define--*********************************/



/***************************--global variable--*******************************/

uint8_t file_name[FILE_NAME_LENGTH] = {0};

uint32_t File_Size = 0;

uint32_t FlashDestination = APP_ADDRESS;

uint8_t Packet_Data[PACKET_128B_SIZE + PACKET_OVERHEAD]= {0};

uint8_t StartUpdateFlag = 0;

const uint8_t key[16] = {0x69, 0x38, 0x6A, 0x34, 0x65, 0x45, 0x72, 0x78, 0x78, 0x6C, 0x65, 0x2D, 0x54, 0x54, 0x50, 0x73};
/*****************************************************************************/

static uint16_t u16_s_CommandExp_GetCrcCheckValue(const uint8_t* Buff,const uint32_t BuffSize)
{
    uint16_t CRCValue = 0;
    uint32_t SizeCnt = 0;

    for(SizeCnt = 0;SizeCnt < BuffSize;SizeCnt++)
    {
        CRCValue += Buff[SizeCnt];
    }

    return (CRCValue);
}

static uint8_t u8_s_CommandExp_InstrCheckIsValid(uint8_t* DataBuff,uint32_t InstrLen)
{
    uint16_t CrcValue = 0;
    uint16_t RecvCrcValue = 0;

    CrcValue = u16_s_CommandExp_GetCrcCheckValue(DataBuff,InstrLen - 3);

    RecvCrcValue = ((DataBuff[InstrLen - 3]) << 8) | DataBuff[InstrLen - 2];

    if(CrcValue != RecvCrcValue)
    {
        return (0);
    }

    return (1);
}

void vd_g_CommandExp_PackSendData(uint8_t Command)
{
    uint16_t CrcValue = 0;
    uint8_t  SendData[20] = {0};

    SendData[0] = CMD_HEAD;
    SendData[1] = CMD_VER;
    SendData[2] = CMD_SID;
    SendData[3] = CMD_OWNER;
    SendData[4] = Command;
    SendData[5] = (0 >> 8) & 0xff;
    SendData[6] = (0 >> 0) & 0xff;
    CrcValue = u16_s_CommandExp_GetCrcCheckValue(SendData,0 + 7);
    SendData[0 + 7] = (CrcValue >> 8) & 0xff;
    SendData[0 + 8] = (CrcValue >> 0) & 0xff;
    SendData[0 + 9] = CMD_TAIL;

    Dev_UART_Send(DEV_UART1,SendData,0 + 10);
}


static int8_t Receive_Packet(uint8_t *dat,uint32_t timeout)
{
    uint16_t InstrLen = 0;
    uint32_t TimeCnt = hwi_SysTick_GetTick();

    while(gUartRcvDataFlag == 0)
    {
        if(hwi_SysTick_GetTick() - TimeCnt >= timeout)
        {
            return ERR_RECV_TIMEOUT;
        }
    }

    gUartRcvDataFlag = 0;

    InstrLen = (Ring_Buffer[5] << 8) | Ring_Buffer[6];

    if((Ring_Buffer[0] != CMD_HEAD)||(InstrLen != PACKET_128B_SIZE))
    {
        return ERR_PACKET_HEAD;
    }
    else
    {
        InstrLen += CMD_MIN_LEN;

        if(Ring_Buffer[InstrLen - 1] != CMD_TAIL)
        {
            return ERR_PACKET_TAIL;
        }
        else
        {
            if(u8_s_CommandExp_InstrCheckIsValid(Ring_Buffer,InstrLen) != 1)
            {
                return ERR_PACKET_CRC;
            }
            else
            {
                memcpy(dat,Ring_Buffer,PACKET_128B_SIZE + CMD_MIN_LEN);
            }
        }
    }
    return 0;
}

int8_t app_file_version_check(char * app_file_name)
{
    /* check if name of app file has "_APP_S_"*/
    if(strstr(app_file_name, APP_FILE_NAME_HEAD) == NULL){
        return ERR_FILE_NAME;
    }

    /* check if name of app file has "." */
    char * tmp = strstr(app_file_name, ".");
    if(tmp == NULL){
        return ERR_FILE_NAME;
    }

    /* get old app version string which stored in flash */
    boot_record_t* brt = get_boot_record_from_ram();
    /* E.g.
       tmp = ".06.08.1"
       new_app_version = "2021.06.08.1"
    */

    //get last character
    char old_version_u_or_f = brt->app_file_name[strlen(brt->app_file_name) - 1];
    char new_version_u_or_f = app_file_name[strlen(app_file_name) - 1];
    if(old_version_u_or_f == 'u' && new_version_u_or_f == 'f')
    {
        return ERR_USR_TO_FACTORY;
    }
#ifndef RDP2    
    else{
        return 0;
    }
#else
    char * new_app_version = tmp - 4;
    char * old_app_version = strstr(brt->app_file_name, ".") - 4;

    /* compare year, then month, then date, then version number */
    if(strcmp(new_app_version, old_app_version) >= 0){
        return 0;
    }
    else{
        return ERR_APP_VERSION;
    }
#endif
}

int8_t Ymodem_ParseFirstPacket(const uint8_t *dat)
{
    uint16_t i = 0;
    int8_t ret = 0;
    uint16_t len = 0;
    boot_record_t* brt = get_boot_record_from_ram();

    File_Size = 0;
    /* Filename packet */
    if (dat[0] != 0)  //fine name
    {
        /* Filename packet has valid data */
        for (i = 0; (dat[i] != 0)&&(i < FILE_NAME_LENGTH-1);i ++,len ++)
        {
            file_name[i] = dat[i];  // backup filename
        }
        file_name[i++] = '\0';//as a string terminator 

        #ifdef APP_VER_CHECK
        ret = app_file_version_check((char *)file_name);
        if(ret != 0){
            brt->app_update_flag = APPRUN_FLAG_DATA;
            update_boot_flash();
            vd_g_CommandExp_PackSendData(ret);
            //delay_ms(300);
            NVIC_SystemReset();
        }
        #endif

        len ++;
        for (i = 0; i < FILE_SIZE_LENGTH;i++)
        {
            File_Size = (File_Size << 8);
            File_Size |= (dat[len+i] & 0x000000ff); //file size
        }
        /* Test the size of the image to be sent */
        /* Image size is greater than Flash size */
        if (File_Size < FLASH_IMAGE_SIZE)
        {
            /* Erase the needed pages where the user application will be loaded */
            /* Define the number of page to be erased */
            StartUpdateFlag = 1;
            if(Flash_ErasePages(APP_ADDRESS, File_Size) == 0)
            {
                ret = 0;
            }
            else
            {
                /* End session */
                ret = ERR_FLASH_ERASE;         //Erase failed
                vd_g_CommandExp_PackSendData(ret);
                NVIC_SystemReset();
            }
        }
        else
        {
            ret = ERR_FILE_SIZE;     // file size too big
        }
    }
    else   /* Filename packet is empty, end session */
    {
        ret = ERR_FILE_NAME;  // Filename is empty
    }
    return ret;
}


uint64_t Convert_64Bits(const uint8_t* dat,uint8_t len)
{
    uint64_t u64data = 0;
    uint8_t temp = 0;
    uint8_t i = 0;
    if(len >= 8)
    {
       len = 8;
    }
    for(i = 0;i < len;i++)
    {
        temp = dat[i];
        u64data |= (((uint64_t)temp) << (8*i));
    }
    return u64data;
}

int8_t Ymodem_ParseDataPacket(const uint8_t *dat,uint16_t length)
{
    int8_t ret = 0;
    uint64_t data = 0;
    uint32_t i = 0;

    uint8_t plainData[PACKET_128B_SIZE] = {0};
    aesDecrypt(key, KEYLEN, dat, plainData, PACKET_128B_SIZE);
    for (i = 0;i < length;i += 8)
    {
         data = Convert_64Bits(&plainData[i],8);

         if(Flash_Program(FlashDestination,data) == 0)
         {
             ret = 0;
             FlashDestination += 8;
         }
         else
         {
             ret = ERR_FLASH_PROGRAM;  /* program failed */
             break;
         }
     }
     return ret;
}

/**
  * @brief  Cal Check sum for YModem Packet
  * @param  data
  * @param  length
   * @retval None
  */
uint8_t CalChecksum(const uint8_t* data, uint32_t size)
{
    uint8_t crc = 0xFF;
    const uint8_t* dataEnd = data+size;
    while(data < dataEnd )
    {
        crc ^= *data++;
    }
    return (crc);
}


int8_t Ymodem_ParseLastPacket(const uint8_t *dat)
{
    int8_t ret = 0;
    uint32_t recieve_bin_size = 0;
    //uint16_t recieve_crc = 0;
    //uint16_t crc = 0;

    for (uint8_t i = 0; i < FILE_SIZE_LENGTH;i++)
    {
        recieve_bin_size = (recieve_bin_size << 8); //file size 
        recieve_bin_size |= (dat[0 + i] & 0x000000ff);
    }

    //recieve_crc = (((uint16_t)dat[4])<< 8); //file CRC
    //recieve_crc +=(((uint16_t)dat[5])<< 0);

    if(recieve_bin_size == File_Size)
    {
        //crc = CalChecksum((uint8_t*)APP_ADDRESS,recieve_bin_size);

        //if(recieve_crc == crc)
        //{
        //    ret = 1;
        //    StartUpdateFlag = 0;
        //}
        //else
        //{
        //    ret = ERR_FILE_CRC;
        //}
        ret = 1;
    }
    else
    {
        ret = ERR_FILE_SIZE;
    }
    return ret;
}

int8_t Ymodem_ParseSignaturePacket(const uint8_t *dat)
{
    int8_t ret = 0;

    /* Signature packet */
    memcpy(signature, dat, SIGNATURE_LENGTH);
    return ret;
}


/******************************************************
  * @brief  Receive a file using the ymodem protocol
  * @param  buf: Address of the first byte
  * @retval The size of the file
  ****************************************************/
int8_t IAP_Update(void)
{
    static uint16_t  packets_received = RX_UPDATE_START;
    int8_t  errorCode = 0;
    errorCode = Receive_Packet(Packet_Data,NAK_TIMEOUT);
    if (errorCode == 0)
    {
        if (Packet_Data[PACKET_SEQNO_INDEX] == RX_UPDATE_START)    // first packet: file info
        {
            errorCode = Ymodem_ParseFirstPacket(&Packet_Data[PACKET_HEADER]);
            FlashDestination = APP_ADDRESS;
            packets_received = RX_UPDATE_START+1;
        }
        else if(Packet_Data[PACKET_SEQNO_INDEX] == RX_UPDATE_DONE)
        {
            errorCode = Ymodem_ParseLastPacket(&Packet_Data[PACKET_HEADER]);
            FlashDestination = APP_ADDRESS;
            packets_received = RX_UPDATE_START;
        }
        else if(Packet_Data[PACKET_SEQNO_INDEX] == RX_SIGNATURE)
        {
            errorCode = Ymodem_ParseSignaturePacket(&Packet_Data[PACKET_HEADER]);
        }
        else  /* Start receiving data after saving the file information */
        {
            if (Packet_Data[PACKET_SEQNO_INDEX] != packets_received)
            {
                errorCode = ERR_PACKET_NUM;  // packet Number lost
            }
            else
            {
                if(Ymodem_ParseDataPacket(&Packet_Data[PACKET_HEADER],PACKET_128B_SIZE) != 0)
                {
                    errorCode = ERR_FLASH_PROGRAM;  // Program Flash error
                    vd_g_CommandExp_PackSendData(errorCode);
                    NVIC_SystemReset();
                }
                else
                {
                    packets_received ++;
                    if(packets_received > 0xAF)
                    {
                        packets_received = 0xA1;
                    }
                }
            }
        }
    }
    vd_g_CommandExp_PackSendData(errorCode);
    return errorCode;
}

/**
  * @brief  Update CRC16 for input byte
  * @param  CRC input value
  * @param  input byte
   * @retval None
  */
uint16_t UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
    uint32_t crc = crcIn;
    uint32_t in = byte|0x100;
    do
    {
        crc <<= 1;
        in <<= 1;
        if(in&0x100)
        {
            ++crc;
        }
        if(crc&0x10000)
        {
            crc ^= 0x1021;
        }
    } while(!(in&0x10000));
    return crc&0xffffu;
}

/**
  * @brief  Cal CRC16 for YModem Packet
  * @param  data
  * @param  length
  * @retval None
  */
uint16_t Cal_CRC16(const uint8_t* data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t* dataEnd = data+size;
    while(data < dataEnd)
    {
        crc = UpdateCRC16(crc,*data++);
    }

    crc = UpdateCRC16(crc,0);
    crc = UpdateCRC16(crc,0);
    return crc&0xffffu;
}


#include "HWI_Hal.h"
#include "i2c.h"
#include "HWI_i2c.h"
#include "log.h"
#include "dev_gauge_bq27z561r2.h"
#include "batTimer.h"
#include "string.h"
#include "wdg.h"
#include "rtc.h"
#include "HWI_i2c.h"

uint8_t databuf[20];

uint8_t Dev_BQ27Z561R2_ReadRegBits(uint8_t reg,uint8_t *data,uint8_t size)
{
    uint8_t ret = 0;
    //ret=hwi_I2C_Mem_Read(&hi2c2,BQ27Z561_ADDR,reg,I2C_MEMADD_SIZE_8BIT,data,1,100);
    ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ27Z561R2_ADDR,reg,I2C_MEMADD_SIZE_8BIT,data,size,0xfff);

    if(ret)
    {
        return I2C_READ_ERROR;
    }
    return ret;
}
/*******************************************************************************
* @brief:write data to reg
* @param1:reg need to write
* @param2:write data
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_WriteRegBits(uint8_t reg,uint8_t data)
{
    uint8_t ret = 0;
    //ret=hwi_I2C_Mem_Read(&hi2c2,BQ27Z561_ADDR,reg,I2C_MEMADD_SIZE_8BIT,data,1,100);
    ret = hwi_I2C_Mem_Write(GPIO_CHANNEL,BQ27Z561R2_ADDR,reg,I2C_MEMADD_SIZE_8BIT,&data,1,0xfff);

    if (ret)
    {
        return I2C_WRITE_ERROR;
    }
    return ret;
}
/*******************************************************************************
* @brief:write data to reg
* @param1:reg need to write
* @param2:write data
* @param2:write data lenth
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_WriteReg(uint8_t reg,uint8_t *data,uint8_t len)
{

    uint8_t ret = 0;
    ret = hwi_I2C_Mem_Write(GPIO_CHANNEL,BQ27Z561R2_ADDR,reg,I2C_MEMADD_SIZE_8BIT,data,len,0xfff);

    if (ret)
    {
        return I2C_WRITE_ERROR;
    }
    return ret;
}
/*******************************************************************************
* @brief:read and compare data from reg
* @param1:reg name which reg need to read
* @param2:cpdata to compare with read data
* @param3:data buffer to save read data
* @param4:read data size
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_ReadAndCompare(uint8_t reg,uint8_t *cpdata,uint8_t *rdata,int32_t size)
{
    uint8_t ret=0,k=0;
    ret=Dev_BQ27Z561R2_ReadRegBits(reg,rdata,size);
    if(ret){
        LOGD("gauge read error");
        return ERROR;
    }
    do{
        if(cpdata[size-1]!=rdata[size-1]){
            LOGD("gauge compare error");
            k=size;
            return ERROR;
        }
    }while(--size);
    return SUCCESS;
}
/*******************************************************************************
* @brief:read data from reg
* @param1:reg need to read
* @param2:data buffer to save read data
* @param3:read data size
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_ReadROMBits(uint8_t reg,uint8_t *data,uint8_t size)
{
    uint8_t ret = 0;
    ret = hwi_I2C_Mem_Read(GPIO_CHANNEL,BQ27Z561R2_ROM_ADDR,reg,I2C_MEMADD_SIZE_8BIT,data,size,0xfff);

    if(ret)
    {
        return I2C_READ_ERROR;
    }
    return ret;
}
/*******************************************************************************
* @brief:write data to ROM bit
* @param1:reg need to write
* @param2:write data
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_WriteROMBits(uint8_t reg,uint8_t *data)
{
    uint8_t ret = 0;
    ret = hwi_I2C_Mem_Write(GPIO_CHANNEL,BQ27Z561R2_ROM_ADDR,reg,I2C_MEMADD_SIZE_8BIT,data,1,0xfff);

    if (ret)
    {
        return I2C_WRITE_ERROR;
    }
    return ret;
}

/*******************************************************************************
* @brief:write data to ROM
* @param1:reg need to write
* @param2:write data
* @param2:write data lenth
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_WriteROM(uint8_t reg,uint8_t *data,uint8_t len)
{

    uint8_t ret = 0;
    ret = hwi_I2C_Mem_Write(GPIO_CHANNEL,BQ27Z561R2_ROM_ADDR,reg,I2C_MEMADD_SIZE_8BIT,data,len,0xfff);

    if (ret)
    {
        return I2C_WRITE_ERROR;
    }
    return ret;
}

/*******************************************************************************
* @brief:read and compare data from reg
* @param1:reg name which reg need to read
* @param2:cpdata to compare with read data
* @param3:data buffer to save read data
* @param4:read data size
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_ROM_ReadAndCompare(uint8_t reg,uint8_t *cpdata,uint8_t *rdata,int32_t size)
{
    uint8_t ret,k;
    ret=Dev_BQ27Z561R2_ReadROMBits(reg,rdata,size);
    if(ret){
        LOGD("gauge rom read error");
        return ERROR;
    }
    do{
        if(cpdata[size-1]!=rdata[size-1]){
            LOGD("gauge rom compare error");
            k=size;
            return ERROR;
        }
    }while(--size);
    return SUCCESS;
}
/*************************************************************************************************
 * @brief    : read ic device name
 * @return   : null
*************************************************************************************************/
uint8_t Dev_BQ27Z561R2_ReadID(void)
{
    uint8_t data[10]={0};
    char name[8]={"bq27z561"};
    uint8_t flag = 0;

    Dev_BQ27Z561R2_WriteRegBits(0x3E,0x4A);
    Dev_BQ27Z561R2_WriteRegBits(0x3F,0x00);
    vTaskDelay(1);
    Dev_BQ27Z561R2_ReadRegBits(0x3E,data,10);
    for(uint8_t i = 0;i < 8; i++)
    {
        if(name[i] == data[i+2]){
            flag++;
        }
        else{
            flag=0;
        }
    }
    if(flag == 8)
    {
        return 1;//soc ID is bq27z561
    }
    else
    {
        return 0;
    }
}

/*************************************************************************************************
 * @brief    : set ic wake time to 20s
 * @return   : null
*************************************************************************************************/
void Dev_BQ27Z561R2_setwakechecktime(void)
{
    //write data
    Dev_BQ27Z561R2_WriteRegBits(0x3E,0x5F); //write 0x465F address
    Dev_BQ27Z561R2_WriteRegBits(0x3F,0x46);

    Dev_BQ27Z561R2_WriteRegBits(0x40,0x14); //write data 0X14 to 0x40 address

    Dev_BQ27Z561R2_WriteRegBits(0x60,0x46);//crc= 0xff - write 0x3e start data(address + data values)(0xff-0x5F-0x46-0x14=0x46)
    Dev_BQ27Z561R2_WriteRegBits(0x61,0x05);//data length = write 0x3e start data length + write 0x60 start data length(0x5F-0x46-0x15-0x46-0x05)

    //read data
    /*Dev_BQ27Z561R2_WriteRegBits(0x3E,0x5F);//write 0x465F address
    Dev_BQ27Z561R2_WriteRegBits(0x3F,0x46);
    vTaskDelay(1);
    Dev_BQ27Z561R2_ReadRegBits(0x3E,data,3);*/
}

/*************************************************************************************************
 * @brief    : reset ic wake time to 1s
 * @return   : null
*************************************************************************************************/
void Dev_BQ27Z561R2_resetwakechecktime(void)
{
    //write data
    Dev_BQ27Z561R2_WriteRegBits(0x3E,0x5F); //write 0x465F address
    Dev_BQ27Z561R2_WriteRegBits(0x3F,0x46);

    Dev_BQ27Z561R2_WriteRegBits(0x40,0x01); //write data 0X01 to 0x40 address

    Dev_BQ27Z561R2_WriteRegBits(0x60,0x59);//crc= 0xff - write 0x3e start data(address + data values)(0xff-0x5F-0x46-0x01=0x46)
    Dev_BQ27Z561R2_WriteRegBits(0x61,0x05);//data length = write 0x3e start data length + write 0x60 start data length(0x5F-0x46-0x15-0x46-0x05)

    //read data
    /*Dev_BQ27Z561R2_WriteRegBits(0x3E,0x5F);//write 0x46address
    Dev_BQ27Z561R2_WriteRegBits(0x3F,0x46);
    Dev_BQ27Z561R2_ReadRegBits(0x3E,data,3);*/
}


/*************************************************************************************************
 * @brief    : get gas soc  // 0x08/0x09->voltage,0x0C/0x0D ->current
 * @return   : soc
*************************************************************************************************/
uint16_t dev_bq27z561r2_getGasSoc(void)
{
    uint16_t temp_soc;
    uint8_t socbuf[2]={0};

    /*get cli set physis value*/
    if(dev_get_phy_val_pos() & (1<<GAS_SOC_E))
    {
        temp_soc = dev_get_phy_val(GAS_SOC_E);
    }
    else
    {
        Dev_BQ27Z561R2_ReadRegBits(0x2c,socbuf,2);
        temp_soc = socbuf[1]*0xFF + socbuf[0];
    }
    return temp_soc;
}

uint16_t Dev_BQ27Z561R2_GetDOD(void)  //0x0074
{
     uint8_t data[20]={0};
     uint16_t values=0;

     Dev_BQ27Z561R2_WriteRegBits(0x3E,0x74);
     Dev_BQ27Z561R2_WriteRegBits(0x3F,0x00);
     vTaskDelay(1);
     Dev_BQ27Z561R2_ReadRegBits(0x40,data,20);

     //LOGD("data[12] = %d , data[13] = %d", data[12],data[13]);
     values = data[13]*0xFF + data[12];
     return values;
}
/***************************************************************************************
 * @brief    : get  Current  // 0x08/0x09->voltage,0x0C/0x0D ->current
 * @return   : Current
****************************************************************************************/
uint16_t Dev_BQ27Z561R2_GetCurrent(void)
{
    uint16_t temp_soc;
    uint8_t socbuf[2]={0};

    Dev_BQ27Z561R2_ReadRegBits(0x0C,socbuf,2);
    temp_soc=socbuf[1]<<8;
    temp_soc+=socbuf[0];
    return temp_soc;
}

/***************************************************************************************
 * @brief    : update Cell Gain
 * @return   : Current
****************************************************************************************/
void Dev_BQ27Z561R2_UpdateCeGain(void)  //0x4000->12135
{
    Dev_BQ27Z561R2_WriteRegBits(0x3E,0x00); //write 0x4000 address
    Dev_BQ27Z561R2_WriteRegBits(0x3F,0x40);

    Dev_BQ27Z561R2_WriteRegBits(0x40,0x67);//write data
    Dev_BQ27Z561R2_WriteRegBits(0x41,0x2F);

    Dev_BQ27Z561R2_WriteRegBits(0x60,0x29);//write crc
    Dev_BQ27Z561R2_WriteRegBits(0x61,0x06);//write data length
}
/***************************************************************************************
 * @brief    : get  Current  // 0x08/0x09->voltage,0x0C/0x0D ->current
 * @return   : Current
****************************************************************************************/
void Dev_BQ27Z561R2_UpdateCCGain(void)//0x4000->1.98
{
    Dev_BQ27Z561R2_WriteRegBits(0x3E,0x06); //write 0x4006 address
    Dev_BQ27Z561R2_WriteRegBits(0x3F,0x40);

    Dev_BQ27Z561R2_WriteRegBits(0x40,0x25);//write data
    Dev_BQ27Z561R2_WriteRegBits(0x41,0xE6);
    Dev_BQ27Z561R2_WriteRegBits(0x42,0xED);
    Dev_BQ27Z561R2_WriteRegBits(0x43,0x3F);

    Dev_BQ27Z561R2_WriteRegBits(0x60,0x82);//write crc
    Dev_BQ27Z561R2_WriteRegBits(0x61,0x08);//write data length
}
/***************************************************************************************
 * @brief    : get  Current  // 0x08/0x09->voltage,0x0C/0x0D ->current
 * @return   : Current
****************************************************************************************/
uint16_t Dev_BQ27Z561R2_GetVoltage(void)
{
    uint16_t temp_soc;
    uint8_t socbuf[2]={0};

    Dev_BQ27Z561R2_ReadRegBits(0x08,socbuf,2);
    temp_soc=socbuf[1]*256;
    temp_soc+=socbuf[0];
    return temp_soc;
}
/*******************************************************************************
* @brief:unseal IC enter unseal mode
* @param:void
*******************************************************************************/
void Dev_BQ27Z561R2_UnSeal(void)
{
    uint8_t unseal_buf1[2]={0x14,0x04};//0x00
    uint8_t unseal_buf2[2]={0x72,0x36};//0x00;
    uint8_t buf[2]={0xFF,0xFF};//0x00

    Dev_BQ27Z561R2_WriteRegBits(0x00,unseal_buf1[0]);
    hwi_delay_ms(1);
    Dev_BQ27Z561R2_WriteRegBits(0x01,unseal_buf1[1]);
    hwi_delay_ms(1);
    Dev_BQ27Z561R2_WriteRegBits(0x00,unseal_buf2[0]);
    hwi_delay_ms(1);
    Dev_BQ27Z561R2_WriteRegBits(0x01,unseal_buf2[1]);
    hwi_delay_ms(1);
    LOGD("gauge unseal");

}
/*******************************************************************************
* @brief:seal IC enter seal mode
* @param:void
*******************************************************************************/
void Dev_BQ27Z561R2_Sealed(void)
{
    uint8_t seal_buf1[2]={0x30,0x00};//0x00
    Dev_BQ27Z561R2_WriteRegBits(0x00,seal_buf1[0]);
    hwi_delay_ms(1);
    Dev_BQ27Z561R2_WriteRegBits(0x01,seal_buf1[1]);
    hwi_delay_ms(1);
    LOGD("gauge sealed");
}
/*******************************************************************************
* @brief:seal IC enter ROM mode
* @param:void
*******************************************************************************/
void Dev_BQ27Z561R2_Goto_RomMode(void)
{
    uint8_t buf[2]={0x00,0x0F};//0x00
    //uint8_t buf[2]={0x33,0x00};//0x00
    Dev_BQ27Z561R2_WriteRegBits(0x00,buf[0]);
    hwi_delay_ms(1);
    Dev_BQ27Z561R2_WriteRegBits(0x01,buf[1]);
    hwi_delay_ms(1);
    hwi_I2C_ReInit(1);
}

/*******************************************************************************
* @brief:show IC control status
* @param:void
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_GetOperationStatus(void) //0x0054
{
    uint8_t buf[2]={0x54,0x00};//
    uint8_t data[4]={0};
    uint8_t temp[1]={0};
    Dev_BQ27Z561R2_WriteRegBits(0x3E,buf[0]);
    hwi_delay_ms(1);
    Dev_BQ27Z561R2_WriteRegBits(0x3F,buf[1]);
    hwi_delay_ms(1);
    Dev_BQ27Z561R2_ReadRegBits(0x40,data,4);

    LOGD("gauge OPStatusA data[1]=%x",data[1]);
    temp[0]=data[1]&0x03;
    return temp[0];
}

/*******************************************************************************
* @brief:Verify Existing Firmware Version
* @param:void
*******************************************************************************/
uint8_t Dev_BQ27Z561R2_VerifyFWVerion(void)
{
    uint8_t ret=0;
    uint8_t write_buf[2]={0x02,0x00};//3E
    uint8_t cp_buf[6]={0x02,0x00,0x15,0x61,0x02,0x01};//3E

    Dev_BQ27Z561R2_WriteRegBits(0x3E,write_buf[0]);
    Dev_BQ27Z561R2_WriteRegBits(0x3F,write_buf[1]);

    ret=Dev_BQ27Z561R2_ReadAndCompare(0x3E,cp_buf,databuf,sizeof(cp_buf));
    memset(databuf,0,20);

    return ret;
}

/*******************************************************************************
* @brief:return to FIREWARE MODE
* @param:void
*******************************************************************************/
void Dev_BQ27Z561R2_Goto_FWMode(void)
{
    uint8_t data[1]={0X11};
    Dev_BQ27Z561R2_WriteROMBits(0x08,&data[0]);
    hwi_I2C_Init(1);
}

void Dev_BQ27Z561R2_full_access(void)
{
    uint8_t data=0xFF;//0x00
    for(uint8_t i=0;i<2;i++){
        Dev_BQ27Z561R2_WriteRegBits(0x00,data);
        hwi_delay_ms(1);
        Dev_BQ27Z561R2_WriteRegBits(0x01,data);
        hwi_delay_ms(1);
    }
}

uint8_t Dev_BQ27Z561R2_Open_Access(void)
{
    uint8_t data,config_error_cnt;
    config_error_cnt = 10;
    do{
        Dev_BQ27Z561R2_UnSeal();
        hwi_delay_ms(10);
        Dev_BQ27Z561R2_full_access();
        data = (Dev_BQ27Z561R2_GetOperationStatus()==0x01);
        config_error_cnt--;

        if(config_error_cnt == 0)
        {
           LOGD("gauge open timer out");
           break;
        }
    }while(!data);
        if(config_error_cnt == 0)
        {
           return ERROR;
        }else{
           return SUCCESS;
        }
}
void Delay_1000ms(void)
{
    hwi_delay_ms(500);
#ifndef DISABLE_WDG
    FeedIwdg();
#endif
    hwi_delay_ms(500);
#ifndef DISABLE_WDG
    FeedIwdg();
#endif
}
/*******************************************************************************
* @brief:Verify Existing Firmware Version
* @param:void
*******************************************************************************/
void Dev_BQ27Z561R2_UpdateDataBlock(BATTERY_TYPE_E batType)
{
    uint8_t ret = 0;
    uint8_t test_data[2];
    uint8_t config_error_cnt = 5;

    /*Verify Existing Firmware Version*/
    ret = Dev_BQ27Z561R2_VerifyFWVerion();
    if(ret)
    {
        LOGD("FW ok");
    }else{
        LOGD("FW fail");
    }

    ret = Dev_BQ27Z561R2_Open_Access();
    if(ret)
    {
        LOGD("gauge open success");
    }else{
        LOGD("gauge open fail");
    }
    Delay_1000ms();

    /*Dev_BQ27Z561R2_Goto_RomMode*/
    Dev_BQ27Z561R2_Goto_RomMode();
    Delay_1000ms();

    /*write Data Block*/
    if(BATTERY_GP == batType)
    {
        do{
            for(uint16_t index = 0 ;index < (uint16_t)ROM_DATA_ROW; index++){
                if(index==0){
                    ret = Dev_BQ27Z561R2_WriteROM(0X11,rom_data1[0],2);
                    hwi_delay_ms(200);
                }else{
                    ret = Dev_BQ27Z561R2_WriteROM(0X0F,rom_data1[index],ROM_DATA_COLUMN);
                    hwi_delay_ms(1);
                }
                if(ret)
                {
                    LOGD("write gp error i=%d",index);
                    break;
                }
            }
            ret=Dev_BQ27Z561R2_ROM_ReadAndCompare(0x14,crc_buf1,databuf,sizeof(crc_buf1));//checksum
            memset(databuf,0,20);
            config_error_cnt--;
            #ifndef DISABLE_WDG
            FeedIwdg();
            #endif
            if(config_error_cnt == 0)
            {
                LOGD("gp config timer out");
                break;
            }
        }while(!ret);
    }
    else
    {
        do{
            for(uint16_t index = 0 ;index < (uint16_t)ROM_DATA_ROW; index++){
                if(index==0){
                    ret = Dev_BQ27Z561R2_WriteROM(0X11,rom_data2[0],2);
                    hwi_delay_ms(200);
                }else{
                    ret = Dev_BQ27Z561R2_WriteROM(0X0F,rom_data2[index],ROM_DATA_COLUMN);
                    hwi_delay_ms(1);
                }
                if(ret)
                {
                    LOGD("write byd error i=%d",index);
                    break;
                }
            }
            ret=Dev_BQ27Z561R2_ROM_ReadAndCompare(0x14,crc_buf2,databuf,sizeof(crc_buf2));//checksum
            memset(databuf,0,20);
            config_error_cnt--;
            #ifndef DISABLE_WDG
            FeedIwdg();
            #endif
            if(config_error_cnt == 0)
            {
                LOGD("byd config timer out");
                break;
            }
        }while(!ret);
    }

    if(ret)
    {
        flash_record_t * frt = get_self_flash_record_from_ram();
        if(BATTERY_GP == batType)
        {
            frt->gauge_version = GAUGE_GP_DATA_VERSION;
        }
        else
        {
            frt->gauge_version = GAUGE_BYD_DATA_VERSION;
        }
        LOGD("%d ROM write success",frt->gauge_version);
        update_data_flash(USR_DATA,INVALID);
    }else{
        LOGD("ROM write fail");
    }

    /*Execute Flash Code*/
    Dev_BQ27Z561R2_Goto_FWMode();
    Delay_1000ms();

    /*seal device*/

}

/***************************************************************************************
 * @brief    : set flash data
 * @return   : Current
****************************************************************************************/
void Dev_BQ27Z561R2_SetDataFlash(uint16_t address,uint16_t values)   //0x4719+4=0X471D  4250=0X109A
{
    uint8_t ads[2]={0};
    uint8_t write_value[2]={0};
    uint8_t crc_lenth[2]={0};
    uint8_t crc=0;
    uint8_t data[4]={0};

    ads[0] = 0xff&address;
    ads[1] = 0xFF&address>>8;
    write_value[0]=values&0xFF;
    write_value[1]=values>>8 &0xFF;
    crc=(0xFF-ads[0]-ads[1]-write_value[0]-write_value[1])&0XFF;
    crc_lenth[0]=crc;
    crc_lenth[1]=0x06;

    Dev_BQ27Z561R2_WriteReg(0x3E,ads,2);//write address
    vTaskDelay(1);
    Dev_BQ27Z561R2_WriteReg(0x40,write_value,2);//write values
    vTaskDelay(1);
    Dev_BQ27Z561R2_WriteReg(0x60,crc_lenth,2);//write crc and length
    vTaskDelay(1);
}

/***************************************************************************************
 * @brief    : read flash data
 * @return   : Current
****************************************************************************************/
uint16_t Dev_BQ27Z561R2_ReadDataFlash(uint16_t address)
{
    uint8_t ads[2]={0};
    uint8_t data[4]={0};
    uint16_t values=0;

    ads[0] = 0xFF&address;
    ads[1] = 0xFF&address>>8;
    Dev_BQ27Z561R2_WriteReg(0x3E,ads,2);
    vTaskDelay(1);
    Dev_BQ27Z561R2_ReadRegBits(0x3E,data,4);

    values= data[3]<<8|data[2];
    LOGD("read data address=0x%x,values=0x%x",address,values);
    return values;
}

#define GAUGE_CONFIG_NUM 14

typedef struct
{
    uint16_t address;
    uint16_t value;
}GAUGE_CONFIG_INFO_T;

GAUGE_CONFIG_INFO_T intDataInfo[GAUGE_CONFIG_NUM] ={
    {0x464B, 0},
    {0x464D, 0},
    {0x4382, 0},
    {0x4384, 0},
    {0x4386, 0},
    {0x438A, 0},
    {0x4677, 0},
    {0x4679, 0},
    {0x4683, 0},
    {0x4685, 0},
    {0x418D, 0},
    {0x418F, 0},
    {0x471D, 0},
    {0x4717, 0},
};

void Dev_BQ27Z561_UpdateFlashData(uint8_t step)
{
    flash_record_t * pfrt = get_self_flash_record_from_ram();

    if(step ==STEP_1)
    {
        intDataInfo[0].value=pfrt->step1_chg_volt+50;//0x1132
        intDataInfo[1].value=pfrt->step1_chg_volt;//0x1100
        intDataInfo[2].value=pfrt->step1_chg_volt;
        intDataInfo[3].value=pfrt->step1_chg_volt;
        intDataInfo[4].value=pfrt->step1_chg_volt;
        intDataInfo[5].value=pfrt->step1_chg_volt;
        intDataInfo[6].value=pfrt->step1_chg_volt;
        intDataInfo[7].value=pfrt->step1_chg_volt-100;//0x109c
        intDataInfo[8].value=pfrt->step1_chg_volt;
        intDataInfo[9].value=pfrt->step1_chg_volt-100;
        intDataInfo[10].value=pfrt->step1_chg_volt;
        intDataInfo[11].value=GAUGE_TERM_CURENT;
        intDataInfo[12].value=pfrt->step1_chg_volt;
        intDataInfo[13].value=GAUGE_TERM_CURENT;
    }else if(step ==STEP_2)
    {
        intDataInfo[0].value=pfrt->step2_chg_volt+50;//0x1102
        intDataInfo[1].value=pfrt->step2_chg_volt;//0x10D0
        intDataInfo[2].value=pfrt->step2_chg_volt;
        intDataInfo[3].value=pfrt->step2_chg_volt;
        intDataInfo[4].value=pfrt->step2_chg_volt;
        intDataInfo[5].value=pfrt->step2_chg_volt;
        intDataInfo[6].value=pfrt->step2_chg_volt;
        intDataInfo[7].value=pfrt->step2_chg_volt-100;//0x106c
        intDataInfo[8].value=pfrt->step2_chg_volt;
        intDataInfo[9].value=pfrt->step2_chg_volt-100;
        intDataInfo[10].value=pfrt->step2_chg_volt;
        intDataInfo[11].value=GAUGE_TERM_CURENT;
        intDataInfo[12].value=pfrt->step2_chg_volt;
        intDataInfo[13].value=GAUGE_TERM_CURENT;
    }else if(step ==STEP_3)
    {
        intDataInfo[0].value=pfrt->step3_chg_volt+50;//0x10D2
        intDataInfo[1].value=pfrt->step3_chg_volt;//0x10A0
        intDataInfo[2].value=pfrt->step3_chg_volt;
        intDataInfo[3].value=pfrt->step3_chg_volt;
        intDataInfo[4].value=pfrt->step3_chg_volt;
        intDataInfo[5].value=pfrt->step3_chg_volt;
        intDataInfo[6].value=pfrt->step3_chg_volt;
        intDataInfo[7].value=pfrt->step3_chg_volt-100;//0x103c
        intDataInfo[8].value=pfrt->step3_chg_volt;
        intDataInfo[9].value=pfrt->step3_chg_volt-100;
        intDataInfo[10].value=pfrt->step3_chg_volt;
        intDataInfo[11].value=GAUGE_TERM_CURENT;
        intDataInfo[12].value=pfrt->step3_chg_volt;
        intDataInfo[13].value=GAUGE_TERM_CURENT;
    }else if(step ==STEP_4)
    {
        intDataInfo[0].value=pfrt->step4_chg_volt+50;//0x10A2
        intDataInfo[1].value=pfrt->step4_chg_volt;//0x1070
        intDataInfo[2].value=pfrt->step4_chg_volt;
        intDataInfo[3].value=pfrt->step4_chg_volt;
        intDataInfo[4].value=pfrt->step4_chg_volt;
        intDataInfo[5].value=pfrt->step4_chg_volt;
        intDataInfo[6].value=pfrt->step4_chg_volt;
        intDataInfo[7].value=pfrt->step4_chg_volt-100;//0x100c
        intDataInfo[8].value=pfrt->step4_chg_volt;
        intDataInfo[9].value=pfrt->step4_chg_volt-100;
        intDataInfo[10].value=pfrt->step4_chg_volt;
        intDataInfo[11].value=GAUGE_TERM_CURENT;
        intDataInfo[12].value=pfrt->step4_chg_volt;
        intDataInfo[13].value=GAUGE_TERM_CURENT;
    }else
    {

    }
}

void Dev_BQ27Z561_CheckFlashData(uint8_t step)
{
    uint8_t i = 0;
    uint16_t data = 0;
    uint8_t error = 0;

    Dev_BQ27Z561_UpdateFlashData(step);
    if(step!=STEP_0)
    {
        for (i = 0; i < (sizeof(intDataInfo) / sizeof(GAUGE_CONFIG_INFO_T)); i++)
        {
            data=Dev_BQ27Z561R2_ReadDataFlash(intDataInfo[i].address);
            if (data != intDataInfo[i].value)
            {
                error = 1;
                LOGE("step%d error:address=0x%x, value=0x%x, data=0x%x\r\n", step, intDataInfo[i].address, intDataInfo[i].value, data);
                Dev_BQ27Z561R2_SetDataFlash(intDataInfo[i].address, intDataInfo[i].value);
            }
        }
        if (1 == error)
        {
            for (i = 0; i < (sizeof(intDataInfo) / sizeof(GAUGE_CONFIG_INFO_T)); i++)
            {
                data= Dev_BQ27Z561R2_ReadDataFlash(intDataInfo[i].address);
                LOGE("step%d read :address=%x, value=0x%x, data=0x%x\r\n",step, intDataInfo[i].address, intDataInfo[i].value, data);
            }
        }
    }
}

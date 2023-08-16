#include  "dev_pd_sy69410vws.h"
#include  "log.h"
#include "i2c.h"
#include "HWI_i2c.h"

static  uint8_t      tx_buf[40];
static  uint8_t      rx_buf[40];
static  PD_MSG_t     new_rx_message;
static  PD_MSG_t     *pd_rx_info;


#define  PD_SY69410  (0x4E)
#define  I2C_READ_ERROR             0x11
#define  I2C_WRITE_ERROR            0x12
#define  ADC_NOT_ENABLED            0x13

uint8_t I2CWriteBytes(uint8_t SlaveAddress, uint8_t MemoryAdress, uint8_t wrNumber, uint8_t* wrPointer){
    uint8_t ret = 0;

    ret = hwi_I2C_Mem_Write(1,SlaveAddress,MemoryAdress,I2C_MEMADD_SIZE_8BIT,wrPointer,wrNumber,0xfff);

    if (ret)
    {
        return I2C_WRITE_ERROR;
    }
    return ret;
}

uint8_t I2CReadBytes(uint8_t SlaveAddress, uint8_t MemoryAdress, uint8_t rdNumber, uint8_t* rdPointer){
    uint8_t ret = 0;

  ret = hwi_I2C_Mem_Read(1,SlaveAddress,MemoryAdress,I2C_MEMADD_SIZE_8BIT,rdPointer,rdNumber,0xfff);

    if (ret)
    {
        return I2C_READ_ERROR;
    }
    return ret;

}

void Set1Reg(uint8_t memory_addr, uint8_t value){
    uint8_t ret = 0;

    ret = hwi_I2C_Mem_Write(1,PD_SY69410,memory_addr,I2C_MEMADD_SIZE_8BIT,&value,1,0xfff);
    if (ret)
    {
       // return I2C_WRITE_ERROR;
    }
}

uint8_t Get1Reg(uint8_t memory_addr){
    uint8_t ret = 0;
    uint8_t tmp = 0;

    ret = hwi_I2C_Mem_Read(1,PD_SY69410,memory_addr,I2C_MEMADD_SIZE_8BIT,&tmp,1,0xfff);

    if (ret)
    {
        return I2C_READ_ERROR;
    }

   return tmp;
}

void Set2Reg(uint8_t memory_addr, uint16_t value){
    uint8_t ret = 0;
    uint8_t write_value[2]={0,0};

    write_value[0]= (uint8_t)(value&0x00ff);
    write_value[1] = (uint8_t)(value>>8);

    ret = hwi_I2C_Mem_Write(1,PD_SY69410,memory_addr,I2C_MEMADD_SIZE_8BIT,write_value,2,0xfff);

    if (ret)
    {
       //return I2C_WRITE_ERROR;
    }

}

uint16_t Get2Reg(uint8_t memory_addr){
    uint8_t ret = 0;
    uint8_t values[2]={0,0};
    uint16_t readvalue = 0;

    ret = hwi_I2C_Mem_Read(1,PD_SY69410,memory_addr,I2C_MEMADD_SIZE_8BIT,values,2,0xfff);
    if (ret)
    {
        return I2C_READ_ERROR;
    }
    readvalue = values[1]<<8 | values[0];

    return readvalue;

}


void Command_Write(COMMAND_TYPE_e command)
{
    Set1Reg(0x23, command);
}

void CC_Role_Config(CC_ROLE_CONFIG_t cc_role_config)
{
    uint8_t temp = cc_role_config.byte;
    Set1Reg(0x1A, temp);
}

void CC_Role_Force(DRP_EN_e a, RP_VALUE_TYPE_e b, CC_TYPE_e c2,CC_TYPE_e c1)
{
    uint8_t temp = (a<<6)|(b<<4)|(c1<<0)|(c2<<2);
    Set1Reg(0x1A, temp);
}


void Hard_Reset_Send(void)
{
    Set1Reg(0x50, 0x05); // start transmit HardReset, Retry 0 times
}

void BIST_Carrier_Mode_Send(void)
{
    Set1Reg(0x50, 0x07); // Enter  BIST carrier mode for 45ms
}


void BIST_TEST_DATA_Mode_Enter(void)
{
    uint8_t temp =Get1Reg(0x19);
    Set1Reg(0x19, temp|(0x02)); // Enter  BIST Test data mode
}
void BIST_TEST_DATA_Mode_Quit(void)
{

    uint8_t temp =Get1Reg(0x19);
    Set1Reg(0x19, temp&(~0x02)); // Quit BIST Test data mode

}

void  PD_Receive_Detect_Config(RECEIVE_DETECE_t temp)
{
    Set1Reg(0X2F, temp.byte);
}

void PD_Msg_Send(PD_MSG_t tmsg)
{
    uint32_t i;
    uint8_t temp;
    uint8_t WRITE_BYTE_COUNT;
    //before send message,clear the last message sending status
    //PD_Transmit_Status_Get();it is necessary but not here
    WRITE_BYTE_COUNT = 2 + (tmsg.header.number_Data_Objects<< 2);//Header:2 Bytes+Data object( num_data_objects*4 )= RITE_BYTE_COUNT=2+4*n
    tx_buf[0] = WRITE_BYTE_COUNT;
    tx_buf[1] = tmsg.header.w&(0xFF);
    tx_buf[2] = (tmsg.header.w>>8)&(0xFF);//Message header part
    temp = 3;
    for( i = 0; i < tmsg.header.number_Data_Objects; i++)
    {
        tx_buf[temp] =     tmsg.data[i]&(0XFF);
        tx_buf[temp + 1] = (tmsg.data[i]>>8)&(0XFF);
        tx_buf[temp + 2] = (tmsg.data[i]>>16)&(0XFF);
        tx_buf[temp + 3] = (tmsg.data[i]>>24)&(0XFF);
        temp += 4;
    }//Message Data part
     I2CWriteBytes(TCPC_Address, TX_BUF_ADDRESS, WRITE_BYTE_COUNT + 1, tx_buf); // write Tx_buffer

    if(tmsg.Sop_Type!=TYPE_SOP)
          Set1Reg(0x50, 0x00|tmsg.Sop_Type); //No retry,start transmit
    else if(tmsg.header.spec_Rev == REV_3)
        Set1Reg(0x50, 0x20|tmsg.Sop_Type); //retry 2 times,start transmit
    else
        Set1Reg(0x50, 0x30|tmsg.Sop_Type); //retry 3 times,start transmit
    //LOGD("\r\n\nTx->ID:%d,Rev:%d,P:%d,D:%d", tmsg.header.messageID,tmsg.header.spec_Rev+1,tmsg.header.port_Power_Role,tmsg.header.port_Data_Role);
    LOGD("Tx->ID:%d", tmsg.header.messageID);
}

uint8_t PD_Receive(void)
{
    uint8_t  i,num;
    uint16_t READABLE_COUNT;
    pd_rx_info= &new_rx_message;
    I2CReadBytes(TCPC_Address, RX_BUF_ADDRESS, 2, rx_buf);
    if(rx_buf[0] > 1)
    {
        READABLE_COUNT = rx_buf[0];
        pd_rx_info->Sop_Type=(SOP_TYPE_e)rx_buf[1];
        I2CReadBytes(TCPC_Address, RX_BUF_ADDRESS, READABLE_COUNT + 1, rx_buf); //读取BUffer内的全部内容
        //Note:
        //rx_buf[0]:READABLE_BYTE_COUNT
        //rx_buf[1]:RX_BUF_FRAME_TYPE
        //rx_buf[2]:Header_L   rx_buf[3]:Header_H
        //rx_buf[4-7]:Data1     rx_buf[8-11]:Data2      rx_buf[12]~rx_buf[15]: Date3   rx_buf[16]~rx_buf[19]: Date4
        pd_rx_info->header.w=(uint32_t)(rx_buf[2]|(rx_buf[3]<<8));
        num=pd_rx_info->header.number_Data_Objects;
        for(i=0;i<num;i++)
        {
            pd_rx_info->data[i]=(rx_buf[i*4+7]<<24)|(rx_buf[i*4+6]<<16)|(rx_buf[i*4+5]<<8)|rx_buf[i*4+4];
        }
        if(num==0)
            //LOGD("\r\nRx<-ID:%d,Rev:%d,P:%d,D:%d,ctr:%d \r\n",pd_rx_info->header.messageID,pd_rx_info->header.spec_Rev+1,pd_rx_info->header.port_Power_Role,pd_rx_info->header.port_Data_Role,pd_rx_info->header.message_Type);
            LOGD("\r\nRx<-ID:%d",pd_rx_info->header.messageID);
        else
            //LOGD("\r\nRx<-ID:%d,Rev:%d,P:%d,D:%d,data:%d \r\n",pd_rx_info->header.messageID,pd_rx_info->header.spec_Rev+1,pd_rx_info->header.port_Power_Role,pd_rx_info->header.port_Data_Role,pd_rx_info->header.message_Type);
            LOGD("\r\nRx<-ID:%d",pd_rx_info->header.messageID);
         return TRUE;
    }
    return FALSE;
}

void   PD_Goodcrc_Header_Init(CABLE_PLUG_TYPE_e cable_plug_type,DATA_ROLE_TYPE_e data_role_type,POWER_ROLE_TYPE_e pwr_role_type,SPEC_REV_TYPE_e spec_rev_type)
{

        uint8_t temp;
        temp=pwr_role_type|(spec_rev_type<<1)|(data_role_type<<3)|(cable_plug_type<<4);
        Set1Reg(0x2E,temp);
}



void PD_Receive_Enable(uint8_t temp)//auto send GoodCRC  enable
{
     uint8_t reg=Get1Reg(0X2F);
     reg=temp;
     Set1Reg(0x2F, reg);
}

void Hard_Reset_Disable(void)
{

    uint8_t temp;
    temp = Get1Reg(0x2F);
    temp = temp & (~0x20);
    Set1Reg(0x2F, temp);

}

void Hard_Reset_Enable(void)
{

    uint8_t temp;
    temp = Get1Reg(0x2F);
    temp = temp | (0x20);
    Set1Reg(0x2F, temp);

}




PD_MSG_t*  PD_Msg_Get(void)
{
    return pd_rx_info;
}


void CC_DRP_Period_Set(DRP_PERIOD_e period)
{
    Set1Reg(0xA2, period);
}

void CC_DRP_Rp_Duty_Set(uint16_t duty)
{
    if(duty>1023)
        duty=1023;
    Set1Reg(0xA3, (uint8_t)(duty&(0xFF)));
    Set1Reg(0xA4, (uint8_t)(duty>>8));
}

void Vcon_Init()
{
    Vcon_Off();
    Vcon_Ocp_Set(Curr_300mA);
}

void Vcon_On(void)
{
     Set1Reg(0x1C, 0X01);
    // pd_status.VCONN_ON=1;
}

void Vcon_Off(void)
{
    Set1Reg(0x1C, 0X00);
    // pd_status.VCONN_ON=0;
}

void Vcon_Discharge_On(void)
{
    uint8_t temp;
    temp=Get1Reg(0x90);
    Set1Reg(0x90,temp|0x20);
}

void Vcon_Discharge_Off(void)
{
    uint8_t temp;
    temp=Get1Reg(0x90);
    Set1Reg(0x90,temp&(~0x20));
}

void Vcon_Ocp_Set(VCON_OCP_e value)
{
    Set1Reg(0x93, value<<5);
}

void Vender_Power_Config(VENDER_PWR_t pwr_config)
{
    Set1Reg(0X90,pwr_config.byte);
}


VENDER_PWR_t  Vender_Power_Get(void)
{
    VENDER_PWR_t pwr_config;
    uint8_t temp=Get1Reg(0X90);
    pwr_config.byte=temp;
    return pwr_config;
}

void Vender_Reset(void)
{
    Set1Reg(0xA0, 0X01);
    //To Do:delay 2ms
}

void Vender_Wakeup_Enable(void)
{
    Set1Reg(0x9F, 0X80);
}

void Vender_Wakeup_Disable(void)
{
    Set1Reg(0x9F, 0X00);
}

void Vender_Auto_Idle_Enable(uint8_t time_out_value)
{
    uint8_t temp;
    temp=Get1Reg(0x9B);
    Set1Reg(0x9B,temp|0X08|(time_out_value&0X07));
}

void Vender_Auto_Idle_Diable(void)
{
    uint8_t temp;
    temp=Get1Reg(0x9B);
    Set1Reg(0x9B,temp&(~0X08));
}

void Vender_PDIC_Shutdown_Quit(void)
{
    uint8_t temp;
    temp=Get1Reg(0x9B);
    Set1Reg(0x9B,temp|(0X20));
}

void Vender_PDIC_Shutdown_Enter(void)
{
    uint8_t temp;
    temp=Get1Reg(0x9B);
    Set1Reg(0x9B,temp&(~0X20));
}


void Vcon_Oc_Detect_Disable(void)
{
    uint8_t temp;
    temp=Get1Reg(0x1B);
    Set1Reg(0x1B,temp|0X01);
}

void Vcon_Oc_Detect_Enable(void)
{
    uint8_t temp;
    temp=Get1Reg(0x1B);
    Set1Reg(0x1B,temp&(~0X01));
}

void Vcon_Ov_Detect_Disable(void)
{
    uint8_t temp;
    temp=Get1Reg(0x1B);
    Set1Reg(0x1B,temp|0X80);
}

void Vcon_Ov_Detect_Enable(void)
{
    uint8_t temp;
    temp=Get1Reg(0x1B);
    Set1Reg(0x1B,temp&(~0X80));
}

//接口层
void CC_Plug_Set(uint8_t cc)
{

    uint8_t temp;
    temp=Get1Reg(0x19);
    if(cc==1)
      temp=temp&(~01);  //Choose CC1 to PD communication
    else if(cc==2)
        temp=temp|(01);
    Set1Reg(0x19,temp); //Choose CC2 to PD communication
}


uint8_t  CC_status_Get(void)
{
    uint8_t temp=Get2Reg(0x1D);
    return temp;
}

uint8_t  Power_status_Get(void)
{
    uint8_t temp=Get2Reg(0x1E);
    return temp;
}

uint8_t  Fault_status_Get(void)
{
    uint8_t temp=Get2Reg(0x1F);
    return temp;
}

uint16_t Alert_status_Get(void)
{
    uint16_t temp=Get2Reg(0x10);
    return temp;
}

void   Alert_Clear(uint16_t temp)
{
    Set2Reg(0x10,temp);
}


void SY69410_Init(void)
{
    VENDER_PWR_t temp;
    Vender_Reset();
    hwi_delay_ms(1);
    Vender_PDIC_Shutdown_Quit();

    hwi_delay_ms(1);
    temp.BG_EN=1;
    temp.OSC_EN=1;
    temp.VCON_DISCHARGEEN=0;
    temp.LPR_EN=0;
    temp.LPR_RPD=0;
    temp.VBUS_DETECTEN=1;

    Vender_Power_Config(temp);
    Vender_Auto_Idle_Enable(PERIOD_102_4ms);
    Command_Write(ENABLE_VBUS_DETECT);

    Vender_Auto_Idle_Diable();
    //App_Int_Fun();//Clear INT

    //PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, REV_3);

}




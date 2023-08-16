#include     "app_pd_com.h"
#include     "app_pd_typec.h"
#include     "app_pd_cfg.h"
#include     "dev_pd_sy69410vws.h"
#include     "HWI_Hal.h"
#include     "log.h"
#include     <string.h>

TCPC_STATUS_Type     tcpc_status;
pd_status_t          pd_status;

static  uint8_t      pd_rx_count;
static  tx_result_t  pd_tx_result;

#define SNK_ONLY  0x01
#define SRC_ONLY  0x02
#define DRP_ONLY  0x03

bool IsVBusOK(void)
{
    if((pd_status.POWER_STATUS_REG&0x04)!=0)
        return TRUE;
    else
        return FALSE;
}

void App_Int_Fun(void)
{
    uint16_t ALERT_REG;
    uint8_t  REG_1FH;
    uint8_t  CC_STATUS_REG;
    ALERT_REG = Get2Reg(0x10); //Alert address 10h-11h

    if(ALERT_REG!=0)
    {
        if((ALERT_REG & ALERT_CC_STATUS) == ALERT_CC_STATUS)
        {
            CC_STATUS_REG = Get1Reg(0x1D); //CC STASUS address 1D
            pd_status.Looking4Connection = (CC_STATUS_REG & (0x20)) >> 5;
            pd_status.ConnectResult = (CC_STATUS_REG & (0x10)) >> 4;
            pd_status.CC2_State = (CC_STATUS_REG & (0x0C)) >> 2;
            pd_status.CC1_State = (CC_STATUS_REG & (0x03));
        }

        if((ALERT_REG & ALERT_POWER_STATUS) == ALERT_POWER_STATUS)
        {
            pd_status.POWER_STATUS_REG = Get1Reg(0x1E); //Power STASUS address 1E
            pd_status.vconn_present=(pd_status.POWER_STATUS_REG>>1)&0x01;
        }

        if((ALERT_REG & ALERT_RX_HARD_RESET) == ALERT_RX_HARD_RESET)
        {
            pd_status.R_HARD_RESET = 1;
            LOGD("HARD_RESET\r\n");
        }

        if((ALERT_REG & ALERT_RX_SOP) == ALERT_RX_SOP)
        {
            PD_Receive();
            LOGD("PD_Receive\r\n");
            // if((new_rx_message.header.messageID!=tcpc_status.rx_stored_msgID)||(new_rx_message.header.message_Type==0 && (new_rx_message.header.number_Data_Objects==0)))
            pd_rx_count++;
        }

        if((ALERT_REG & ALERT_TX_FAIL) == ALERT_TX_FAIL && (ALERT_REG & ALERT_TX_SUCCESS) == ALERT_TX_SUCCESS)
        {
            pd_tx_result.Transmit_Hard_Reset_Success = 1;
        }
        else if((ALERT_REG & ALERT_TX_FAIL) == ALERT_TX_FAIL)
        {
            pd_tx_result.Transmit_Fail = 1;
                pd_tx_result.transmit_status=TRANSMIT_FAIL;
        }
        else if((ALERT_REG & ALERT_TX_DISCARD) == ALERT_TX_DISCARD)
        {
            pd_tx_result.Transmit_Discard = 1;
                pd_tx_result.transmit_status=TRANSMIT_DISCARD;
        }
        else if((ALERT_REG & ALERT_TX_SUCCESS) == ALERT_TX_SUCCESS)
        {
            pd_tx_result.Transmit_Success = 1;
              pd_tx_result.transmit_status=TRANSMIT_SUCCESS;
        }

        if((ALERT_FAULT & ALERT_REG) == ALERT_FAULT)
        {
            REG_1FH = Get1Reg(0x1F);

            if((REG_1FH & 0x01) == 1)//i2C error
            {
                Command_Write(RESET_TX_BUF);
                pd_tx_result.Transmit_Discard = 1;
                pd_tx_result.I2C_error = 1;
            }
            if((REG_1FH & 0x02) == 0x02)//VCON_OC happened
            {
                Vcon_Off();
                //To Do:
            }
            if((REG_1FH & 0x80) == 0x80)//VCON_OV happened
            {
                Vcon_Off();
                Vcon_Discharge_Off();
                //To Do:
            }
            if((REG_1FH & 0x04) == 0x04)//CC_UV_Fault happeded
            {
                //To Do:
            }
            Set1Reg(0x1F, REG_1FH);
        }

        Set2Reg(0x10, ALERT_REG);//or  Set1Reg(0X10,0xFF); Set1Reg(0X11,0xFF);
        //hwi_delay_us(60);//it is necessary to ensure INT is cleared ;
        vTaskDelay(1);
    }
}

PDTRANS_STATUS_e  PD_Transmit_Status_Get(void)
{
    PDTRANS_STATUS_e temp=pd_tx_result.transmit_status;
    if(temp!=0)
        pd_tx_result.transmit_status=TRANSMIT_NONE;
    return temp;
}


void start_drp(void)
{
    Command_Write(LOOK_4_CONNECTION);
}

void enable_bist_data_mode(uint8_t en)
{
    if(en==0)
        BIST_TEST_DATA_Mode_Quit();
    else
        BIST_TEST_DATA_Mode_Enter();
}


void PD_Send_Control_Message(CONTROL_MESSAGE_TYPE_e type)
{
    PD_MSG_t t_mes;

    memset(t_mes.data,0,sizeof(t_mes.data));
    t_mes.header.extended=0;

    t_mes.header.message_Type=type;
    t_mes.header.number_Data_Objects=0;
    t_mes.header.port_Data_Role=tcpc_status.data_role;
    t_mes.header.port_Power_Role=tcpc_status.power_role;
    t_mes.header.spec_Rev=tcpc_status.pd_rev;
    t_mes.header.messageID=tcpc_status.messageID;
    tcpc_status.messageID=(tcpc_status.messageID+1)%7;
    t_mes.Sop_Type=TYPE_SOP;
    PD_Msg_Send(t_mes);
}

bool PD_Receive_NewMsg_Check(void)
{
    if(pd_rx_count>0)
        return TRUE;
    else
      return FALSE;
}

// interface layer
bool PD_Receive_Hard_Reset_Check(void)
{
     bool temp;
     temp=((pd_status.R_HARD_RESET==1)?TRUE:FALSE);
     if(pd_status.R_HARD_RESET==1)  pd_status.R_HARD_RESET=0;
     return temp;
}


// interface layer
void PD_Receive_NewMsg_Flag_Clear(void) // decrease 1
{
      if(pd_rx_count>0)
        pd_rx_count--;

}

// DATA_ROLE_TYPE_e POWER_ROLE_TYPE_e SPEC_REV_TYPE_e
void  Set_Power_Role(POWER_ROLE_TYPE_e temp)
{
    if(tcpc_status.power_role!=temp)
    {
         tcpc_status.power_role=temp;
         PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
    }
}

void  Set_Data_Role(DATA_ROLE_TYPE_e temp)
{
    if(tcpc_status.data_role!=temp)
    {
         tcpc_status.data_role=temp;
         PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
    }
}

void  Set_PD_Rev(SPEC_REV_TYPE_e temp)
{
    if(tcpc_status.pd_rev!=temp)
    {
        tcpc_status.pd_rev=temp;
        PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
    }
}

unsigned char Is_looking_4_connection(void)
{
    if(pd_status.Looking4Connection==1)
        return 1;
    else
        return 0;
}


pd_phy_t pd={
      .start_drp=start_drp,
      .set_cc_plug=CC_Plug_Set,              //Used to selece CC1 or CC2 to communication
      .set_cc_role=CC_Role_Force,            //Use to set CC role
      .set_drp_period=CC_DRP_Period_Set,     //Used set Dual_role_togggle period
      .set_drp_rp_duty=CC_DRP_Rp_Duty_Set,   //used to ser Rp duty cycle during one DRP period :Rp_duty=(duty+1)/1024
      .set_power_role=Set_Power_Role,
      .set_data_role=Set_Data_Role,
      .set_pd_rev=Set_PD_Rev,
      .enable_pd_receive=PD_Receive_Enable,
      .PD_Msg_Get=PD_Msg_Get,
      .enable_bist_data_mode=enable_bist_data_mode,
      .send_hard_reset=Hard_Reset_Send,
      .send_msg=PD_Msg_Send,
      .send_ctrl_msg=PD_Send_Control_Message,
      .send_bist_carry_mode=BIST_Carrier_Mode_Send,
      .is_new_msg_received=PD_Receive_NewMsg_Check,
      .is_vbus_ok=IsVBusOK,
      .clear_new_msg_received=PD_Receive_NewMsg_Flag_Clear,
      .is_hard_reset_received=PD_Receive_Hard_Reset_Check,
      .PD_Transmit_Status_Get=PD_Transmit_Status_Get,
      .vcon_on=Vcon_On,
      .vcon_off=Vcon_Off,
      .chip_init=SY69410_Init,
      .PD_Goodcrc_Header_Init=PD_Goodcrc_Header_Init,
};

void app_pd_init()
{
    App_PD_Power_Sink_Capability_Init();
    tcpc_status.drp_state=SNK_ONLY;
    pd.chip_init();
    App_Typec_Init();
}

void app_pd_process()
{
    if(HWI_PIN_RESET == hwi_GPIO_ReadPin(PD_INT_E)){
        App_Int_Fun();
    }
        App_TypeC_scan();
}


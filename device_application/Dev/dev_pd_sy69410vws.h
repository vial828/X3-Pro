#ifndef DEV_PD_SY69410VWS_H
#define DEV_PD_SY69410VWS_H


#include <stdint.h>
#include "app_pd_general.h"
#include "app_pd_com.h"

//#pragma anon_unions

//Alert bit definition
#define  ALERT_VENDER_DEFINED                          ((uint16_t)(1<<15))
#define  ALERT_RX_OVERFLOW                             ((uint16_t)(1<<10))
#define  ALERT_FAULT                                   ((uint16_t)(1<<9))
#define  ALERT_TX_SUCCESS                              ((uint16_t)(1<<6))
#define  ALERT_TX_DISCARD                              ((uint16_t)(1<<5))
#define  ALERT_TX_FAIL                                 ((uint16_t)(1<<4))
#define  ALERT_RX_HARD_RESET                           ((uint16_t)(1<<3))
#define  ALERT_RX_SOP                                  ((uint16_t)(1<<2))
#define  ALERT_POWER_STATUS                            0x0002
#define  ALERT_CC_STATUS                               0x0001

#define EN_SOP         (uint8_t)(0x01)
#define EN_SOP1        (uint8_t)(0x01<<1)
#define EN_SOP2        (uint8_t)(0x01<<2)
#define EN_SOP1_DEBUG  (uint8_t)(0x01<<3)
#define EN_SOP2_DEBUG  (uint8_t)(0x01<<4)
#define EN_HARD_RESET  (uint8_t)(0x01<<5)
#define EN_CABLE_RESET (uint8_t)(0x01<<6)

typedef struct
{
    uint32_t w;
    struct{
    uint32_t  pps_status                            :1;
    uint32_t  bist_carry_mode                       :1;
    uint32_t  not_supported                         :1;
    uint32_t  reject                                :1;
    uint8_t   src_cap                               :1;
    uint8_t   sink_cap                              :1;
    uint32_t  reserve                               :(32-6);
    }bit;
}send_flag_t;



typedef enum
{
    LOOK_4_CONNECTION = 0x99,
    DISABLE_VBUS_DETECT = 0X22,
    ENABLE_VBUS_DETECT = 0x33,
    RESET_TX_BUF= 0xDD,
    RESET_RX_BUF= 0xEE,
} COMMAND_TYPE_e;


typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t cc1_state:2;
        uint8_t cc2_state:2;
        uint8_t connect_result:1;
        uint8_t looking4connect:1;
        uint8_t reserve:2;
    };
}CC_STATUS_t;


#ifdef USE_BA41
    typedef union
    {
        uint8_t byte;
        struct
        {
            uint8_t  enable_sop_message:1;
            uint8_t  enable_sopp_message:1;
            uint8_t  enable_soppp_message:1;
            uint8_t  enale_sopd_message:1;
            uint8_t  enable_sopdd_message:1;
            uint8_t  enable_hard_reset:1;
            uint8_t  enable_cable_reset:1;
            uint8_t  reserve:1;
        };
    }RECEIVE_DETECE_t;
#else
    typedef union
    {
        uint8_t byte;
        struct
            {
            uint8_t  enable_sop_message:1;
            uint8_t  enable_sopp_message:1;
            uint8_t  enable_soppp_message:1;
            uint8_t  enale_sopd_message:1;
            uint8_t  enable_sopdd_message:1;
            uint8_t  enable_hard_reset:1;
            uint8_t  enable_cable_reset:1;
            uint8_t  reserve:1;
            };
    }RECEIVE_DETECE_t;

#endif

typedef union
{
    uint8_t byte;
    struct
    {
        CC_TYPE_e         cc1_role:2;
        CC_TYPE_e         cc2_role:2;
        RP_VALUE_TYPE_e   rp_valur:2;
        uint8_t           drp_en:1;
        uint8_t           reserve:1;
    };
}CC_ROLE_CONFIG_t;


typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t reserve:1;
        uint8_t vcon_present:2;
        uint8_t vbus_present:1;
        uint8_t vbus_present_detect:1;
        uint8_t reserve2:2;
        uint8_t tcpc_init:1;
        uint8_t reserve3:2;
    };
}POWER_STATUS_t;


typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t i2cerror: 1;
        uint8_t vcon_oc:1;
        uint8_t cc_uv_fault:1;
        uint8_t Reserve:4;
        uint8_t vcon_ov:1;
    };
}FAULT_STATUS_t;



typedef enum
{
    PPS_APDO = 3,
    FIX_PDO = 0,
} PDO_TYPE_e;


typedef enum
{
    Curr_200mA = 0,
    Curr_300mA,
    Curr_400mA,
    Curr_500mA,
    Curr_600mA,
}VCON_OCP_e;

typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t  OSC_EN:1;
        uint8_t  VBUS_DETECTEN:1;
        uint8_t  BG_EN:1;
        uint8_t  LPR_EN:1;
        uint8_t  LPR_RPD:1;
        uint8_t  VCON_DISCHARGEEN:1;
        uint8_t  reserve:2;
    };
}VENDER_PWR_t;





//I2C related
bool IsVBusOK(void);

//used for PD
void Command_Write(COMMAND_TYPE_e command);// 5 commands are allowed:LOOK_4_CONNECTION /DISABLE_VBUS_DETECT/ENABLE_VBUS_DETECT/RESET_TX_BUF/RESET_RX_BUF
void CC_Role_Config(CC_ROLE_CONFIG_t cc_role_config);//used to set CC1/CC2 role,pull_up current(if choose Rp) and dual_role_toggle Enable Bit
void CC_Role_Force(DRP_EN_e a, RP_VALUE_TYPE_e b, CC_TYPE_e c2,CC_TYPE_e c1);
void CC_DRP_Period_Set(DRP_PERIOD_e period);//Used set Dual_role_togggle period
void CC_DRP_Rp_Duty_Set(uint16_t duty);    //used to ser Rp duty cycle during one DRP period :Rp_duty=(duty+1)/1024
void CC_Plug_Set(uint8_t cc);

uint8_t  CC_status_Get(void);
uint8_t  Power_status_Get(void);
uint8_t  Fault_status_Get(void);
uint16_t Alert_status_Get(void);
void  Alert_Clear(uint16_t);


void PD_Receive_Enable(uint8_t temp);//Enable all kinds of PD messages,including SOP,SOP',SOP'',SOP_Debug,hard_reset,Cable_reset and so on

void PD_Receive_Detect_Config(RECEIVE_DETECE_t byte);//used to set Receive detective

uint8_t PD_Receive(void);//receive message in INT function
PD_MSG_t*  PD_Msg_Get(void);//return the pd packet received recetly
void PD_Goodcrc_Header_Init(CABLE_PLUG_TYPE_e cable_plug_type,DATA_ROLE_TYPE_e data_role_type,POWER_ROLE_TYPE_e pwr_role_type,SPEC_REV_TYPE_e spec_rev_type);
void BIST_Carrier_Mode_Send(void); //start BIST Carrier Mode : Shall send out a continuous string of BMC encoded alternating "1"s and ¡°0¡±s for 45ms
void BIST_TEST_DATA_Mode_Enter(void);
void BIST_TEST_DATA_Mode_Quit(void);
void Hard_Reset_Send(void);//Send one hard reset,which would reset all PD receive chanel disable
void PD_Msg_Send(PD_MSG_t t_mes);//Send one Pd message based on t_msg

void Vcon_Init(void);//init config,close VCON,Set OCP Current,Enable OC/OV detect;
void Vcon_On(void);//turn on VCON_BKFT
void Vcon_Off(void);//Disable VCON_BKFT
void Vcon_Discharge_On(void);//Start VCON discharge current about 2mA
void Vcon_Discharge_Off(void);//Disable VCON Dischrge current,which is nessary after VCON_OV happend,otherwise VCON Discharge current would always on
void Vcon_Ocp_Set(VCON_OCP_e value);//used to set VCON_OCP current:    Curr_200mA,Curr_300mA,Curr_400mA,Curr_500mA,Curr_600mA
void Vcon_Oc_Detect_Disable(void);//Disable VCON OC detect,which means VCON_OC max current would be change to 0.8A
void Vcon_Oc_Detect_Enable(void);//Enable Vcon_OC detect,max current is limit according to VCON_OCP reg(200~600mA)
void Vcon_Ov_Detect_Disable(void);
void Vcon_Ov_Detect_Enable(void);
void Vender_PDIC_Shutdown_Enter(void);//IC quit sleep mode
void Vender_PDIC_Shutdown_Quit(void);// IC enters sleep mode, CC1 and CC2 would be set to Rd,24M would be off
void Vender_Power_Config(VENDER_PWR_t pwr_config );
VENDER_PWR_t  Vender_Power_Get(void);//return Vender_Power_Config info
void Vender_Reset(void);//IC reset all, then Vender_PDIC_Enable would be necessary
void Vender_Wakeup_Enable(void);//IC wold quit low_pwr mode after Ra/Rd/Rp attach;
void Vender_Wakeup_Disable(void);//IC would not quit low_pwr mode even though Ra/Rd/Rp attach;
void Vender_Auto_Idle_Enable(uint8_t time_out_value);//IC would quit standby mode to IDLE mode if the conditions are met
void Vender_Auto_Idle_Diable(void);//IC wold not turn off 24M clock if conditions are met

void SY69410_Init(void);

extern uint8_t I2CWriteBytes(uint8_t SlaveAddress, uint8_t MemoryAdress, uint8_t wrNumber, uint8_t* wrPointer);
extern uint8_t I2CReadBytes(uint8_t SlaveAddress, uint8_t MemoryAdress, uint8_t rdNumber, uint8_t* rdPointer);
extern void Set1Reg(uint8_t memory_addr, uint8_t value);
extern uint8_t Get1Reg(uint8_t memory_addr) ;
extern void    Set2Reg(uint8_t memory_addr, uint16_t value);
extern uint16_t Get2Reg(uint8_t memory_addr);

#endif

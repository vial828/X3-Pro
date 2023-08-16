#ifndef PD_COM
#define PD_COM

//#pragma anon_unions

#include  <stdint.h>
#include  "app_pd_general.h"

#define  V_PPS_SMALL_STEP           500         //-500~500mV
#define  N_HARDRESETCPOUNT           2
#define  T_CCDEBOUNCE               100
#define  T_PDDEBOUNCE               30
#define  T_TRY_SRC_DEBOUNCE         380
#define  T_TRY_SNK_DEBOUNCE         380
#define  T_SRC_RECOVER              830
#define  T_UNCHUNKING_NOTSUPPORTED  10
#define  T_CHUNKING_NOTSUPPORTED    45
#define  T_DISCOVER_IDENTITY        45
#define  T_PSTransition             500      //min450    max 550
#define  T_PSSOURCEOFF              835      //min 750   max 920

#define  CAPSCOUNT_MAX              50
#define  BIST_CARRIER_MODE          5
#define  BIST_TEST_DATA             8
#define  T_FIRST_SOURCE_CAP         150 //
#define  T_FIRST_DISCOVER_IDENTITY  45
#define  T_VDM_SENDER_RESPONSE      27   //min 24 max 30
#define  T_TYPEC_SEND_SOURCE_CAP    150  //min 100 max 200
#define  T_SEND_RESPONSE            27   //min 24 max 30
#define  T_PPS_REQUEST              7   //max=10
#define  T_PPS_TIME_OUT             13   //12~15s

#define  T_PS_HARDRESET             30  // min 25 ; max=35
#define  T_SAFE5V                    ///  max 275ms
#define  T_SAVE0V                  //     max 650ms
#define  T_VCONN_DISCHARGE
#define  T_SRC_RECOVER_SPR          880    //0.66s~1s
#define  T_SRC_RECOVER_EPR          1200    //1085~1425ms
#define  T_POWER_TRANSITION         300     //self defined
#define  T_SRC_TRANSITION           30      //25~35ms

#define  T_NEW_SRC                       275  //
#define  T_ERROR_ROCOVERY                25  //
#define  T_ERROE_ROCOVERY_WITH_VCONN     240  //

#define T_TYPEC_SINKWAIT_CAP             465
#define T_SNK_WAIT_SRC_RECOVERY          (660)
#define T_SWAP_SRC_START                 20
#define T_PPSSRC_TRANS_SMALL             20     //max25
#define T_PPSSRC_TRANS_LARGE             200    //max 275

#define T_VCONN_SOURCE_TIMEOUT           150  //min=100    max=200

//I2C slave adress
#define  TCPC_Address                    (0x4E)
#define  RX_BUF_ADDRESS                  0X30
#define  TX_BUF_ADDRESS                  0x51


#define DISCOVER_IDENTITY  0
#define ACK  1
#define REQ  0
#define NACK   2
#define BUSY  3

#define PASSIVE_CABLE  3
#define ACTIVE_CABLE   4

typedef enum
{
    FSM_SWITCH_IMM=0,
    FSM_WAITING
}fsm_result_e;



typedef struct
{
      // pd_status_t pd_status ;//pd_status
      void  (*start_drp)(void);
      void  (*set_cc_plug)(uint8_t cc);
      void  (*set_cc_role)(DRP_EN_e a, RP_VALUE_TYPE_e b, CC_TYPE_e c2,CC_TYPE_e c1);
      void  (*set_drp_period)(DRP_PERIOD_e period);//Used set Dual_role_togggle period
      void  (*set_drp_rp_duty)(uint16_t duty);    //used to ser Rp duty cycle during one DRP period :Rp_duty=(duty+1)/1024
      void  (*set_power_role)(POWER_ROLE_TYPE_e);
      void  (*set_data_role)(DATA_ROLE_TYPE_e);
      void  (*set_pd_rev)(SPEC_REV_TYPE_e);
      void  (*enable_pd_receive)(uint8_t temp);
      PD_MSG_t*(*PD_Msg_Get)(void);
      void  (*enable_bist_data_mode)(uint8_t en);
      void  (*send_hard_reset)(void);
      void  (*send_msg)(PD_MSG_t t_mes);
      void  (*send_ctrl_msg)(CONTROL_MESSAGE_TYPE_e type);
      void  (*send_bist_carry_mode)(void);
      bool  (*is_new_msg_received)(void);
      void  (*clear_new_msg_received)(void);
      bool  (*is_hard_reset_received)(void);
      bool  (*is_vbus_ok)(void);
      PDTRANS_STATUS_e (*PD_Transmit_Status_Get)(void);
      void  (*vcon_init)(void);
      void  (*vcon_on)(void);//turn on VCON_BKFT
      void  (*vcon_off)(void);//Disable VCON_BKFT
      void  (*chip_init)(void);//init Chip
      void  (*PD_Goodcrc_Header_Init)(CABLE_PLUG_TYPE_e cable_plug_type,DATA_ROLE_TYPE_e data_role_type,POWER_ROLE_TYPE_e pwr_role_type,SPEC_REV_TYPE_e spec_rev_type);
}pd_phy_t;

void  App_Int_Fun(void);
void  app_pd_init(void);
void  app_pd_process(void);

#endif

#include   <stdio.h>
#include   "app_pd_snk.h"
#include   "app_pd_cfg.h"
#include   "log.h"

extern  void  PD_Protocol_Reset(void);

extern pd_status_t  pd_status;
extern pd_phy_t  pd;

//extern timer_t * GTIMER_SEL3;
//extern timer_t * GTIMER_SEL4;
extern ptimer_t GTIMER_SEL3;
extern ptimer_t GTIMER_SEL4;

void  Start_Timer_GTIMER_SEL3(uint32_t period, uint8_t opt, TIMER_CB_F callback);
void  Start_Timer_GTIMER_SEL4(uint32_t period, uint8_t opt, TIMER_CB_F callback);

static  uint8_t  flag_snk_send_PD_msg;
uint8_t  flag_snk_prswap_sysclk_start;
uint8_t  flag_snk_vcon_swap_sysclk_start;

static  uint8_t   data_request_num;
static  uint32_t  data_request_current;
uint32_t  data_request_voltage;

uint16_t Expected_Request_Voltage;
uint8_t  debug_snk_test_num;
uint8_t  debug_request_next;
uint8_t  debug_request_add_voltage;

static  pd_snk_soft_reset_adjust_fsm_e       fsm_pd_snk_soft_reset;

#define SEND_NEW_MSG        1
#define NOT_SEND_NEW_MSG    0
typedef void fun_t(void);

static void (*SNK_Process_Control_Msg[])()
    ={
         NULL,                               // case    UNRECGNITED:            break;//do nothing
         NULL,                               // case    GOODCRC :               break;//do nothing
         PD_SNK_Send_Soft_Reset,             // case    GOTOMIN:                break;
         PD_SNK_Send_Soft_Reset,             // case    ACCEPT:                 tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;  flag_snk_send_PD_msg=1; break;
         PD_SNK_Send_Soft_Reset,             // case    REJECT:                 tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;  flag_snk_send_PD_msg=1;  break;
         NULL,                               // case    PING:                   break;//do nothing
         NULL,                               // case    PS_RDY :                break;// do nothign
         PD_SNK_Send_Not_Supported,          // case    GET_SOURCE_CAP :        tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;    flag_snk_send_PD_msg=1;    break;
         PD_SNK_Send_SINK_CAP,               // case    GET_SINK_CAP :          tcpc_status.PDstate_SNK=PE_SNK_Give_Sink_Cap;    flag_snk_send_PD_msg=1;        break;
         PD_SNK_Send_Not_Supported,          // case    DR_SWAP :               tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;      break;
         NULL,                               // case    PR_SWAP :               break;
         NULL,                               // case    VCONN_SWAP :            tcpc_status.PDstate_SNK=PE_SNK_VCONN_SWAP;            break;    //2ѡ1, ACCEPT or Not supported
         PD_SNK_Send_Soft_Reset,             // case    WAIT :                  tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;  flag_snk_send_PD_msg=1; break;
         PD_SNK_Soft_Send_Accept_to_Soft_Reset,  // case    SOFT_RESET :        break;    //process outsid
         PD_SNK_Send_Not_Supported,          // case    DATA_RESET :            tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported; flag_snk_send_PD_msg=1;      break;
         NULL,                               // case    DATA_RESET_COMPLETE:    break;
         NULL,                               // case    NOT_SUPPORTED :         break;    //do nothing
         PD_SNK_Send_Not_Supported,          // case    GET_SOURCE_CAP_EXTEND :     tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;      break; //sending src_cap_extened is also correct response;
         PD_SNK_Send_Not_Supported,          // case    GET_STATUS_PD :         tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;      break;
         PD_SNK_Send_Not_Supported,          // case    FR_SWAP :               tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;     break;
         PD_SNK_Send_Not_Supported,          // case    GET_PPS_STATUS :        tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;     break;
         PD_SNK_Send_Not_Supported,          // case    GET_COUNTRY_CODES :     tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;  flag_snk_send_PD_msg=1;    break;
         PD_SNK_Send_Not_Supported,          // case    GET_SINK_CAP_EXTENED :  tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;    flag_snk_send_PD_msg=1;    break; //Send
         PD_SNK_Send_Not_Supported,          // default:                        tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;    flag_snk_send_PD_msg=1;    break;
         NULL,
         NULL
     };

void PD_SNK_Send_Soft_Reset_Callback(const ptimer_t tm)
{
    if(GTIMER_SEL3){
        if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
            GTIMER_SEL3 = NULL;
        }
    }

    PD_SNK_Send_Soft_Reset();
}

void PD_SNK_Send_Soft_Reset(void)
{
       flag_snk_send_PD_msg=1;
       tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
}

void  PD_SNK_Send_SOURCE_CAP(void)
{
       tcpc_status.PDstate_SNK=PE_SNK_Give_Source_Cap;
       flag_snk_send_PD_msg=1;
}


void  PD_SNK_Send_SINK_CAP(void)
{
       tcpc_status.PDstate_SNK=PE_SNK_Give_Sink_Cap;
       flag_snk_send_PD_msg=1;
}

void PD_SNK_Send_Not_Supported_Callback(const ptimer_t tm)
{
    if(GTIMER_SEL4){
        if(bat_timer_delete(GTIMER_SEL4, portMAX_DELAY)==pdPASS){
            GTIMER_SEL4 = NULL;
        }
    }

    PD_SNK_Send_Not_Supported();
}

void  PD_SNK_Send_Not_Supported(void)
{
       flag_snk_send_PD_msg=1;
       tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;
}

void  PD_SNK_Soft_Send_Accept_to_Soft_Reset(void)//VCON_SWAP PRSWAP  SOFTRESET
{
      tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
      flag_snk_send_PD_msg=1;
}

/*******************************************************************************************************************************

                                                        SNK Processs

********************************************************************************************************************************/
void  App_Vendor_PD_SNK(void)
{
    PD_MSG_t* message_rx;

    //LOGD("\r\n tcpc_status.PDstate_SNK =%d",tcpc_status.PDstate_SNK);
    if(pd.is_hard_reset_received())// Receive Hard_Reset
    {
        pd.enable_bist_data_mode(0);
        //Gtimer_Stop(GTIMER_SEL3);
        //Gtimer_Stop(GTIMER_SEL4);
        //TIMER_SAFE_DELETE(GTIMER_SEL3);
        //TIMER_SAFE_DELETE(GTIMER_SEL4);
        if(GTIMER_SEL3){
            if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                GTIMER_SEL3 = NULL;
            }
        }
        if(GTIMER_SEL4){
            if(bat_timer_delete(GTIMER_SEL4, portMAX_DELAY)==pdPASS){
                GTIMER_SEL4 = NULL;
            }
        }
        //Stop_Sysclock();
        flag_snk_prswap_sysclk_start=FALSE;
        tcpc_status.hard_resrt_on=1;
        tcpc_status.PDstate_SNK=PE_SNK_Hard_Reset_Wait;    //receive hard reset,wait tsrc_recovery then restart
    }
    else switch(tcpc_status.PDstate_SNK)
    {
         case    PE_SNK_Default:
                      PD_SNK_Init(); //Reset Protocol Layer
                      tcpc_status.PDstate_SNK=PE_SNK_Startup;//new start
                      break;
         case    PE_SNK_Startup:   //start after hard reset or prswap
                      PD_Protocol_Reset();
                      pd.set_pd_rev(REV_3);
                      tcpc_status.PPS_Mode=0;
                      tcpc_status.Explict_Contract=0;
                      Expected_Request_Voltage=9000; //12000 measns max request voltage=12V;change to 9000 if the max input voltage is 9V
                      tcpc_status.PDstate_SNK=PE_SNK_Discovery;
                      break;
         case    PE_SNK_Discovery:
                      if(pd.is_vbus_ok()) //Vbus is present
                      {
                          tcpc_status.PDstate_SNK=PE_SNK_Wait_for_Capablities;
                          //Start_SinkWaitCapTimer(GTIMER_SEL3, T_TYPEC_SINKWAIT_CAP, PD_SNK_Send_Hard_Reset); //source_cap has to be received within 465ms
                          Start_Timer_GTIMER_SEL3(T_TYPEC_SINKWAIT_CAP, TIMER_OPT_ONESHOT, PD_SNK_Send_Hard_Reset_Callback);
                      }
                      break;
         case    PE_SNK_Wait_for_Capablities:
                      if(pd.is_new_msg_received())
                      {
                          pd.clear_new_msg_received();
                          tcpc_status.PD_Connected=1;
                          message_rx=pd.PD_Msg_Get();
                             if((message_rx->header.number_Data_Objects>0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOURCE_CAPABILITIES))
                             {
                                 //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                                 //TIMER_SAFE_DELETE(GTIMER_SEL3);
                                 if(GTIMER_SEL3){
                                     if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                                         GTIMER_SEL3 = NULL;
                                     }
                                 }
                                 tcpc_status.PDstate_SNK=PE_SNK_Evaluate_Capablity;
                                 LOGD("Rx:SRC_CAP");
                             }
                             else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
                             {
                                 //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                                 //TIMER_SAFE_DELETE(GTIMER_SEL3);
                                 if(GTIMER_SEL3){
                                     if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                                         GTIMER_SEL3 = NULL;
                                     }
                                 }
                                 flag_snk_send_PD_msg=1;
                                 tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
                             }
                     }
                     break;
          case    PE_SNK_Evaluate_Capablity:
                      tcpc_status.hardresetcounter=0;//receive source cap
                      PE_SNK_Evaluate_Capablity_act(); //to get request num and current
                      tcpc_status.PDstate_SNK=PE_SEN_Send_Request;
                      LOGD("Tx:REQUEST");
                      flag_snk_send_PD_msg=1;
                      break;
         case    PE_SEN_Send_Request:              PE_SNK_Select_Capability_act();  break;
         case    PE_SNK_Wait_Accept:               PE_SNK_Wait_Accept_act();        break;
         case    PE_SNK_Transition_Sink:           PE_SNK_Transition_Sink_act();    break;
         case    PE_SNK_Ready:                     PE_SNK_Ready_act();              break;
         case    PE_SNK_Give_Sink_Cap:             PE_SNK_Give_Sink_Cap_act();      break;
         case    PE_SNK_Give_Source_Cap:           PE_SNK_Get_Source_Cap_act();     break;
         case    PE_SNK_Hard_Reset:
                      //close protection;turn off sink
                      PD_SNK_Send_Hard_Reset();
                      break;
         case    PE_SNK_Hard_Reset_Wait:
                      //Start_Hard_Reset_Timer(GTIMER_SEL3, T_SNK_WAIT_SRC_RECOVERY, PD_SNK_Jump_to_Default);  //wait time=660ms
                      Start_Timer_GTIMER_SEL3(T_SNK_WAIT_SRC_RECOVERY, TIMER_OPT_ONESHOT, PD_SNK_Jump_to_Default_Callback);
                      break;
         case    PE_SNK_Soft_Reset:                PE_SNK_Soft_Reset_fsm();  break;
         //case    PE_SNK_PR_SWAP:                 SNK_PRSWAP_fsm();         break;
         //case    PE_SNK_VCONN_SWAP:              SNK_VCONN_Swap_fsm();     break;
         case   PE_SNK_Transition_to_Default:
                      //Reset Local HW
                      pd.set_data_role(UFP);
                      pd.set_power_role(SNK_ROLE);
                      pd.set_pd_rev(REV_3);
                      tcpc_status.PDstate_SNK=PE_SNK_Startup;
                      tcpc_status.hard_resrt_on=0;
                      pd.enable_pd_receive(EN_SOP|EN_HARD_RESET); //after receive hard reset,EN_RECEIVE should be reset
                      break;
         case PE_SNK_Hard_Reset_Received:       tcpc_status.PDstate_SNK=PE_SNK_Transition_to_Default;      break;//
         case PE_SNK_Soft_Reset_Receive:        PE_SNK_Soft_Reset_Receive_act();       break;
         case PE_SNK_Send_NotSupported:         PE_SNK_Send_NotSupported_act();        break;
         case PE_SNK_BIST_TEST_DATA:                                                   break;
         case PE_SNK_BIST_Carried_Mode:         PE_SNK_BIST_Carried_Mode_act();        break;
         case PE_SNK_Send_GetSRCCAP:            PE_SNK_Send_GetSRCCAP_act();           break;
         case PE_SNK_Disable:                                                          break;
         case PE_SNK_Send_Debug:                PE_SNK_Send_Debug_act();               break;
         default:                                                                      break;
    }
}

void  PD_SNK_Init()
{
      tcpc_status.PD_Connected = 0;
      tcpc_status.hard_resrt_on=0;
      tcpc_status.hardresetcounter=0;

      pd.set_power_role(SNK_ROLE);//hard reset or por would reset the port data role to default:SRC/DFT
      pd.set_data_role(UFP);
      pd.set_pd_rev(REV_3);
      PD_Protocol_Reset();
      tcpc_status.cc_vbus_update_en=1;

      tcpc_status.pd_rev=REV_3;

      pd.PD_Transmit_Status_Get(); //清除发送状态
      pd.enable_pd_receive(EN_SOP|EN_HARD_RESET); //Enable receive SOP and Hard Reset not including SOP'
      //Gtimer_Stop(GTIMER_SEL3);
      //Gtimer_Stop(GTIMER_SEL4);
      //TIMER_SAFE_DELETE(GTIMER_SEL3);
      if(GTIMER_SEL3){
          if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
              GTIMER_SEL3 = NULL;
          }
      }
      //TIMER_SAFE_DELETE(GTIMER_SEL4);
      if(GTIMER_SEL4){
          if(bat_timer_delete(GTIMER_SEL4, portMAX_DELAY)==pdPASS){
              GTIMER_SEL4 = NULL;
          }
      }
      data_request_num=0;
      //Stop_Sysclock();
      flag_snk_prswap_sysclk_start=FALSE;
      LOGD("\r\n SNK Init");
}

void PD_Protocol_Reset(void)
{
      tcpc_status.rx_stored_msgID=-1;
      tcpc_status.messageID=0;
}
void PD_SNK_Jump_to_Default_Callback(const ptimer_t tm)
{
    if(GTIMER_SEL3){
        if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
            GTIMER_SEL3 = NULL;
        }
    }

    PD_SNK_Jump_to_Default();
}

void PD_SNK_Jump_to_Default()
{
     //Stop_Hard_Reset_Timer(GTIMER_SEL3);
     //TIMER_SAFE_DELETE(GTIMER_SEL3);
     if(GTIMER_SEL3){
         if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
             GTIMER_SEL3 = NULL;
         }
     }
     pd.vcon_off();
     tcpc_status.PDstate_SNK=PE_SNK_Transition_to_Default;
}

void PE_SNK_Evaluate_Capablity_act()
{
    PD_MSG_t*      message_rx;
    source_cap_t   rx_source_cap_data;
    source_cap_t*  rx_source_cap=&rx_source_cap_data;
    sink_cap_t*    my_sink_cap =App_PD_Get_Sink_Capa();


    #ifdef REQUEST_DEBUGE
       uint8_t i;
       message_rx=pd.PD_Msg_Get();
       if(tcpc_status.pd_rev!=message_rx->header.spec_Rev)
       {
             if(message_rx->header.spec_Rev==REV_3)
             {
                 pd.set_pd_rev(REV_3);
             }
             else
             {
                 pd.set_pd_rev(REV_2);
             }
             //PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
      }
      if(debug_request_next==1)
      {
          debug_request_next=0;
          data_request_num++;
          if(data_request_num>=message_rx->header.number_Data_Objects)
          {
             data_request_num=0;
          }
      }
      i=data_request_num;
      rx_source_cap->w=message_rx->data[i];
      if( (((message_rx->data[i])&SUPPLY_TYPE_MASK)==FIXED_SUPPLY_TYPE) &&(rx_source_cap->fixed.voltage_50mv*50<=Expected_Request_Voltage) )
      {
          data_request_current=rx_source_cap->fixed.current_max_10ma;//请求最大电流
          //LOGD("\r\nfixed:%d mV,%d mA ",rx_source_cap->fixed.voltage_50mv*50,rx_source_cap->fixed.current_max_10ma*10);
          tcpc_status.PPS_Mode=0;
      }
      else if((((message_rx->data[i])&SUPPLY_TYPE_MASK)==APDO_SUPPLY_TYPE))//PPS
      {
            //LOGD("\r\nfixed:%d mV,%d mA ",rx_source_cap->fixed.voltage_50mv*50,rx_source_cap->fixed.current_max_10ma*10);
            data_request_current=rx_source_cap->pps.current_max_50ma;
            if(tcpc_status.PPS_Mode==1)
            {
                if(debug_request_add_voltage)
                    data_request_voltage=data_request_voltage+10;
            }
            else
            {
                data_request_voltage=rx_source_cap->pps.minimum_voltage_100mv;
                tcpc_status.PPS_Mode=1;
            }
     }
     else //other
     {
            rx_source_cap->w=message_rx->data[0];
            data_request_current=rx_source_cap->fixed.current_max_10ma;//请求最大电流
            data_request_num=0;
     }
    #else
    /*
            Request rule:1.V_request<=Expected_Request_Voltage;
                         2.source_type=fixed;
                         3.choose Max_power;
    */
    uint16_t  request_line[7]={0};
    uint16_t  request_current[7]={0};
    uint32_t  request_power[7]={0};
    uint32_t  max_power=0;
    uint32_t i;
    message_rx=pd.PD_Msg_Get();
    if(tcpc_status.pd_rev!=message_rx->header.spec_Rev)
    {
        if(message_rx->header.spec_Rev==REV_3)
        {
            tcpc_status.pd_rev=REV_3;
        }
        else
        {
            tcpc_status.pd_rev=REV_2;
        }
        pd.PD_Goodcrc_Header_Init(NOT_CABLE,tcpc_status.data_role, tcpc_status.power_role, tcpc_status.pd_rev);
    }

    for(i=0;i<message_rx->header.number_Data_Objects;i++)
    {
            rx_source_cap->w=message_rx->data[i];
            if( (((message_rx->data[i])&SUPPLY_TYPE_MASK)==FIXED_SUPPLY_TYPE) &&(rx_source_cap->fixed.voltage_50mv*50<=Expected_Request_Voltage) )
            {
                    request_line[i]=1;
                    request_current[i]=rx_source_cap->fixed.current_max_10ma;//请求最大电流

                  /*
                  if(rx_source_cap->fixed.current_max_10ma*rx_source_cap->fixed.voltage_50mv<(18*1000000/500))//src提供功率<18W
                  {
                       request_current[i]=rx_source_cap->fixed.current_max_10ma;//请求最大电流
                  }
                  else if(18*1000000/500/rx_source_cap->fixed.voltage_50mv>300)// Src提供功率>18W 并且>3A
                  {
                       request_current[i]=300;//请求3A
                  }
                  else
                       request_current[i]=18*1000000/500/rx_source_cap->fixed.voltage_50mv;
                  */

                 request_power[i]=request_current[i]*rx_source_cap->fixed.voltage_50mv;
                 //LOGD("\r\nfixed:%d mV,%d mA ",rx_source_cap->fixed.voltage_50mv*50,rx_source_cap->fixed.current_max_10ma*10);
            }
            else if((((message_rx->data[i])&SUPPLY_TYPE_MASK)==FIXED_SUPPLY_TYPE))
            {
                 request_line[i]=0;
                 //LOGD("\r\nfixed:%d mV,%d mA ",rx_source_cap->fixed.voltage_50mv*50,rx_source_cap->fixed.current_max_10ma*10);
            }
            else
            {
                 request_line[i]=0;
                 //LOGD("\r\nPPS:min %d mV,max %d mV,%d mA ",rx_source_cap->pps.minimum_voltage_100mv*100,rx_source_cap->pps.maximum_voltage_100mv*100,rx_source_cap->pps.current_max_50ma*50);
            }
    }

    for(i=0;i<message_rx->header.number_Data_Objects;i++)
    {
        if(request_line[i]==1)
        {
            if(max_power<=request_power[i])
            {
                max_power=request_power[i];
                data_request_num=i;
                data_request_current=request_current[i];
            }
        }
    }
   #endif
}


void PD_Advertise_SNK_Capablity(void)
{
    uint8_t i;
    PD_MSG_t t_mes;
    sink_cap_t* my_sink_capa=App_PD_Get_Sink_Capa();
    t_mes.header.extended=0;
    t_mes.header.messageID=tcpc_status.messageID;
    t_mes.header.message_Type=SINK_CAPABILITIES;
    t_mes.header.number_Data_Objects=REQUEST_TYPE_SUPPORTED_MAX;
    t_mes.header.port_Data_Role=tcpc_status.data_role;
    t_mes.header.port_Power_Role=tcpc_status.power_role;
    t_mes.header.spec_Rev=tcpc_status.pd_rev;
    tcpc_status.messageID=(tcpc_status.messageID+1)%7;
    for(i=0;i<SUPPLY_TYPE_SUPPORTED_MAX;i++)
    {
      t_mes.data[i]=my_sink_capa[i].w;
    }
    t_mes.Sop_Type=TYPE_SOP;
    pd.send_msg(t_mes);
}

void PE_SNK_Select_Capability_act()
{
      if(flag_snk_send_PD_msg==1)
      {
           PD_Send_Request(data_request_num,data_request_current);
             if(data_request_num==0)
                 tcpc_status.above5V=0;
             else
                 tcpc_status.above5V=1;
          flag_snk_send_PD_msg=0;
      }
      else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
      {
           case TRANSMIT_FAIL:    tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
                                  flag_snk_send_PD_msg=1;
                                  LOGD("Tx fail");
                                  break;
           case TRANSMIT_DISCARD: flag_snk_send_PD_msg =1;
                                  break;   //retransmit
           case TRANSMIT_SUCCESS:
                tcpc_status.PDstate_SNK=PE_SNK_Wait_Accept;
                LOGD("wait accept");
                //Start_SenderResponseTimer(GTIMER_SEL3,T_SEND_RESPONSE, PD_SNK_Send_Soft_Reset);
                Start_Timer_GTIMER_SEL3(T_SEND_RESPONSE, TIMER_OPT_ONESHOT, PD_SNK_Send_Soft_Reset_Callback);
                break;
           case TRANSMIT_ERROR:   flag_snk_send_PD_msg =1;          break;   //retransmit
           default           :    break;   //
      }
}


void PD_Send_Request(uint8_t number,uint16_t current)
{
    request_t request_msg;
    PD_MSG_t  t_mes;
    request_msg.w = 0;
    if(tcpc_status.PPS_Mode==1)
    {
    /*    request_msg.pps.capablity_mismatch=0;
          request_msg.pps.erp_mode_capable=0;
          request_msg.pps.giveback_flag=0;
          request_msg.pps.no_usb_suspend=0;
          request_msg.pps.object_position=number+1;
          request_msg.pps.operating_current_50ma=40;
          request_msg.pps.reserved=0;
          request_msg.pps.reserved1=0;
          request_msg.pps.unchunked_extended_message_support=0;
          request_msg.pps.usb_communications_capable=0;
          request_msg.pps.voltage_20mv=data_request_voltage*5;
          time_s=0;*/
    }
    else
    {
        request_msg.fixed.max_min_current_10ma=current;
        request_msg.fixed.operating_current_10mA=current;
        request_msg.fixed.reserved=0;
        request_msg.fixed.erp_mode_capable=0;
        request_msg.fixed.unchunked_extended_message_support=0;
        request_msg.fixed.no_usb_suspend=0;
        request_msg.fixed.usb_communications_capable=0;
        request_msg.fixed.capablity_mismatch=0;
        request_msg.fixed.giveback_flag=0;
        request_msg.fixed.object_position=number+1;
    }

        t_mes.header.extended=0;
        t_mes.header.messageID=tcpc_status.messageID;
        t_mes.header.message_Type=REQUEST;
        t_mes.header.number_Data_Objects=1;
        t_mes.header.port_Data_Role=tcpc_status.data_role;
        t_mes.header.port_Power_Role=SNK_ROLE;
        t_mes.header.spec_Rev=tcpc_status.pd_rev;
        tcpc_status.messageID=(tcpc_status.messageID+1)%7;
        t_mes.data[0]=request_msg.w;
        t_mes.Sop_Type=TYPE_SOP;
        LOGD("request %d:Current:%d ",number+1,current);
        pd.send_msg(t_mes);
}

void PE_SNK_Wait_Accept_act()
{
    PD_MSG_t*  message_rx;

    if(pd.is_new_msg_received())
        {
            pd.clear_new_msg_received();
            message_rx=pd.PD_Msg_Get();
            if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==ACCEPT))
            {
                //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                //TIMER_SAFE_DELETE(GTIMER_SEL3);
                if(GTIMER_SEL3){
                    if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                        GTIMER_SEL3 = NULL;
                    }
                }
                tcpc_status.PDstate_SNK=PE_SNK_Transition_Sink;
                //pd_pwr.Input_Current_Set(0);//receive accept
                LOGD("RX:accept");
                //Start_PSTransitionTimer(GTIMER_SEL3, T_PSTransition, PD_SNK_Send_Hard_Reset);//wait soft reset ,else send hard reset
                Start_Timer_GTIMER_SEL3(T_PSTransition, TIMER_OPT_ONESHOT, PD_SNK_Send_Hard_Reset_Callback);
            }
            else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
            {
                //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                //TIMER_SAFE_DELETE(GTIMER_SEL3);
                if(GTIMER_SEL3){
                    if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                        GTIMER_SEL3 = NULL;
                    }
                }
                flag_snk_send_PD_msg=1;
                tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
            }
            else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==REJECT ||message_rx->header.message_Type==WAIT )&&(tcpc_status.Explict_Contract==1))
                { //reveive reject or wait in explict_contract
                    //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                    if(GTIMER_SEL3){
                        if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                            GTIMER_SEL3 = NULL;
                        }
                    }
                    tcpc_status.PDstate_SNK=PE_SNK_Ready;
                }
            else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==REJECT ||message_rx->header.message_Type==WAIT )&&(tcpc_status.Explict_Contract==0))
                { //reveive reject or wait in unexplict_contract
                    //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                    if(GTIMER_SEL3){
                        if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                            GTIMER_SEL3 = NULL;
                        }
                    }
                    tcpc_status.PDstate_SNK=PE_SNK_Wait_for_Capablities;
                    //Start_SinkWaitCapTimer(GTIMER_SEL3, T_TYPEC_SINKWAIT_CAP, PD_SNK_Send_Hard_Reset); //source_cap has to be received within 465ms
                    Start_Timer_GTIMER_SEL3(T_TYPEC_SINKWAIT_CAP, TIMER_OPT_ONESHOT, PD_SNK_Send_Hard_Reset_Callback);
                }
        }
}

void PE_SNK_Transition_Sink_act()
{
        PD_MSG_t*  message_rx;
        if(pd.is_new_msg_received())
        {
                pd.clear_new_msg_received();
                message_rx=pd.PD_Msg_Get();
                if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==PS_RDY))
                {
                    LOGD("Rx:PS_RDY");
                    //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                    //TIMER_SAFE_DELETE(GTIMER_SEL3);
                    if(GTIMER_SEL3){
                        if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                            GTIMER_SEL3 = NULL;
                        }
                    }
                    tcpc_status.PDstate_SNK=PE_SNK_Ready;
                    if(tcpc_status.PPS_Mode==1)
                    {
                          //Set VDPM and IDPM
                          //pd_pwr.Charge_Current_Set(1000);
                          //pd_pwr.Input_Current_Set(data_request_current*50);
                    }
                    else
                    {
                          //Set VDPM and IDPM
                          //pd_pwr.Input_Current_Set(data_request_current*10);
                          //pd_pwr.Charge_Current_Set(1000);
                    }
                }
                else if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==SOFT_RESET))
                {
                    //Stop_SinkWaitCapTimer(GTIMER_SEL3);
                    //TIMER_SAFE_DELETE(GTIMER_SEL3);
                    if(GTIMER_SEL3){
                        if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                            GTIMER_SEL3 = NULL;
                        }
                    }
                    flag_snk_send_PD_msg=1;
                    tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
                }
                else
                {
                     PD_SNK_Send_Hard_Reset();
                }
        }
}

void PD_SNK_Send_Hard_Reset_Callback(const ptimer_t tm)
{
    if(GTIMER_SEL3){
        if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
            GTIMER_SEL3 = NULL;
        }
    }
    PD_SNK_Send_Hard_Reset();
}

void PD_SNK_Send_Hard_Reset()
{
    if(tcpc_status.hardresetcounter<N_HARDRESETCPOUNT)//max hard_reset count=2
    {
        pd.send_hard_reset();
        tcpc_status.hard_resrt_on=1;
        tcpc_status.hardresetcounter++;
        tcpc_status.PDstate_SNK=PE_SNK_Hard_Reset_Wait;
    }
    else
        tcpc_status.PDstate_SNK=PE_SNK_Disable;
}


void PE_SNK_Send_NotSupported_act()
{
     if(flag_snk_send_PD_msg==1)
     {
         if(tcpc_status.pd_rev==REV_3)
             pd.send_ctrl_msg(NOT_SUPPORTED);
         else
             pd.send_ctrl_msg(REJECT);//NO NOT_SUPPORTED in REV_2
        flag_snk_send_PD_msg=0;
        //Stop_Chunking_Notsupported_Timer(GTIMER_SEL4);
        //TIMER_SAFE_DELETE(GTIMER_SEL4);
        if(GTIMER_SEL4){
             if(bat_timer_delete(GTIMER_SEL4, portMAX_DELAY)==pdPASS){
                 GTIMER_SEL4 = NULL;
             }
         }
     }
    switch(pd.PD_Transmit_Status_Get())//Read would clear this status
        {
              case TRANSMIT_FAIL:   tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
                                    flag_snk_send_PD_msg=1;
                                    break;
              case TRANSMIT_DISCARD:     flag_snk_send_PD_msg =1;
                                    break;   //retransmit
              case TRANSMIT_SUCCESS:
                                    tcpc_status.PDstate_SNK=PE_SNK_Ready;
                                    break;
              case TRANSMIT_ERROR:  flag_snk_send_PD_msg =1;                 break;//retransmit
              default         :     break;   //
        }
}


void PE_SNK_Ready_act()
{
       PD_MSG_t * message_rx;
       uint8_t   msg_num_data;
       uint8_t   msg_extended;
       uint8_t   msg_type;
       if(pd.is_new_msg_received())
       {
           pd.clear_new_msg_received();
           message_rx=pd.PD_Msg_Get();
           msg_num_data=message_rx->header.number_Data_Objects;
           msg_extended=message_rx->header.extended;
           msg_type=message_rx->header.message_Type;

           if((msg_num_data>0)&&(msg_extended==0)&&(msg_type==SOURCE_CAPABILITIES))
           {
               tcpc_status.PDstate_SNK=PE_SNK_Evaluate_Capablity;// restart Request
           }
           else if((msg_num_data==0)&&(msg_extended==0)&&(msg_type==SOFT_RESET))
           {
               tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset_Receive;
               flag_snk_send_PD_msg=1;
           }
           else if((msg_num_data==0)&&(msg_extended==0)&&(msg_type==PR_SWAP))
           {
               tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported;    //Not Supported
               flag_snk_send_PD_msg=1;
           }
                else if((msg_num_data==0)&&(msg_extended==0)&&(msg_type==VCONN_SWAP))
           {
               tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported; //Not Supported
               flag_snk_send_PD_msg=1;
           }
           else
               PD_SNK_Process_Protocol_Error();
        }
        /*else if(uart_flag.command.request==1)
        {
               uart_flag.command.request=0;
               flag_snk_send_PD_msg=1;
               tcpc_status.PDstate_SNK=PE_SNK_Send_GetSRCCAP;
        }*/
        else if(debug_snk_test_num==1)//debug: test send Soft_reset
        {
               debug_snk_test_num=0;
               tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
               flag_snk_send_PD_msg=1;
        }
        else if(debug_snk_test_num==2)// debug test hard reset
        {
               debug_snk_test_num=0;
               PD_SNK_Send_Hard_Reset();
        }
        else if(debug_snk_test_num!=0)
        {
               tcpc_status.PDstate_SNK=PE_SNK_Send_Debug;
               flag_snk_send_PD_msg=1;
        }
       /*if((tcpc_status.PPS_Mode==1)&&(time_s>T_PPS_REQUEST))
        {
               time_s=0;
               flag_snk_send_PD_msg=1;
               tcpc_status.PDstate_SNK=PE_SNK_Send_GetSRCCAP;
        }*/
}


void PD_SNK_Process_Protocol_Error()
{
    PD_MSG_t* message_rx=pd.PD_Msg_Get();
    uint8_t msg_data_num=message_rx->header.number_Data_Objects;
    uint8_t msg_extended=message_rx->header.extended;
    uint8_t msg_type=message_rx->header.message_Type;
    if(msg_data_num==0 && msg_extended==0)
    {
          SNK_Process_Control_Msg[msg_type]();
    }
    else if(msg_extended==0 && msg_data_num!=0)
    {
          switch(msg_type)
            {
                case       SOURCE_CAPABILITIES:        break;//do nothing
                case       REQUEST:                    break;//process outside
                case       BIST:
                             if( message_rx->data[0]>>28==BIST_CARRIER_MODE && (tcpc_status.above5V==0) )
                              {
                                  flag_snk_send_PD_msg=1;
                                  tcpc_status.PDstate_SNK=PE_SNK_BIST_Carried_Mode;
                              }
                              else if(message_rx->data[0]>>28==BIST_TEST_DATA)
                              {
                                  pd.enable_bist_data_mode(1);
                                  tcpc_status.PDstate_SNK=PE_SNK_BIST_TEST_DATA;
                              }
                              else
                              {
                                  tcpc_status.PDstate_SNK=PE_SNK_Send_NotSupported; flag_snk_send_PD_msg=1;
                              }
                           break;
                case       SINK_CAPABILITIES:          break;  //do nothing
                case       BATTERY_STATUS:             break;  //do nothing
                case       ALERT:                      PD_SNK_Send_Not_Supported();        break;
                case       GET_COUNTRY_INFO:           PD_SNK_Send_Not_Supported();         break;
                case       ENTER_USB:                  PD_SNK_Send_Not_Supported();      break;
                case       VENDER_DEFINED:             PD_SNK_Send_Not_Supported();      break;// do nothing
            default:                                   break;
            }
    }
    else
    {
            switch(msg_type)
            {
            case        SOURCE_CAPABILITIES_EXTENDED:                                break;
            case        STATUS:                                                      break;
            case        GET_BATTERY_CAP:           PD_SNK_Send_Not_Supported();      break;
            case        GET_BATTERY_STATUS :       PD_SNK_Send_Not_Supported();      break;
            case        BATTERY_CAPABILITIES :                                       break;
            case        GET_MANUFACTURER_INFO :    PD_SNK_Send_Not_Supported();      break;
            case        MANUFACTURER_INFO :        PD_SNK_Send_Not_Supported();      break;
            case        SECURITY_REQUES:           PD_SNK_Send_Not_Supported();      break;
            case        SECURITY_RESPONSE :        PD_SNK_Send_Not_Supported();      break;
            case        FIRMWARE_UPDATE_REQUEST :  break;
            case        FIRMWARE_UPDATE_RESPONSE:  break;
            case        PPS_STATUS :               break;
            case        COUNTRY_INFO :             break;
            case        COUNTRY_CODES :            break;
            case        SINK_CAPABILITIES_EXTENDED :    break;
            case        31:                        if(((message_rx->data[0])&(0X00008000))!=0)    //chunked message
                                                        //Start_Chunking_Notsupported_Timer(GTIMER_SEL4,T_CHUNKING_NOTSUPPORTED,PD_SNK_Send_Not_Supported);
                                                        Start_Timer_GTIMER_SEL4(T_CHUNKING_NOTSUPPORTED, TIMER_OPT_ONESHOT, PD_SNK_Send_Not_Supported_Callback);
                                                   else //unchunked message
                                                        //Start_Chunking_Notsupported_Timer(GTIMER_SEL4,T_UNCHUNKING_NOTSUPPORTED,PD_SNK_Send_Not_Supported);
                                                        Start_Timer_GTIMER_SEL4(T_UNCHUNKING_NOTSUPPORTED, TIMER_OPT_ONESHOT, PD_SNK_Send_Not_Supported_Callback);
                                                   break;
            default :                              break;
            }
    }

}

void  PE_SNK_Send_Control_Msg_act(CONTROL_MESSAGE_TYPE_e message_type,PD_snk_state_e success_path,uint8_t success_send_flag,PD_snk_state_e fail_path,uint8_t fail_send_flag)
{
         if(flag_snk_send_PD_msg==1)
        {
            pd.send_ctrl_msg(message_type);
            flag_snk_send_PD_msg=0;
        }
        switch(pd.PD_Transmit_Status_Get())//Read would clear this status
        {
            case TRANSMIT_FAIL:        tcpc_status.PDstate_SNK=fail_path;
                                       flag_snk_send_PD_msg=fail_send_flag;
                                       break; //delay then transmit
            case TRANSMIT_DISCARD:     flag_snk_send_PD_msg =1;
                                       break;       //retransmit
            case TRANSMIT_SUCCESS:
                                       tcpc_status.PDstate_SNK=success_path;
                                       flag_snk_send_PD_msg=success_send_flag;
                                                   break;
            case TRANSMIT_ERROR:       flag_snk_send_PD_msg =1;            break;   //retransmit
            default         :          break;   //
        }
}

void  PE_SNK_Send_Complx_Msg_act(fun_t* action,PD_snk_state_e success_path,uint8_t success_send_flag,PD_snk_state_e fail_path,uint8_t fail_send_flag)
{
         if(flag_snk_send_PD_msg==1)
        {
            action();
            flag_snk_send_PD_msg=0;
        }
        switch(pd.PD_Transmit_Status_Get())//Read would clear this status
        {
            case TRANSMIT_FAIL:        tcpc_status.PDstate_SNK=fail_path;
                                       flag_snk_send_PD_msg=fail_send_flag;
                                       break; //delay then transmit
            case TRANSMIT_DISCARD:     flag_snk_send_PD_msg =1;
                                       break;       //retransmit
            case TRANSMIT_SUCCESS:
                                       tcpc_status.PDstate_SNK=success_path;
                                       flag_snk_send_PD_msg=success_send_flag;
                                       break;
            case TRANSMIT_ERROR:       flag_snk_send_PD_msg =1;              break;   //retransmit
            default         :          break;   //
        }
}

void PE_SNK_Soft_Reset_Receive_act()
{
      PE_SNK_Send_Control_Msg_act(ACCEPT,//send control msg ACCEPT
                                  PE_SNK_Wait_for_Capablities,NOT_SEND_NEW_MSG, //success path
                                  PE_SNK_Soft_Reset,SEND_NEW_MSG);//fail path

        /*  if(flag_snk_send_PD_msg==1)
       {
             PD_Protocol_Reset();//receive soft reset would cleat msgID
           PD_Send_Control_Message(ACCEPT);
           flag_snk_send_PD_msg=0;
       }
       switch(PD_Transmit_Status_Get())//Read would clear this status
       {
           case TRANSMIT_FAIL:       tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
                                     flag_snk_send_PD_msg=1;
                                     break;   //delay then transmit
           case TRANSMIT_DISCARD:    flag_snk_send_PD_msg =1;
                                     break;   //retransmit
           case TRANSMIT_SUCCESS:
                                     tcpc_status.PDstate_SNK=PE_SNK_Wait_for_Capablities;
                                     break;   //retransmit
           case TRANSMIT_ERROR:      flag_snk_send_PD_msg =1;          break;   //retransmitc
           default           :       break;   //
       }
         */
}

/*

void PD_SNK_Send_Soft_reset()
{
      tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
      flag_snk_send_PD_msg=1;
}
*/

void PE_SNK_Give_Sink_Cap_act()
{
    PE_SNK_Send_Complx_Msg_act(PD_Advertise_SNK_Capablity,  //operation
                              PE_SNK_Ready,NOT_SEND_NEW_MSG,//success path
                              PE_SNK_Soft_Reset,SEND_NEW_MSG);//fail path

}


void PE_SNK_Get_Source_Cap_act()
{
     /*  PE_SNK_Send_Complx_Msg_act(PD_Advertise_SRC_Capablity,
                                 PE_SNK_Ready,NOT_SEND_NEW_MSG,
                                 PE_SNK_Soft_Reset,SEND_NEW_MSG);
     */
}

void PE_SNK_BIST_Carried_Mode_act(void)
{
      PE_SNK_Send_Complx_Msg_act(pd.send_bist_carry_mode,
                                 PE_SNK_Ready,NOT_SEND_NEW_MSG, //success path
                                 PE_SNK_Ready,NOT_SEND_NEW_MSG);//fail path

}


void PE_SNK_Send_GetSRCCAP_act(void)
{
      PE_SNK_Send_Control_Msg_act(GET_SOURCE_CAP,//to send msg
                                  PE_SNK_Ready,NOT_SEND_NEW_MSG, //success path
                                  PE_SNK_Soft_Reset,SEND_NEW_MSG);//fail path
}

void PE_SNK_Send_Debug_act()
{
        if(flag_snk_send_PD_msg==1)
        {
            if(debug_snk_test_num==5)
               pd.send_ctrl_msg(GET_PPS_STATUS);
            else if(debug_snk_test_num==6)
                 pd.send_ctrl_msg(GET_SINK_CAP);
            else if(debug_snk_test_num==7)
                 pd.send_ctrl_msg(GET_SOURCE_CAP_EXTEND);
            else
                 pd.send_ctrl_msg(GET_PPS_STATUS);

            flag_snk_send_PD_msg=0;
        }
        switch(pd.PD_Transmit_Status_Get())//Read would clear this status
        {
            case TRANSMIT_FAIL:        tcpc_status.PDstate_SNK=PE_SNK_Soft_Reset;
                                       flag_snk_send_PD_msg=1;
                                       break;   //delay then transmit
            case TRANSMIT_DISCARD:     flag_snk_send_PD_msg =1;
                                       break;       //retransmit
            case TRANSMIT_SUCCESS:
                                       tcpc_status.PDstate_SNK=PE_SNK_Ready;  //wait PPS_STATUS
                                       break;
            case TRANSMIT_ERROR:       flag_snk_send_PD_msg =1;               break;//retransmit
            default         :          break;   //
        }
        PE_SNK_Send_Control_Msg_act(GET_SOURCE_CAP,//to send msg
                                    PE_SNK_Ready,NOT_SEND_NEW_MSG, //success path
                                    PE_SNK_Soft_Reset,SEND_NEW_MSG);//fail path
}




/* *****************************************************Soft Reset Process******************************************************
typedef enum
{
   FSM_PD_SNK_SR_SEND_SOFTRRESET=0,
     FSM_PD_SNK_SR_WAIT_ACCEPT,
     FSM_PD_SNK_SR_RESTART
}pd_snk_soft_reset_adjust_fsm_e;

********************************************************************************************************************************/

static fsm_result_e (*_fsm_pd_snk_soft_reset_handle[])(pd_snk_soft_reset_adjust_fsm_e*)=
{
    _fsm_pd_snk_sr_send_soft_reset,
    _fsm_pd_snk_sr_wait_accept,
    _fsm_pd_snk_sr_restart
};

void PE_SNK_Soft_Reset_fsm()
{
    fsm_result_e result;
    do
    {
       result=_fsm_pd_snk_soft_reset_handle[fsm_pd_snk_soft_reset](&fsm_pd_snk_soft_reset);
    }while (result==FSM_SWITCH_IMM);
}

static fsm_result_e _fsm_pd_snk_sr_send_soft_reset(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm)
{
    if(flag_snk_send_PD_msg==1)
    {
       PD_Protocol_Reset(); //Clear message ID
       pd.send_ctrl_msg(SOFT_RESET);
       LOGD("\r\n Tx:soft rst");
       flag_snk_send_PD_msg=0;
    }
    else switch(pd.PD_Transmit_Status_Get())//Read would clear this status
    {
        case TRANSMIT_FAIL:          tcpc_status.PDstate_SNK=PE_SNK_Hard_Reset;
                                     return FSM_SWITCH_IMM;
        case TRANSMIT_DISCARD:       flag_snk_send_PD_msg =1;
                                     break;   //retransmit
        case TRANSMIT_SUCCESS:
                                    *pd_snk_softreset_fsm=FSM_PD_SNK_SR_WAIT_ACCEPT;
                                    //Start_SenderResponseTimer(GTIMER_SEL3,25,PD_SNK_Send_Hard_Reset);
                                    Start_Timer_GTIMER_SEL3(25, TIMER_OPT_ONESHOT, PD_SNK_Send_Hard_Reset_Callback);
                                    return FSM_SWITCH_IMM;
        case TRANSMIT_ERROR:        flag_snk_send_PD_msg =1;       break;   //retransmit
        default  :                                                 break;   //
    }
    return FSM_WAITING;
}


static fsm_result_e _fsm_pd_snk_sr_wait_accept(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm)
{
    PD_MSG_t*  message_rx;
    if(pd.is_new_msg_received())
    {
        pd.clear_new_msg_received();
        message_rx=pd.PD_Msg_Get();
        if((message_rx->header.number_Data_Objects==0)&&(message_rx->header.extended==0)&&(message_rx->header.message_Type==ACCEPT))
        {
             //Stop_SenderResponseTimer(GTIMER_SEL3);
             //TIMER_SAFE_DELETE(GTIMER_SEL3);
             if(GTIMER_SEL3){
                 if(bat_timer_delete(GTIMER_SEL3, portMAX_DELAY)==pdPASS){
                     GTIMER_SEL3 = NULL;
                 }
             }
             *pd_snk_softreset_fsm=FSM_PD_SNK_SR_RESTART;
             return FSM_SWITCH_IMM;
        }
    }
    return FSM_WAITING;
}


static fsm_result_e _fsm_pd_snk_sr_restart(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm)
{
     //reset protocal layer
     *pd_snk_softreset_fsm=FSM_PD_SNK_SR_SEND_SOFTRRESET;
     tcpc_status.PDstate_SNK=PE_SNK_Discovery;
     return FSM_WAITING;
}


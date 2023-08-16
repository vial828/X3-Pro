#ifndef PD_SNK
#define PD_SNK

#include "stdint.h"
#include "app_pd_com.h"
#include "app_pd_general.h"
#include "batTimer.h"

typedef enum
{
    FSM_PD_SNK_SR_SEND_SOFTRRESET=0,
    FSM_PD_SNK_SR_WAIT_ACCEPT,
    FSM_PD_SNK_SR_RESTART
}pd_snk_soft_reset_adjust_fsm_e;


typedef enum
{
    PE_SNK_Default=0,
    PE_SNK_Startup,
    PE_SNK_Discovery,
    PE_SNK_Hard_Reset_Received,
    PE_SNK_Wait_for_Capablities,
    PE_SNK_Evaluate_Capablity,
    PE_SEN_Send_Request,
    PE_SNK_Wait_Accept,
    PE_SNK_Transition_Sink,
    PE_SNK_Ready,
    PE_SNK_Give_Sink_Cap,
    PE_SNK_Hard_Reset,
    PE_SNK_Soft_Reset,
    PE_SNK_Transition_to_Default,
    PE_SNK_Soft_Reset_Receive,
    PE_SNK_Send_NotSupported,
    PE_SNK_BIST_TEST_DATA,
    PE_SNK_BIST_Carried_Mode,
    PE_SNK_Give_Source_Cap,
    PE_SNK_VCONN_SWAP,
    PE_SNK_Hard_Reset_Wait,
    PE_SNK_Send_GetSRCCAP,
    PE_SNK_PR_SWAP,
    PE_SNK_Disable,
    PE_SNK_Send_Debug,
}PD_snk_state_e;


void  App_Vendor_PD_SNK(void);

void PD_SNK_Init(void);
void PD_Protocol_Reset(void);
void PD_SNK_Soft_Send_Accept_to_Soft_Reset(void);
void PD_SNK_Send_Soft_Reset(void);
void PD_SNK_Send_Soft_Reset_Callback(const ptimer_t tm);
void PD_SNK_Send_SOURCE_CAP(void);
void PD_SNK_Send_SINK_CAP(void);
void PD_SNK_Send_Not_Supported(void);
void PD_SNK_Send_Not_Supported_Callback(const ptimer_t tm);

void PD_SNK_Send_Hard_Reset(void);
void PD_SNK_Send_Hard_Reset_Callback(const ptimer_t tm);
void PE_SNK_Ready_act(void);
void PE_SNK_Evaluate_Capablity_act(void);

void  PD_Advertise_SNK_Capablity(void);
void PE_SNK_Select_Capability_act(void);
void PD_Send_Request(uint8_t number,uint16_t current);
void PD_SNK_Send_Soft_reset(void);
void PE_SNK_Wait_Accept_act(void);
void PE_SNK_Give_Sink_Cap_act(void);
void PE_SNK_Get_Source_Cap_act(void);

void PE_SNK_Soft_Reset_fsm(void);
void PD_SNK_Process_Protocol_Error(void);
void PE_SNK_Soft_Reset_Receive_act(void);
void PE_SNK_Transition_Sink_act(void);
void PD_SNK_Jump_to_Default(void);
void PD_SNK_Jump_to_Default_Callback(const ptimer_t tm);
void PE_SNK_Send_NotSupported_act(void);
void PE_SNK_BIST_Carried_Mode_act(void);
void PD_Advertise_SNK_Capablity(void);
void PE_SNK_Send_GetSRCCAP_act(void);
void PE_SNK_Send_Debug_act(void);


void  PE_SNK_Send_Control_Msg_act(CONTROL_MESSAGE_TYPE_e message_type,PD_snk_state_e success_path,uint8_t success_send_flag,PD_snk_state_e fail_path,uint8_t fail_send_flag);
void  PE_SNK_Send_Complx_Msg_act(void(*action)(),PD_snk_state_e success_path,uint8_t success_send_flag,PD_snk_state_e fail_path,uint8_t fail_send_flag);

static fsm_result_e _fsm_pd_snk_sr_send_soft_reset(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm);
static fsm_result_e _fsm_pd_snk_sr_wait_accept(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm);
static fsm_result_e _fsm_pd_snk_sr_restart(pd_snk_soft_reset_adjust_fsm_e* pd_snk_softreset_fsm);

#endif

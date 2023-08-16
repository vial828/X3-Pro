#ifndef TYPEC_H
#define TYPEC_H

#include "stdint.h"
#include "app_pd_general.h"
#include "batTimer.h"

typedef enum
{
    Unattached_DRP=0,
    Attachwait_SNK,
    Attached_SNK,//
    Try_SRC,
    Attachwait_SRC,
    Attached_SRC,
    Trywait_SNK,
    Unattached_SNK,
    Unattached_SRC
} TYPEC_STATE;


bool Action_Check_Key(void);
void  App_Typec_Init(void);
void  Choose_PDcommunication_CC(void);
void  Updata_CC_Status(void);
void Action_Unattached_DRP(void);
void  Action_Attachwait_SNK(void);
uint8_t Is_Unattached_SNK(void);
void  Jump_to_Trywait_SNK(void);
void  Action_Trywait_SNK(void);
void  Check_Vbus_to_Attached_SNK(void);
void Check_Vbus_to_Attached_SNK_Callback(const ptimer_t tm);
void Jump_to_Attached_SNK(void);
void  Action_Attached_SNK(void);
void  Jump_to_Unattached_SNK(void);
void  Jump_to_Unattached_SNK_Callback(const ptimer_t tm);
void Jump_to_DRP_from_SNK(void);
uint8_t Is_Unattached_SRC(void);
void  Check_Vbus_to_Try_SRC(void);
void  Action_Attachwait_SRC(void);
void  Check_Vbus_to_Attached_SRC(void);
void  Jump_to_Attached_SRC(void);
void  Action_Attached_SRC(void);
void  Jump_to_Unattached_SRC(void);
void  Action_Try_SRC(void);
void  App_TypeC_scan(void);
void  Stop_All_PD_Timer(void);

#endif

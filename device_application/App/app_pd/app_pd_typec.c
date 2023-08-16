#include    "app_pd_typec.h"
#include    "app_pd_snk.h"
#include    "HWI_Hal.h"
#include    "log.h"

//1 remove error at state:CC1-SRC.Rd ;

extern pd_status_t  pd_status;
extern pd_phy_t     pd;

//timer_t * GTIMER_SEL2 = NULL;
//timer_t * GTIMER_SEL3 = NULL;
//timer_t * GTIMER_SEL4 = NULL;
ptimer_t GTIMER_SEL2 = NULL;
ptimer_t GTIMER_SEL3 = NULL;
ptimer_t GTIMER_SEL4 = NULL;


void  Start_Timer_GTIMER_SEL2(uint32_t period, uint8_t opt, TIMER_CB_F callback){

    if(!GTIMER_SEL2)
    {
        //GTIMER_SEL2 =  timer_create(period,  opt, callback, NULL);
        GTIMER_SEL2 = bat_timer_reset_ext(GTIMER_SEL2, "GTIMER_SEL2", period,  opt, callback);
        bat_timer_start(GTIMER_SEL2, portMAX_DELAY);
    }
}

void  Start_Timer_GTIMER_SEL3(uint32_t period, uint8_t opt, TIMER_CB_F callback){

    if(!GTIMER_SEL3)
    {
        //GTIMER_SEL3 =  timer_create(period,  opt, callback, NULL);
        GTIMER_SEL3 = bat_timer_reset_ext(GTIMER_SEL3, "GTIMER_SEL3", period,  opt, callback);
        bat_timer_start(GTIMER_SEL3, portMAX_DELAY);
    }
}

void  Start_Timer_GTIMER_SEL4(uint32_t period, uint8_t opt, TIMER_CB_F callback){

    if(!GTIMER_SEL4)
    {
        //GTIMER_SEL4 =  timer_create(period,  opt, callback, NULL);
        GTIMER_SEL4 = bat_timer_reset_ext(GTIMER_SEL4, "GTIMER_SEL4", period,  opt, callback);
        bat_timer_start(GTIMER_SEL4, portMAX_DELAY);
    }
}

void  App_TypeC_scan(void)
{
        if(!Action_Check_Key())
        {
            if(tcpc_status.error_recovery==1)
            {
                pd.set_cc_role(DRP_MODE,RP_3A,Open_CC,Open_CC);
                //pd_pwr.OTG_Off();
                tcpc_status.error_recovery=0;
                //hwi_delay_ms(T_ERROR_ROCOVERY);//delay 25ms
                vTaskDelay(T_ERROR_ROCOVERY);
                tcpc_status.Now_TypeC_State= Unattached_SNK;
                pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);
                Updata_CC_Status();
            }
            switch(tcpc_status.Now_TypeC_State)
            {
                case Attachwait_SNK:
                    Action_Attachwait_SNK();
                    break;
                case Unattached_SNK:
                    Is_Unattached_SNK();
                    break;
                case Attached_SNK:
                    Action_Attached_SNK();
                    break;
                default:
                    Action_Attached_SNK();
                    break;
            }
        }
}


bool Action_Check_Key(void)
{
      return FALSE;
}


void  Choose_PDcommunication_CC(void)
{
    if(tcpc_status.CC1_PD == 1)
        pd.set_cc_plug(1);//choose CC1
    else if(tcpc_status.CC2_PD == 1)
        pd.set_cc_plug(2);//choose CC2
    else
        LOGD("communication_CC  error");
}

void Updata_CC_Status()
{
    if(tcpc_status.CC1_PD == 1)
        tcpc_status.CC_State = pd_status.CC1_State;
    else if(tcpc_status.CC2_PD == 1)
        tcpc_status.CC_State = pd_status.CC2_State;
    else
        LOGD("update_CC  error");
}

void Action_Unattached_DRP(void)
{
    Jump_to_Unattached_SNK();
}



void  Action_Attachwait_SNK()
{
    Updata_CC_Status();
    if(tcpc_status.CC_State != SNK_OPEN)
    {
        //Start_CCdebounce_Timer(GTIMER_SEL2,T_CCDEBOUNCE,Check_Vbus_to_Attached_SNK);
        Start_Timer_GTIMER_SEL2(T_CCDEBOUNCE, TIMER_OPT_ONESHOT, Check_Vbus_to_Attached_SNK_Callback);
    }
    else
    {
        //Stop_CCdebounce_Timer(GTIMER_SEL2);
        //TIMER_SAFE_DELETE(GTIMER_SEL2);
        if(GTIMER_SEL2){
            if(bat_timer_delete(GTIMER_SEL2, portMAX_DELAY)==pdPASS){
                GTIMER_SEL2 = NULL;
            }
        }
        tcpc_status.Now_TypeC_State = Unattached_SNK;
        pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);
        LOGD("\r\n unttached_snk");
    }
}

uint8_t Is_Unattached_SNK(void)
{
    if(pd_status.CC1_State!=SNK_OPEN && pd_status.CC2_State==SNK_OPEN)
    {
        tcpc_status.CC1_PD =1;
        tcpc_status.CC2_PD =0;
        tcpc_status.Now_TypeC_State = Attachwait_SNK;
        LOGD("\r\n unattachwait_snk");
        return 0;
    }
    else if(pd_status.CC1_State==SNK_OPEN && pd_status.CC2_State!=SNK_OPEN)
    {
        tcpc_status.CC1_PD =0;
        tcpc_status.CC2_PD =1;
        tcpc_status.Now_TypeC_State = Attachwait_SNK;
        LOGD("\r\n unattachwait_snk");
        return 0;
    }
    return 1;
}

void Check_Vbus_to_Attached_SNK_Callback(const ptimer_t tm)
{
    if(GTIMER_SEL2){
        if(bat_timer_delete((GTIMER_SEL2), portMAX_DELAY)==pdPASS){
            (GTIMER_SEL2) = NULL;
        }
    }

    Check_Vbus_to_Attached_SNK();
}

void Check_Vbus_to_Attached_SNK()
{

    //Stop_CCdebounce_Timer(GTIMER_SEL2);
    //TIMER_SAFE_DELETE(GTIMER_SEL2);
    if(GTIMER_SEL2){
        if(bat_timer_delete(GTIMER_SEL2, portMAX_DELAY)==pdPASS){
            GTIMER_SEL2 = NULL;
        }
    }
    if(pd.is_vbus_ok())    //Vbus_present
    {
         Jump_to_Attached_SNK();
         tcpc_status.Now_TypeC_State = Attached_SNK;
         LOGD("\r\n Attaed_SNK");
    }
    else
    {
         // LOGD("checkingVBus");
         //Start_CCdebounce_Timer(GTIMER_SEL2,10,Check_Vbus_to_Attached_SNK);//check vbus every 10ms
         Start_Timer_GTIMER_SEL2(10, TIMER_OPT_ONESHOT, Check_Vbus_to_Attached_SNK_Callback);
    }
}

void Jump_to_Attached_SNK()
{
    Choose_PDcommunication_CC();
    //pd_pwr.Input_Voltage_Set(4500);//Set VDPM/IDPM CHARGE_EN
    //pd_pwr.Input_Current_Set(500); //5V 500mA
    tcpc_status.PDstate_SNK=0;     //
}


void  Action_Attached_SNK()
{
    if(tcpc_status.cc_vbus_update_en==1)
        Updata_CC_Status();
    if((tcpc_status.CC_State == SNK_OPEN) || ((!pd.is_vbus_ok()) && tcpc_status.hard_resrt_on==0 &&(tcpc_status.cc_vbus_update_en) )) //Vbus is not present or CC Open
    {
        //Start_PDdebounce_Timer(GTIMER_SEL2,T_PDDEBOUNCE,Jump_to_Unattached_SNK);
        Start_Timer_GTIMER_SEL2(T_PDDEBOUNCE, TIMER_OPT_ONESHOT, Jump_to_Unattached_SNK_Callback);
    }
    else
    {
        //Stop_PDdebounce_Timer(GTIMER_SEL2);
        //TIMER_SAFE_DELETE(GTIMER_SEL2);
        if(GTIMER_SEL2){
            if(bat_timer_delete(GTIMER_SEL2, portMAX_DELAY)==pdPASS){
                GTIMER_SEL2 = NULL;
            }
        }
    }
    App_Vendor_PD_SNK();
}

void Jump_to_Unattached_SNK_Callback(const ptimer_t tm)
{
    if(GTIMER_SEL2){
        if(bat_timer_delete((GTIMER_SEL2), portMAX_DELAY)==pdPASS){
            (GTIMER_SEL2) = NULL;
        }
    }

    Jump_to_Unattached_SNK();
}

void  Jump_to_Unattached_SNK()
{
    Stop_All_PD_Timer();

    pd.vcon_off();
    tcpc_status.Now_TypeC_State = Unattached_SNK;
    pd.enable_pd_receive(EN_HARD_RESET);    //Disbale receive SOP

    tcpc_status.CC1_PD =0;
    tcpc_status.CC2_PD =0;
    LOGD("\r\n Back to Unattached_SNK");
}


void  App_Typec_Init(void)
{
    pd.enable_pd_receive(EN_HARD_RESET);
    tcpc_status.Now_TypeC_State= Unattached_SNK;
    tcpc_status.power_role=(POWER_ROLE_TYPE_e)SNK_ROLE;
    tcpc_status.data_role =(DATA_ROLE_TYPE_e)UFP;
    tcpc_status.pd_rev=REV_3;
    tcpc_status.CC1_PD=1;
    pd.set_cc_role(NO_DRP_MODE,RP_3A,RD_CC,RD_CC);
}


void Stop_All_PD_Timer(void)
{
    //TIMER_SAFE_DELETE(GTIMER_SEL2);
    //TIMER_SAFE_DELETE(GTIMER_SEL3);
    //TIMER_SAFE_DELETE(GTIMER_SEL4);
    if(GTIMER_SEL2){
        if(bat_timer_delete(GTIMER_SEL2, portMAX_DELAY)==pdPASS){
            GTIMER_SEL2 = NULL;
        }
    }
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
}

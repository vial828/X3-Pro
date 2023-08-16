#include  "app_pd_cfg.h"

source_cap_t  source_output_capa[SUPPLY_TYPE_SUPPORTED_MAX];
sink_cap_t    sink_output_capa[REQUEST_TYPE_SUPPORTED_MAX];
cable_t       cable_capa;


void PD_Power_Source_Output_Mode_Init(void)
{
    /*  Note:1.If the USB Suspend Supported flag is cleared, then the Sink Shall Not apply the [USB 2.0] or [USB 3.2] rules for
    suspend and May continue to draw the negotiated power.
    2.
    */
    //init 5V3A
    source_output_capa[0].w=DUAL_ROLE_POWER_1|USB_SUSPEND_SUPPORT_0|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
                            DUAL_ROLE_DATA_0|UNCHUNKED_EXTENDED_MSG_SUPPORTED_0|EPR_MODE_CAPABLE_0|FIXED_SUPPLY_TYPE;
    source_output_capa[0].fixed.current_max_10ma=300;
    source_output_capa[0].fixed.voltage_50mv=100;

    //init 9V3A
    source_output_capa[1].w=DUAL_ROLE_POWER_0|USB_SUSPEND_SUPPORT_0|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
                               DUAL_ROLE_DATA_0|UNCHUNKED_EXTENDED_MSG_SUPPORTED_0|EPR_MODE_CAPABLE_0|FIXED_SUPPLY_TYPE;
    source_output_capa[1].fixed.current_max_10ma=300;
    source_output_capa[1].fixed.voltage_50mv=180;


    #ifdef   PPS_ENABLE
    //init  PPS 3-12V 3A
    source_output_capa[2].w=SPR_PPS|APDO_SUPPLY_TYPE;
    source_output_capa[2].pps.current_max_50ma=60;
    source_output_capa[2].pps.minimum_voltage_100mv=30;
    source_output_capa[2].pps.maximum_voltage_100mv=120;
    #else
    //12V 2A
    source_output_capa[2].w=DUAL_ROLE_POWER_0|USB_SUSPEND_SUPPORT_0|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
                               DUAL_ROLE_DATA_0|UNCHUNKED_EXTENDED_MSG_SUPPORTED_0|EPR_MODE_CAPABLE_0|FIXED_SUPPLY_TYPE;
    source_output_capa[2].fixed.current_max_10ma=200;
    source_output_capa[2].fixed.voltage_50mv=240;
    #endif
}

 source_cap_t* PD_Get_Source_Capa(void)
{
    return source_output_capa;
}

void App_PD_Power_Sink_Capability_Init(void)
{
     /*
       Note 1:In the case that the Sink needs more than vSafe5V (e.g., 12V) to provide full functionality, then the Higher Capability bit Shall be set.
          2.
    */
     //init 5V3A
     sink_output_capa[0].w=FIXED_SUPPLY_TYPE|DUAL_ROLE_POWER_0|HIGHER_CAPABILITY_1|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
                            DUAL_ROLE_DATA_0|FAST_ROLE_SWAP_CURRENT_0;
     sink_output_capa[0].fixed.operational_current_10ma    =300;
     sink_output_capa[0].fixed.voltage_50mv=100;

    //init 9V3A
     sink_output_capa[1].w=FIXED_SUPPLY_TYPE|DUAL_ROLE_POWER_0|HIGHER_CAPABILITY_0|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
                            DUAL_ROLE_DATA_0|FAST_ROLE_SWAP_CURRENT_0;
     sink_output_capa[1].fixed.operational_current_10ma=300;  //250
     sink_output_capa[1].fixed.voltage_50mv=180;

     //init 12V3A
     sink_output_capa[2].w=FIXED_SUPPLY_TYPE|DUAL_ROLE_POWER_0|HIGHER_CAPABILITY_0|UNCONSTRAINED_POWER_0|USB_COMMUNICATION_CAPABLE_0|
                            DUAL_ROLE_DATA_0|FAST_ROLE_SWAP_CURRENT_0;
     sink_output_capa[2].fixed.operational_current_10ma=300;
     sink_output_capa[2].fixed.voltage_50mv=240;

}

 sink_cap_t*  App_PD_Get_Sink_Capa(void)
{
    return sink_output_capa;
}

 cable_t*  PD_Get_Cable_Capa(void)
{
    return &cable_capa;
}


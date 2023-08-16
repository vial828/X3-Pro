#include <stdint.h>

#define FIXED_SUPPLY_TYPE                            0
#define APDO_SUPPLY_TYPE                         (0x3UL<<30)
#define SUPPLY_TYPE_MASK                         (0x3UL<<30)

#define DUAL_ROLE_POWER_0                            0
#define DUAL_ROLE_POWER_1                       (1<<29)
#define USB_SUSPEND_SUPPORT_0                        0
#define USB_SUSPEND_SUPPORT_1                   (1<<28)
#define UNCONSTRAINED_POWER_0                        0
#define UNCONSTRAINED_POWER_1                   (1<<27)
#define USB_COMMUNICATION_CAPABLE_0                  0
#define USB_COMMUNICATION_CAPABLE_1             (1<<26)
#define DUAL_ROLE_DATA_0                             0
#define DUAL_ROLE_DATA_1                        (1<<25)
#define UNCHUNKED_EXTENDED_MSG_SUPPORTED_0           0
#define UNCHUNKED_EXTENDED_MSG_SUPPORTED_1      (1<<24)
#define EPR_MODE_CAPABLE_0                           0
#define EPR_MODE_CAPABLE_1                      (1<<23)
#define PEAK_CURRENT_0                               0

#define SPR_PPS                                      0
#define PPS_POWER_LMITED_0                           0
#define PPS_POWER_LMITED_1                          (1<<27)

#define HIGHER_CAPABILITY_0                          0
#define HIGHER_CAPABILITY_1                          (1<<28)
#define FAST_ROLE_SWAP_CURRENT_0                     (0<<23)
#define FAST_ROLE_SWAP_CURRENT_1                     (1<<23)
#define FAST_ROLE_SWAP_CURRENT_2                     (2<<23)
#define FAST_ROLE_SWAP_CURRENT_3                     (3<<23)



#define     SUPPLY_TYPE_SUPPORTED_MAX         3
#define     SUPPLY_PPS_SUPPORTED_NUM          1
#define     REQUEST_TYPE_SUPPORTED_MAX        3

// typedef u32  uint32_t;
typedef union
{
    uint32_t w;
    struct{
        uint32_t  data_size                     :9;
        uint32_t  reserve                       :1;
        uint32_t  request_chunk                 :2;
        uint32_t  chunk_number                  :4;
        uint32_t  chunked                       :1;
    }bit;

}extended_header_t;

typedef union
{
    uint32_t w;
    struct{
        uint32_t   output_voltage        :16;
        uint32_t   output_current        :8;
        uint32_t   reserve1              :4;
        uint32_t   OMF                   :1;
        uint32_t   PTF                   :2;
        uint32_t   reserve               :1;
    }bit;
}pps_status_t;


typedef union
{
    uint32_t w;
    struct{
        uint32_t  current_max_10ma              :10;
        uint32_t  voltage_50mv                  :10;
        uint32_t  peak_current                  :2;
        uint32_t  reserved                      :1;
        uint32_t  epr_mode_capable              :1;
        uint32_t  unchunked_support             :1;
        uint32_t  dual_role_data                :1;
        uint32_t  usb_communication_capable     :1;
        uint32_t  unconstained_power            :1;
        uint32_t  usb_suspend_support           :1;
        uint32_t  dual_role_power               :1;
        uint32_t  power_type                    :2;
    }fixed;

    struct{
        uint32_t  current_max_50ma              :7;
        uint32_t  reserved                      :1;
        uint32_t  minimum_voltage_100mv         :8;
        uint32_t  reserved1                     :1;
        uint32_t  maximum_voltage_100mv         :8;
        uint32_t  reserved2                     :2;
        uint32_t  pps_power_limited             :1;
        uint32_t  reserved3                     :2;
        uint32_t  power_type                    :2;
    }pps;
}source_cap_t;

typedef union
{
    uint32_t w;
    struct{
          uint32_t operational_current_10ma         :10;
          uint32_t voltage_50mv                     :10;
          uint32_t reserved                         :3 ;
          uint32_t fast_role_swap_current           :2 ;
          uint32_t dual_role_data                   :1 ;
          uint32_t  usb_communication_capable        :1 ;
          uint32_t  unconstained_power               :1;
          uint32_t  higher_capability                :1;
          uint32_t  dual_role_power                 :1;
          uint32_t  power_type                      :2;
    }fixed;

    struct{
        uint32_t  current_max_50ma              :7;
        uint32_t  reserved                      :1;
        uint32_t  minimum_voltage_100mv         :8;
        uint32_t  reserved1                     :1;
        uint32_t  maximum_voltage_100mv         :8;
        uint32_t  reserved2                     :3;
        uint32_t  spr_pps                       :2;
        uint32_t  power_type                    :2;
    }pps;
}sink_cap_t;

typedef union
{
    uint32_t w;
    struct{
          uint32_t max_min_current_10ma                  :10; //Giveback_flag=1,minimum current;/Giveback_flag=0,maximum current;
          uint32_t operating_current_10mA                :10;
          uint32_t reserved                              :2 ;
          uint32_t erp_mode_capable                      :1;
          uint32_t unchunked_extended_message_support    :1 ;
          uint32_t no_usb_suspend                        :1 ;
          uint32_t usb_communications_capable            :1;
          uint32_t capablity_mismatch                    :1;
          uint32_t  giveback_flag                        :1;
          uint32_t  object_position                      :4;
   }fixed;

  struct{
        uint32_t  operating_current_50ma                :7;
        uint32_t  reserved                              :2;
        uint32_t  voltage_20mv                          :12;
        uint32_t  reserved1                             :1;
        uint32_t  erp_mode_capable                      :1;
        uint32_t  unchunked_extended_message_support    :1;
        uint32_t  no_usb_suspend                        :1;
        uint32_t  usb_communications_capable            :1;
        uint32_t  capablity_mismatch                    :1;
        uint32_t  giveback_flag                         :1;
        uint32_t  object_position                       :4;
    }pps;
}request_t;



typedef union
{
    uint32_t w;
    struct{
          uint32_t usb_highest_speed                  :3;
          uint32_t  reserve                           :2;
          uint32_t  current_capablity                 :2;
          uint32_t  reserve1                          :2;
          uint32_t  maximum_vbus                      :2;
          uint32_t  cable_termination_type            :2;
          uint32_t  cable_latency                     :1;
          uint32_t  epr_mode_capable                  :1;
          uint32_t  typec_or_capative                 :1;
          uint32_t  reserve2                          :1;
          uint32_t  vdo_version                       :3;
          uint32_t   firmware_version                 :4;
          uint32_t   hw_version                       :4;
   }passive_cable_t;
}cable_t;

typedef union
{
      uint32_t w;
      struct{
          uint32_t  command                            :5;
          uint32_t  reserve                            :1;
          uint32_t  command_type                       :2;
          uint32_t  object_position                    :3;
          uint32_t  reserve1                           :2;
          uint32_t  vdm_version                        :2;
          uint32_t  vdm_type                           :1;
          uint32_t  svid                              :16;
    }bit;
}vdm_header_t;

typedef union
{
      uint32_t w;
      struct{
          uint32_t  usb_vender_id                           :16;
          uint32_t  reserve                                 :5;
          uint32_t  connector_type                          :2;
          uint32_t  sop_product_type                        :3;
          uint32_t  modal_operation_supported               :1;
          uint32_t  product_type                            :3;
          uint32_t  usb_communication_capable_usb_device    :1;
          uint32_t  usb_communication_capable_usb_host      :1;
    }bit;
}id_header_vdo_t;

void          PD_Power_Source_Output_Mode_Init(void);
source_cap_t* PD_Get_Source_Capa(void);
void          App_PD_Power_Sink_Capability_Init(void);
sink_cap_t*   App_PD_Get_Sink_Capa(void);
cable_t*      PD_Get_Cable_Capa(void);


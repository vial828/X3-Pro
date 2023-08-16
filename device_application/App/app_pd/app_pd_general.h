#ifndef pd_general_h
#define pd_general_h

//#pragma anon_unions

#define EN_SOP         (uint8_t)(0x01)
#define EN_SOP1        (uint8_t)(0x01<<1)
#define EN_SOP2        (uint8_t)(0x01<<2)
#define EN_SOP1_DEBUG  (uint8_t)(0x01<<3)
#define EN_SOP2_DEBUG  (uint8_t)(0x01<<4)
#define EN_HARD_RESET  (uint8_t)(0x01<<5)
#define EN_CABLE_RESET (uint8_t)(0x01<<6)

typedef enum
{
    FALSE = 0,
    TRUE  = 1,
}bool;

typedef enum
{
    REV_1 = 0,
    REV_2 = 1,
    REV_3 = 2,
    REV_RESERVED = 3,
} SPEC_REV_TYPE_e;

typedef enum
{
    UFP = 0,
    DFP = 1,
} DATA_ROLE_TYPE_e;

typedef enum
{
    SNK_ROLE   = 0,
    SRC_ROLE   = 1,
} POWER_ROLE_TYPE_e;

typedef enum
{
      CABLE_PLUG=1,
    NOT_CABLE=0,
}CABLE_PLUG_TYPE_e;

typedef enum
{
    RP_DEFAULT = 0,
    RP_1P5A    = 1,
    RP_3A      = 2,
    RESERVE_RP
} RP_VALUE_TYPE_e;

typedef enum
{
    Reserve_CC = 0,
    RP_CC = 1,
    RD_CC= 2,
    Open_CC = 3
} CC_TYPE_e;

typedef enum
{
    SRC_OPEN = 0,
    SRC_Ra = 1,
    SRC_Rd = 2,
    SRC_Reserved = 3,
} CC_STATETYPE_Rp;

typedef enum
{
    SNK_OPEN = 0,
    SNK_Default = 1,
    SNK_Power_1p5 = 2,
    SNK_Power_3 = 3,
} CC_STATETYPE_Rd;

typedef enum
{
    TYPE_SOP = 0,
    TYPE_SOPP = 1,
    TYPE_SOPPP = 2,
    TYPE_SOP_DBGG = 3,
    TYPE_SOP_DBGGG = 4,
    TYPE_CABLE_RESET = 6
} SOP_TYPE_e; //3bits,B2..0

typedef enum
{
    DRP_MODE = 1,
    NO_DRP_MODE =0,
} DRP_EN_e;

typedef enum
{
    UNRECGNITED = 0,
    GOODCRC = 1,
    GOTOMIN= 2,
    ACCEPT = 3,
    REJECT = 4,
    PING = 5,
    PS_RDY = 6,
    GET_SOURCE_CAP = 7,
    GET_SINK_CAP = 8,
    DR_SWAP = 9,
    PR_SWAP= 10,
    VCONN_SWAP = 11,
    WAIT = 12,
    SOFT_RESET = 13,
    DATA_RESET = 14,
    DATA_RESET_COMPLETE = 15,
    NOT_SUPPORTED = 16,
    GET_SOURCE_CAP_EXTEND = 17,
    GET_STATUS_PD = 18,
    FR_SWAP = 19,
    GET_PPS_STATUS = 20,
    GET_COUNTRY_CODES = 21,
    GET_SINK_CAP_EXTENED = 22,
    GET_SOURCE_INFO=23,
    GET_REVISION=24,
} CONTROL_MESSAGE_TYPE_e;

typedef enum
{
    PERIOD_51_2ms=0,
    PERIOD_57_6ms,
    PERIOD_64ms,
    PERIOD_70_4ms,
    PERIOD_76_8ms,
    PERIOD_83_2ms,
    PERIOD_89_6ms,
    PERIOD_96ms,
    PERIOD_102_4ms,
    PERIOD_108_8ms,
    PERIOD_115_2ms,
    PERIOD_121_6ms,
    PERIOD_128ms,
    PERIOD_134_4ms,
    PERIOD_140_8ms,
    PERIOD_147_2ms
}DRP_PERIOD_e;

//DATA_ROLE_TYPE_e data_role_type,POWER_ROLE_TYPE_e pwr_role_type,SPEC_REV_TYPE_e
typedef struct
{
    uint8_t   CC_State;//Read only
    uint8_t   Now_TypeC_State;
    uint8_t   CC1_PD;
    uint8_t   CC2_PD;
    uint8_t   PDstate_SNK;
    uint8_t   PDstate_SRC;
    uint8_t   PD_Connected;
    uint8_t   PPS_Mode;
    uint8_t   capscounter;
    uint8_t   hardresetcounter;
    uint8_t   Explict_Contract;//used in pd_snk.c when receive reject or wait for request
    DATA_ROLE_TYPE_e    data_role;
    POWER_ROLE_TYPE_e    power_role;
    uint8_t    messageID;
    SPEC_REV_TYPE_e    pd_rev; //´æ´¢µ±Ç°PD°æ±¾
    uint8_t    above5V;// preparing for BIST carry mode
    uint8_t    hard_resrt_on;
    uint8_t    drp_state;//0x01:SNK   0x02:SRC   0x03:DRP
    int8_t     rx_stored_msgID;
    uint16_t   request_voltage; //used in pd.src
    uint16_t   request_current; //used in pd.src
    uint16_t   request_voltage_history;//check PPS SMALl or LARGE step
    uint8_t    cc_vbus_update_en;
    uint8_t    flag_snk_to_src;//used in prswap
    uint8_t    flag_src_to_snk;//used in prswap
    uint8_t    error_recovery; //used in prswap
    uint8_t    ra;
} TCPC_STATUS_Type;

typedef enum
{
    SOURCE_CAPABILITIES = 1,
    REQUEST = 2,
    BIST = 3,
    SINK_CAPABILITIES = 4,
    BATTERY_STATUS = 5,
    ALERT = 6,
    GET_COUNTRY_INFO = 7,
    ENTER_USB = 8,
    VENDER_DEFINED = 15
} DATA_MESSAGE_TYPE_e;

typedef enum
{
    SOURCE_CAPABILITIES_EXTENDED = 1,
    STATUS = 2,
    GET_BATTERY_CAP = 3,
    GET_BATTERY_STATUS = 4,
    BATTERY_CAPABILITIES = 5,
    GET_MANUFACTURER_INFO = 6,
    MANUFACTURER_INFO = 7,
    SECURITY_REQUES = 8,
    SECURITY_RESPONSE = 9,
    FIRMWARE_UPDATE_REQUEST = 10,
    FIRMWARE_UPDATE_RESPONSE = 11,
    PPS_STATUS = 12,
    COUNTRY_INFO = 13,
    COUNTRY_CODES = 14,
    SINK_CAPABILITIES_EXTENDED = 15
} EXTENDED_DATA_MESSAGE_TYPE_e;

typedef union
{
    uint32_t w;
    struct
    {
        uint32_t message_Type:5;
        uint32_t port_Data_Role :1;
        uint32_t spec_Rev:2;
        uint32_t port_Power_Role:1;
        uint32_t messageID:3;
        uint32_t number_Data_Objects:3;
        uint32_t extended:1;
        uint32_t reserve:16;
    };
}PD_MSG_HEADER_t;

typedef struct
{
    PD_MSG_HEADER_t header;
    uint32_t data[7];
    SOP_TYPE_e Sop_Type;
}PD_MSG_t;

typedef struct
{
    uint8_t    POWER_STATUS_REG;//read only
    uint16_t   Vendor_STASTUS_REG;//Read only
    uint8_t    Looking4Connection;//Read only
    uint8_t    ConnectResult;//Read only
    uint8_t    CC1_State;//Read only
    uint8_t    CC2_State;//Read only
    uint8_t    Vsafe0V;//Read only
    uint8_t    vconn_present;//read only
    uint8_t    R_HARD_RESET; //used in app_pd_com.h
    uint8_t    VCONN_ON;     //read only
} pd_status_t;

typedef enum
{
    TRANSMIT_NONE=0,
    TRANSMIT_FAIL,
    TRANSMIT_DISCARD,
    TRANSMIT_SUCCESS,
    TRANSMIT_ERROR

}PDTRANS_STATUS_e;


typedef struct
{
    uint8_t Transmit_Hard_Reset_Success;
    uint8_t Transmit_Fail;
    uint8_t Transmit_Discard;
    uint8_t Transmit_Success;
    uint8_t I2C_error;
    PDTRANS_STATUS_e transmit_status;
}tx_result_t;


typedef enum
{
    Rp = 0,
    Rd = 1,
} ConnectResult_Type;

extern  TCPC_STATUS_Type  tcpc_status;

#endif


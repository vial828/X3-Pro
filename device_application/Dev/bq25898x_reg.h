#ifndef __BQ25898X_REG__
#define __BQ25898X_REG__

//#define  BQ25898_ADDR     (0x6B << 1)
//#define  BQ25898D_ADDR    (0x6A << 1)
#define  BQ25898_ADDR     (0x6B )
#define  BQ25898D_ADDR    (0x6A )

#define  BQ25898X_ADDR    BQ25898D_ADDR
#define BQ25898E_REG_NUMS  21

/* Register 00h */
#define BQ25898X_REG_00              0x00
#define BQ25898X_ENHIZ_MASK          0x80
#define BQ25898X_ENHIZ_SHIFT         7
#define BQ25898X_ENHIZ_ENABLE        1
#define BQ25898X_ENHIZ_DISABLE       0

#define BQ25898X_ENILIM_MASK         0x40
#define BQ25898X_ENILIM_SHIFT        6
#define BQ25898X_ENILIM_ENABLE       1
#define BQ25898X_ENILIM_DISABLE      0

#if (BQ25898X_ADDR == BQ25898D_ADDR)
    #define BQ25898X_IINLIM_MASK         0x3F
    #define BQ25898X_IINLIM_SHIFT        0
    #define BQ25898X_IINLIM_BASE         0
    #define BQ25898X_IINLIM_LSB          50
#endif

/* Register 01h */
#define BQ25898X_REG_01              0x01

#if (BQ25898X_ADDR == BQ25898D_ADDR)
    #define BQ25898X_DPLUS_MASK          0xE0
    #define BQ25898X_DPLUS_SHIFT         5
    #define BQ25898X_DPLUS_HIZ           0x00
    #define BQ25898X_DPLUS_0V            0x01
    #define BQ25898X_DPLUS_0P6V          0x02
    #define BQ25898X_DPLUS_1P2V          0x03
    #define BQ25898X_DPLUS_2P0V          0x04
    #define BQ25898X_DPLUS_2P7V          0x05
    #define BQ25898X_DPLUS_3P3V          0x06
    #define BQ25898X_DPLUS_SHORT         0x07

    #define BQ25898X_DMINUS_MASK         0x1C
    #define BQ25898X_DMINUS_SHIFT        2
    #define BQ25898X_DMINUS_HIZ          0x00
    #define BQ25898X_DMINUS_0V           0x01
    #define BQ25898X_DMINUS_0P6V         0x02
    #define BQ25898X_DMINUS_1P2V         0x03
    #define BQ25898X_DMINUS_2P0V         0x04
    #define BQ25898X_DMINUS_2P7V         0x05
    #define BQ25898X_DMINUS_3P3V         0x06
#endif

#define BQ25898X_EN_12V_BIT_MASK     0x02
#define BQ25898X_EN_12V_BIT_SHIFT    1
#define BQ25898X_EN_12V_ENABLE       1
#define BQ25898X_EN_12V_DISABLE      0

#define BQ25898X_VDPM_OS_BIT_MASK    0x01
#define BQ25898X_VDPM_OS_BIT_SHIFT   0
#define BQ25898X_VDPM_OS_0P6A        0x01
#define BQ25898X_VDPM_OS_0P4A        0x00

/* Register 0x02 */
#define BQ25898X_REG_02               0x02
#define BQ25898X_CONV_START_MASK      0x80
#define BQ25898X_CONV_START_SHIFT     7
#define BQ25898X_CONV_START_ENABLE    1
#define BQ25898X_CONV_START_DISABLE   0

#define BQ25898X_CONV_RATE_MASK       0x40
#define BQ25898X_CONV_RATE_SHIFT      6
#define BQ25898X_CONV_RATE_ONE_SHOT   0
#define BQ25898X_CONV_RATE_CONTINUE   1

#define BQ25898X_BOOST_FREQ_MASK      0x20
#define BQ25898X_BOOST_FREQ_SHIFT     5
#define BQ25898X_BOOST_FREQ_1500K     0
#define BQ25898X_BOOST_FREQ_500K      1

#define BQ25898X_ICOEN_MASK          0x10
#define BQ25898X_ICOEN_SHIFT         4
#define BQ25898X_ICO_ENABLE          1
#define BQ25898X_ICO_DISABLE         0

/* High Voltage DCP Enable */
#define BQ25898X_HVDCPEN_MASK        0x08
#define BQ25898X_HVDCPEN_SHIFT       3
#define BQ25898X_HVDCP_ENABLE        1
#define BQ25898X_HVDCP_DISABLE       0

#define BQ25898X_MAXCEN_MASK         0x04
#define BQ25898X_MAXCEN_SHIFT        2
#define BQ25898X_MAXC_ENABLE         1
#define BQ25898X_MAXC_DISABLE        0

#define BQ25898X_FORCE_DPDM_MASK     0x02
#define BQ25898X_FORCE_DPDM_SHIFT    1
#define BQ25898X_FORCE_DPDM          1

#define BQ25898X_AUTO_DPDM_EN_MASK   0x01
#define BQ25898X_AUTO_DPDM_EN_SHIFT  0
#define BQ25898X_AUTO_DPDM_ENABLE    1
#define BQ25898X_AUTO_DPDM_DISABLE   0

/* Register 0x03 */
#define BQ25898X_REG_03              0x03

#define BQ25898X_VOK_OTG_EN_MASK     0x80
#define BQ25898X_FORCE_DSEL_MASK     0x80
#define BQ25898X_BAT_LOAEN_SHIFT     7

#define BQ25898X_WDT_RESET_MASK      0x40
#define BQ25898X_WDT_RESET_SHIFT     6
#define BQ25898X_WDT_RESET           1

#define BQ25898X_OTG_CONFIG_MASK     0x20
#define BQ25898X_OTG_CONFIG_SHIFT    5
#define BQ25898X_OTG_ENABLE          1
#define BQ25898X_OTG_DISABLE         0

#define BQ25898X_CHG_CONFIG_MASK     0x10
#define BQ25898X_CHG_CONFIG_SHIFT    4
#define BQ25898X_CHG_ENABLE          1
#define BQ25898X_CHG_DISABLE         0

#define BQ25898X_SYS_MINV_MASK       0x0E
#define BQ25898X_SYS_MINV_SHIFT      1
#define BQ25898X_SYS_MINV_BASE       3000
#define BQ25898X_SYS_MINV_LSB        100

 /* 2.9V BAT falling */
#define BQ25898X_MIN_VBAT_SEL_MASK   0x01
#define BQ25898X_MIN_VBAT_SEL_SHIFT   0
#define BQ25898X_MIN_VBAT_2900        0
#define BQ25898X_MIN_VBAT_2500        1

/* Register 0x04*/
#define BQ25898X_REG_04              0x04
 /* Current pulse control Enable */
#define BQ25898X_EN_PUMPX_MASK       0x80
#define BQ25898X_EN_PUMPX_SHIFT      7
#define BQ25898X_PUMPX_ENABLE        1
#define BQ25898X_PUMPX_DISABLE       0

/* Fast Charge Current Limit */
#define BQ25898X_ICHG_MASK           0x7F
#define BQ25898X_ICHG_SHIFT          0
#define BQ25898X_ICHG_BASE           0
#define BQ25898X_ICHG_LSB            64

/* Register 0x05 */
#define BQ25898X_REG_05              0x05
/* Precharge Current Limit */
#define BQ25898X_IPRECHG_MASK        0xF0
#define BQ25898X_IPRECHG_SHIFT       4
#define BQ25898X_IPRECHG_BASE        64
#define BQ25898X_IPRECHG_LSB         64

/* Termination Current Limit */
#define BQ25898X_ITERM_MASK          0x0F
#define BQ25898X_ITERM_SHIFT         0
#define BQ25898X_ITERM_BASE          64
#define BQ25898X_ITERM_LSB           64

/* Register 0x06*/
#define BQ25898X_REG_06              0x06
/* Full Charge Voltage Limit */
#define BQ25898X_VREG_MASK           0xFC
#define BQ25898X_VREG_SHIFT          2
#define BQ25898X_VREG_BASE           3840
#define BQ25898X_VREG_LSB            16

/* Battery Precharge to Fast Charge Threshold */
#define BQ25898X_BATLOWV_MASK        0x02
#define BQ25898X_BATLOWV_SHIFT       1
#define BQ25898X_BATLOWV_2800MV      0
#define BQ25898X_BATLOWV_3000MV      1

/* Battery Recharge Threshold Offset */
#define BQ25898X_VRECHG_MASK         0x01
#define BQ25898X_VRECHG_SHIFT        0
#define BQ25898X_VRECHG_100MV        0
#define BQ25898X_VRECHG_200MV        1

/* Register 0x07*/
#define BQ25898X_REG_07              0x07
/* Charging Termination Enable */
#define BQ25898X_EN_TERM_MASK        0x80
#define BQ25898X_EN_TERM_SHIFT       7
#define BQ25898X_TERM_ENABLE         1
#define BQ25898X_TERM_DISABLE        0

/* STAT Pin Disable */
#define BQ25898X_STAT_DIS_MASK        0x40
#define BQ25898X_STAT_DIS_SHIFT       6
#define BQ25898X_STAT_DIS_ENABLE      0
#define BQ25898X_STAT_DIS_DISABLE     1

/* I2C Watchdog Timer Setting */
#define BQ25898X_WDT_MASK            0x30
#define BQ25898X_WDT_SHIFT           4
#define BQ25898X_WDT_DISABLE         0x00
#define BQ25898X_WDT_40S             0x01
#define BQ25898X_WDT_80S             0x02
#define BQ25898X_WDT_160S            0x03

/* Charging Safety Timer Enable */
#define BQ25898X_EN_TIMER_MASK       0x08
#define BQ25898X_EN_TIMER_SHIFT      3
#define BQ25898X_EN_TIMER_ENABLE     1
#define BQ25898X_EN_TIMER_DISABLE    0

/* Fast Charge Timer Setting */
#define BQ25898X_CHG_TIMER_MASK      0x06
#define BQ25898X_CHG_TIMER_SHIFT     1
#define BQ25898X_CHG_TIMER_5HOURS    0x00
#define BQ25898X_CHG_TIMER_8HOURS    0x01
#define BQ25898X_CHG_TIMER_12HOURS   0x02
#define BQ25898X_CHG_TIMER_20HOURS   0x03

/* JEITA Low Temperature Current Setting */
#define BQ25898X_JEITA_ISET_MASK     0x01
#define BQ25898X_JEITA_ISET_SHIFT    0
#define BQ25898X_JEITA_ISET_50PCT    0
#define BQ25898X_JEITA_ISET_20PCT    1

/* Register 0x08*/
#define BQ25898X_REG_08              0x08
/* IR Compensation Resistor Setting */
#define BQ25898X_RES_COMP_MASK       0xE0
#define BQ25898X_RES_COMP_SHIFT      5
#define BQ25898X_RES_COMP_BASE       0
#define BQ25898X_RES_COMP_LSB        20

/* IR Compensation Voltage Clamp */
#define BQ25898X_VCLAMP_MASK         0x1C
#define BQ25898X_VCLAMP_SHIFT        2
#define BQ25898X_VCLAMP_COMP_BASE    0
#define BQ25898X_VCLAMP_COMP_LSB     32

/* Thermal Regulation Threshold */
#define BQ25898X_TREG_MASK           0x03
#define BQ25898X_TREG_SHIFT          0
#define BQ25898X_TREG_60C            0x00
#define BQ25898X_TREG_80C            0x01
#define BQ25898X_TREG_100C           0x02
#define BQ25898X_TREG_120C           0x03

/* Register 0x09 */
#define BQ25898X_REG_09              0x09
/* Force Start Input Current Optimizer(ICO) */
#define BQ25898X_FORCE_ICO_MASK      0x80
#define BQ25898X_FORCE_ICO_SHIFT     7
#define BQ25898X_FORCE_ICO           1
/* Safety Timer Setting during DPM */
#define BQ25898X_TMR2X_EN_MASK       0x40
#define BQ25898X_TMR2X_EN_SHIFT      6
#define BQ25898X_TMR2X_ENABLE        1
#define BQ25898X_TMR2X_DISABLE       0
/* Force BATFET off to enable ship mode with tSM_DLY delay time */
#define BQ25898X_BATFET_DIS_MASK     0x20
#define BQ25898X_BATFET_DIS_SHIFT    5
#define BQ25898X_BATFET_OFF          1
/* JEITA High Temperature Voltage Setting */
#define BQ25898X_JEITA_VSET_MASK     0x10
#define BQ25898X_JEITA_VSET_SHIFT    4
#define BQ25898X_JEITA_VSET_N200MV   0
#define BQ25898X_JEITA_VSET_VREG     1
/* BATFET turn off delay by tSM_DLY when BATFET_DIS bit is set */
#define BQ25898X_BATFET_DLY_MASK     0x08
#define BQ25898X_BATFET_DLY_SHIFT    3
#define BQ25898X_BATFET_DLY_OFF      0
#define BQ25898X_BATFET_DLY_ON       1
/* BATFET full system reset enable */
#define BQ25898X_BATFET_RST_EN_MASK  0x04
#define BQ25898X_BATFET_RST_EN_SHIFT  2
#define BQ25898X_BATFET_RST_DISABLE   0
#define BQ25898X_BATFET_RST_ENABLE    1

#define BQ25898X_PUMPX_UP_MASK       0x02
#define BQ25898X_PUMPX_UP_SHIFT      1
#define BQ25898X_PUMPX_UP            1

#define BQ25898X_PUMPX_DOWN_MASK     0x01
#define BQ25898X_PUMPX_DOWN_SHIFT    0
#define BQ25898X_PUMPX_DOWN          1

/* Register 0x0A */
#define BQ25898X_REG_0A              0x0A
#define BQ25898X_BOOSTV_MASK         0xF0
#define BQ25898X_BOOSTV_SHIFT        4
#define BQ25898X_BOOSTV_BASE         4550
#define BQ25898X_BOOSTV_LSB          64

#define BQ25898X_PFM_OTG_DIS_MASK     8
#define BQ25898X_PFM_OTG_DIS_SHIFT    3
#define BQ25898X_PFM_OTG_DIS_DISABLE  1
#define BQ25898X_PFM_OTG_DIS_ENABLE   0

#define BQ25898X_BOOST_LIM_MASK      0x07
#define BQ25898X_BOOST_LIM_SHIFT     0
#define BQ25898X_BOOST_LIM_500MA     0x00
#define BQ25898X_BOOST_LIM_800MA     0x01
#define BQ25898X_BOOST_LIM_1000MA    0x02
#define BQ25898X_BOOST_LIM_1200MA    0x03
#define BQ25898X_BOOST_LIM_1500MA    0x04
#define BQ25898X_BOOST_LIM_1800MA    0x05
#define BQ25898X_BOOST_LIM_2100MA    0x06
#define BQ25898X_BOOST_LIM_2400MA    0x07

/* Register 0x0B */
#define BQ25898X_REG_0B              0x0B
#define BQ25898X_VBUS_STAT_MASK      0xE0
#define BQ25898X_VBUS_STAT_SHIFT     5

#if (BQ25898X_ADDR == BQ25898D_ADDR)
    #define BQ25898X_VBUS_NO_INPUT       0x00
    #define BQ25898X_VBUS_SDP            0x01
    #define BQ25898X_VBUS_CDP            0x02
    #define BQ25898X_VBUS_DCP            0x03
    #define BQ25898X_VBUS_HDCP           0x04
    #define BQ25898X_VBUS_UNKOWN         0x05
    #define BQ25898X_VBUS_NON_STANDARD   0x06
    #define BQ25898X_VBUS_OTG            0x07
#else
    #define BQ25898X_VBUS_NO_INPUT       0x00
    #define BQ25898X_VBUS_SDP            0x01
    #define BQ25898X_VBUS_ADAPTER        0x02
    #define BQ25898X_VBUS_OTG            0x07
#endif

#define BQ25898X_CHG_STAT_MASK          0x18
#define BQ25898X_CHG_STAT_SHIFT         3
#define BQ25898X_CHG_STAT_IDLE          0x00
#define BQ25898X_CHG_STAT_PRECHG        0x01
#define BQ25898X_CHG_STAT_FASTCHG       0x02
#define BQ25898X_CHG_STAT_CHGDONE       0x03

#define BQ25898X_PG_STAT_MASK        0x04
#define BQ25898X_PG_STAT_SHIFT       2

#define BQ25898X_VSYS_STAT_MASK      0x01
#define BQ25898X_VSYS_STAT_SHIFT     0

/* Register 0x0C*/
#define BQ25898X_REG_0C              0x0c
#define BQ25898X_FAULT_WDT_MASK      0x80
#define BQ25898X_FAULT_WDT_SHIFT     7

#define BQ25898X_FAULT_BOOST_MASK    0x40
#define BQ25898X_FAULT_BOOST_SHIFT   6

#define BQ25898X_FAULT_WATCHDOG_MASK     0x80
#define BQ25898X_FAULT_WATCHDOG_SHIFT    7

#define BQ25898X_FAULT_OUTPUTVOLTAGE_MASK     0x08
#define BQ25898X_FAULT_OUTPUTVOLTAGE_SHIFT    3


#define BQ25898X_FAULT_CHRG_MASK     0x30
#define BQ25898X_FAULT_CHRG_SHIFT    4
#define BQ25898X_FAULT_CHRG_NORMAL   0
#define BQ25898X_FAULT_CHRG_INPUT    1
#define BQ25898X_FAULT_CHRG_THERMAL  2
#define BQ25898X_FAULT_CHRG_TIMER    3

#define BQ25898X_FAULT_BAT_MASK      0x08
#define BQ25898X_FAULT_BAT_SHIFT     3

#define BQ25898X_FAULT_NTC_MASK      0x07
#define BQ25898X_FAULT_NTC_SHIFT     0

#define BQ25898X_FAULT_NTC_TSCOLD    1
#define BQ25898X_FAULT_NTC_TSHOT     2

#define BQ25898X_FAULT_NTC_WARM      2
#define BQ25898X_FAULT_NTC_COOL      3
#define BQ25898X_FAULT_NTC_COLD      5
#define BQ25898X_FAULT_NTC_HOT       6

/* Register 0x0D */
#define BQ25898X_REG_0D               0x0D
#define BQ25898X_FORCE_VINDPM_MASK    0x80
#define BQ25898X_FORCE_VINDPM_SHIFT   7
#define BQ25898X_FORCE_VINDPM_ENABLE  1
#define BQ25898X_FORCE_VINDPM_DISABLE 0

#define BQ25898X_VINDPM_MASK         0x7F
#define BQ25898X_VINDPM_SHIFT        0
#define BQ25898X_VINDPM_BASE         2600
#define BQ25898X_VINDPM_LSB          100


/* Register 0x0E*/
#define BQ25898X_REG_0E              0x0E
#define BQ25898X_THERM_STAT_MASK     0x80
#define BQ25898X_THERM_STAT_SHIFT    7

#define BQ25898X_BATV_MASK           0x7F
#define BQ25898X_BATV_SHIFT          0
#define BQ25898X_BATV_BASE           2304
#define BQ25898X_BATV_LSB            20


/* Register 0x0F */
#define BQ25898X_REG_0F              0x0F
#define BQ25898X_SYSV_MASK           0x7F
#define BQ25898X_SYSV_SHIFT          0
#define BQ25898X_SYSV_BASE           2304
#define BQ25898X_SYSV_LSB            20


/* Register 0x10*/
#define BQ25898X_REG_10              0x10
#define BQ25898X_TSPCT_MASK          0x7F
#define BQ25898X_TSPCT_SHIFT         0
#define BQ25898X_TSPCT_BASE          21
/* should be 0.465,kernel does not support float */
#define BQ25898X_TSPCT_LSB           465

/* Register 0x11 */
#define BQ25898X_REG_11              0x11
#define BQ25898X_VBUS_GD_MASK        0x80
#define BQ25898X_VBUS_GD_SHIFT       7

#define BQ25898X_VBUSV_MASK          0x7F
#define BQ25898X_VBUSV_SHIFT         0
#define BQ25898X_VBUSV_BASE          2600
#define BQ25898X_VBUSV_LSB           100

/* Register 0x12*/
#define BQ25898X_REG_12              0x12
#define BQ25898X_ICHGR_MASK          0x7F
#define BQ25898X_ICHGR_SHIFT         0
#define BQ25898X_ICHGR_BASE          0
#define BQ25898X_ICHGR_LSB           50

/* Register 0x13*/
#define BQ25898X_REG_13              0x13
#define BQ25898X_VDPM_STAT_MASK      0x80
#define BQ25898X_VDPM_STAT_SHIFT     7

#define BQ25898X_IDPM_STAT_MASK      0x40
#define BQ25898X_IDPM_STAT_SHIFT     6

#define BQ25898X_IDPM_LIM_MASK       0x3F
#define BQ25898X_IDPM_LIM_SHIFT      0
#define BQ25898X_IDPM_LIM_BASE       100
#define BQ25898X_IDPM_LIM_LSB        50

/* Register 0x14 */
#define BQ25898X_REG_14              0x14
#define BQ25898X_RESET_MASK          0x80
#define BQ25898X_RESET_SHIFT         7
#define BQ25898X_RESET               1

#define BQ25898X_ICO_OPTIMIZED_MASK  0x40
#define BQ25898X_ICO_OPTIMIZED_SHIFT 6

#define BQ25898X_PN_MASK             0x38
#define BQ25898X_PN_SHIFT            3

#define BQ25898X_TS_PROFILE_MASK     0x04
#define BQ25898X_TS_PROFILE_SHIFT    2

#define BQ25898X_DEV_REV_MASK        0x03
#define BQ25898X_DEV_REV_SHIFT       0


#endif

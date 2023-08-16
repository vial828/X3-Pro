#ifndef STRATOS_DEFS_H
#define STRATOS_DEFS_H

//#define NICO
//#define NICO_HEAT
//#define TEST_ADC
//#define TEST_PERFORMANCE
//#define DISABLE_WDG
//#define SINGLE_COIL     /*SINGLE_COIL or DOUBLE_COIL */
#define ENABLE_FUEL_GAUGE       /*use fuel gauge or not*/
//#define ENABLE_STICK_SENSOR       /*use stick sensor or not*/
//#define AUTO_TEST
//#define ENABLE_MONITOR_TASK_LOG
#define DEBUG_OFF
#endif

#ifdef SINGLE_COIL
    #define HTR_EN_1
#else//default  Double Coil
    #define HTR_EN_1
    #define HTR_EN_2
#endif

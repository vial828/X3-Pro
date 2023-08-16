#ifndef DEV_APP_VER__
#define DEV_APP_VER__
#include "stratos_defs.h"

#ifdef SSCOM

    #ifdef SINGLE_COIL
        #define DEVICE_APP_VER  "X3_SINGLE_APP_S_POA_2023.03.20.1.u"
    #else //dual coil
        #define DEVICE_APP_VER  "X3_DUAL_APP_S_POA_2023.08.03.1.u"
    #endif
#else
    #ifdef SINGLE_COIL
        #define DEVICE_APP_VER  "X3_SINGLE_APP_S_POA_2023.03.20.1.f"
    #else //dual coil
        #define DEVICE_APP_VER  "X3_DUAL_APP_S_POA_2023.08.03.1.f"
    #endif
#endif

#define BL_VERSION    "X3_BL_RDP"

#define dev_app_version  202308031

#endif



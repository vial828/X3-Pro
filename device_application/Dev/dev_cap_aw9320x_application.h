#ifndef __AW9320X_DEMO_H_
#define __AW9320X_DEMO_H_

#include "HWI_Hal.h"
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "HWI_i2c.h"

#include "dev_cap_aw_type.h"


#ifdef __cplusplus
extern "C" {
#endif

#define AW9320X_I2C_DEV_ADDR    (0x12)
#define AW_SPP_RECV_MAX_LEN     (1024)

struct aw9320x_hw_info {
    uint8_t i2c_dev_addr;
};
void dev_aw93205_init(void);
void dev_aw9320x_data_log(void);
void dev_aw9320x_self_cap_parasitic_data_get(void);
void dev_aw9320x_process(void);
int8_t dev_aw93205_id_get(void);


#ifdef __cplusplus
}
#endif
#endif


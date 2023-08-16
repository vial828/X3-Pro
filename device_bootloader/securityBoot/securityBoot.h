#ifndef _SECURITY_BOOT_H_
#define _SECURITY_BOOT_H_

#include <stdint.h>
#include <ymodem.h>

#define HASH_BLOCK  (4)
#define SIGNATURE_LENGTH        (32)

extern uint8_t signature[];

void do_hash(char * hash_code);
int securityboot(void);
int securitybootAfterUpdate(void);
void delay_ms(uint32_t delay);

#endif

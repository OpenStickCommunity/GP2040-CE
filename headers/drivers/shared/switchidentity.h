#ifndef _SWITCH_IDENTITY_H_
#define _SWITCH_IDENTITY_H_

#include <stddef.h>
#include <stdint.h>

#define SWITCH_IDENTITY_DOMAIN_SWITCH2 0x00
#define SWITCH_IDENTITY_DOMAIN_SWITCH  0x40

void getSwitchIdentityBytes(uint8_t domain, uint8_t * out, size_t len);

#endif

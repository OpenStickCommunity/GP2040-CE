/*
 * Andrew Blackledge - Sep 5 2023
 * 
 * Interface for Bluetooth stack
 */

#ifndef BT_INTERFACE_H_
#define BT_INTERFACE_H_

#include <stdint.h>

void setupBTInterface();

#ifdef __cplusplus
extern "C" {
#endif
uint32_t custom_btstack_storage_offset();

#ifdef __cplusplus
}
#endif

#endif
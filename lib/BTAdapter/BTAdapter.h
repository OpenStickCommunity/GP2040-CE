/*
 * Andrew Blackledge - Sep 8 2023
 * 
 * Interface for Bluetooth stack
 */


#ifndef BT_ADAPTER_H_
#define BT_ADAPTER_H_

#include <stdint.h>

void updateBluetoothInputs(uint16_t leftX, uint16_t leftY, uint16_t rightX, uint16_t rightY, uint16_t buttons);

#endif
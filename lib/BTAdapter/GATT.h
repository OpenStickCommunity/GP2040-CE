
// C:\Users\Andrew\Documents\projects\GP2040-CE\lib\BTAdapter\GATT.h generated from C:\Users\Andrew\Documents\projects\GP2040-CE\lib\BTAdapter\BTAdapter.gatt for BTstack
// it needs to be regenerated when the .gatt file is updated. 

// To generate C:\Users\Andrew\Documents\projects\GP2040-CE\lib\BTAdapter\GATT.h:
// C:/Users/Andrew/Documents/projects/pico-sdk/lib/btstack/tool/compile_gatt.py C:\Users\Andrew\Documents\projects\GP2040-CE\lib\BTAdapter\BTAdapter.gatt C:\Users\Andrew\Documents\projects\GP2040-CE\lib\BTAdapter\GATT.h

// att db format version 1

// binary attribute representation:
// - size in bytes (16), flags(16), handle (16), uuid (16/128), value(...)

#include <stdint.h>

// Reference: https://en.cppreference.com/w/cpp/feature_test
#if __cplusplus >= 200704L
constexpr
#endif
const uint8_t profile_data[] =
{
    // ATT DB Version
    1,

    // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18, 
    // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME - READ
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a, 
    // 0x0003 VALUE CHARACTERISTIC-GAP_DEVICE_NAME - READ -'GP2040-CE Gamepad'
    // READ_ANYBODY
    0x19, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x2a, 0x47, 0x50, 0x32, 0x30, 0x34, 0x30, 0x2d, 0x43, 0x45, 0x20, 0x47, 0x61, 0x6d, 0x65, 0x70, 0x61, 0x64, 
    // 0x0004 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT - WRITE | INDICATE | READ | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x04, 0x00, 0x03, 0x28, 0x2a, 0x05, 0x00, 0xbc, 0x2a, 
    // 0x0005 VALUE CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT - WRITE | INDICATE | READ | DYNAMIC
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0x01, 0x05, 0x00, 0xbc, 0x2a, 
    // 0x0006 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x06, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // 0x0007 PRIMARY_SERVICE-GATT_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x28, 0x01, 0x18, 
    // 0x0008 CHARACTERISTIC-GATT_DATABASE_HASH - READ
    0x0d, 0x00, 0x02, 0x00, 0x08, 0x00, 0x03, 0x28, 0x02, 0x09, 0x00, 0x2a, 0x2b, 
    // 0x0009 VALUE CHARACTERISTIC-GATT_DATABASE_HASH - READ -''
    // READ_ANYBODY
    0x18, 0x00, 0x02, 0x00, 0x09, 0x00, 0x2a, 0x2b, 0x40, 0x4b, 0xc0, 0x4f, 0xea, 0x35, 0x23, 0x16, 0xac, 0x59, 0x5a, 0xbf, 0x01, 0x70, 0xc7, 0x1a, 
    // END
    0x00, 0x00, 
}; // total size 61 bytes 


//
// list service handle ranges
//
#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 0x0001
#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 0x0006
#define ATT_SERVICE_GAP_SERVICE_01_START_HANDLE 0x0001
#define ATT_SERVICE_GAP_SERVICE_01_END_HANDLE 0x0006
#define ATT_SERVICE_GATT_SERVICE_START_HANDLE 0x0007
#define ATT_SERVICE_GATT_SERVICE_END_HANDLE 0x0009
#define ATT_SERVICE_GATT_SERVICE_01_START_HANDLE 0x0007
#define ATT_SERVICE_GATT_SERVICE_01_END_HANDLE 0x0009

//
// list mapping between characteristics and handles
//
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT_01_VALUE_HANDLE 0x0005
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TDS_CONTROL_POINT_01_CLIENT_CONFIGURATION_HANDLE 0x0006
#define ATT_CHARACTERISTIC_GATT_DATABASE_HASH_01_VALUE_HANDLE 0x0009

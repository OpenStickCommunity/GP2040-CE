#ifndef GP2040_BLE_IDENTITY_H
#define GP2040_BLE_IDENTITY_H

// BLE Device Identity defaults
//
// These macros populate the BLE Device Information Service (0x180A) so that
// hosts like the Windows Gamepad Tester display a meaningful vendor / product
// identity instead of "Unknown Gamepad / Vendor: 0000 / Product: 0000".
//
// Per-board overrides:
//   A board may ship a minimal, dependency-free
//   configs/<Board>/BleIdentityConfig.h that #defines any of the macros
//   below. BLEHIDManager.cpp pulls it in via __has_include, mirroring the
//   BatteryConfig.h pattern. BoardConfig.h itself cannot be included from
//   BLEHIDManager.cpp because it transitively pulls in TinyUSB's hid.h,
//   which conflicts with BTstack's hid_report_type_t in that translation
//   unit.

#if defined(__has_include)
#  if __has_include("BleIdentityConfig.h")
#    include "BleIdentityConfig.h"
#  endif
#endif

// PnP ID (characteristic 0x2A50) ----------------------------------------------
// Vendor Source ID: 0x01 = Bluetooth SIG, 0x02 = USB Implementers Forum.
// USB-IF is the option most OS / driver databases key off.
#ifndef BLE_VENDOR_SOURCE_ID
#define BLE_VENDOR_SOURCE_ID  0x02
#endif

// Same VID/PID GP2040-CE uses for its USB HID (DirectInput) mode
// (see headers/drivers/hid/HIDDescriptors.h), so BLE presents the same
// gamepad identity known to SDL / Steam mapping databases.
// Deliberately NOT 0x045E (Microsoft) / 0x054C (Sony) / 0x057E (Nintendo) —
// spoofing those over BLE causes Windows to bind console class drivers that
// cannot talk to a BLE endpoint.
#ifndef BLE_VENDOR_ID
#define BLE_VENDOR_ID         0x10C4
#endif

#ifndef BLE_PRODUCT_ID
#define BLE_PRODUCT_ID        0x82C0
#endif

// Product Version is a 16-bit field, conventionally BCD JJ.M.N as 0xJJMN.
#ifndef BLE_PRODUCT_VERSION
#define BLE_PRODUCT_VERSION   0x0100
#endif

// DIS string characteristics -------------------------------------------------
#ifndef BLE_MANUFACTURER_NAME
#define BLE_MANUFACTURER_NAME "OpenStickCommunity"
#endif

#ifndef BLE_MODEL_NUMBER
#define BLE_MODEL_NUMBER      "GP2040-CE"
#endif

// Hardware Revision: board name. GP2040_BOARDCONFIG is a quoted compile
// definition set by the top-level CMakeLists.txt (e.g. "PimoroniPicoLipo2XLW").
#ifndef BLE_HARDWARE_REVISION
#define BLE_HARDWARE_REVISION GP2040_BOARDCONFIG
#endif

#endif // GP2040_BLE_IDENTITY_H

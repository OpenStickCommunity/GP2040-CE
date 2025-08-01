/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>

#define PS3_ENDPOINT_SIZE 64

// DS3 Sixaxis
#define PS3_VENDOR_ID         0x054C
#define PS3_PRODUCT_ID        0x0268

#define PS3_ALT_VENDOR_ID     0x10C4
#define PS3_ALT_PRODUCT_ID    0x82C0

#define PS3_WHEEL_VENDOR_ID   0x046D
#define PS3_WHEEL_PRODUCT_ID  0xC294

#define PS3_GUITAR_VENDOR_ID  0x12BA
#define PS3_GUITAR_PRODUCT_ID 0x0200

#define PS3_DRUM_VENDOR_ID    0x12BA
#define PS3_DRUM_PRODUCT_ID   0x0210

/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

#define ENDPOINT0_SIZE	64
#define PS3_FEATURES_SIZE 48

#define GAMEPAD_INTERFACE	0
#define GAMEPAD_ENDPOINT	1
#define GAMEPAD_SIZE		64

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

// HAT report (4 bits)
#define PS3_HAT_UP        0x00
#define PS3_HAT_UPRIGHT   0x01
#define PS3_HAT_RIGHT     0x02
#define PS3_HAT_DOWNRIGHT 0x03
#define PS3_HAT_DOWN      0x04
#define PS3_HAT_DOWNLEFT  0x05
#define PS3_HAT_LEFT      0x06
#define PS3_HAT_UPLEFT    0x07
#define PS3_HAT_NOTHING   0x08

// Button report (16 bits)
#define PS3_MASK_SQUARE   (1U <<  0)
#define PS3_MASK_CROSS    (1U <<  1)
#define PS3_MASK_CIRCLE   (1U <<  2)
#define PS3_MASK_TRIANGLE (1U <<  3)
#define PS3_MASK_L1       (1U <<  4)
#define PS3_MASK_R1       (1U <<  5)
#define PS3_MASK_L2       (1U <<  6)
#define PS3_MASK_R2       (1U <<  7)
#define PS3_MASK_SELECT   (1U <<  8)
#define PS3_MASK_START    (1U <<  9)
#define PS3_MASK_L3       (1U << 10)
#define PS3_MASK_R3       (1U << 11)
#define PS3_MASK_PS       (1U << 12)
#define PS3_MASK_TP       (1U << 13)

// HID analog sticks only report 8 bits
#define PS3_JOYSTICK_MIN 0x00
#define PS3_JOYSTICK_MID 0x7F
#define PS3_JOYSTICK_MAX 0xFF

#define PS3_WHEEL_MIN 0x00
#define PS3_WHEEL_MID 0x7F
#define PS3_WHEEL_MAX 0xFF

#define PS3_CENTER_SIXAXIS 0xFF01

typedef enum {
    PS3_OTHER = 0x00,
    PS3_GUITAR = 0x06,
    PS3_DRUMS,
    PS3_GAMEPAD = 0x07,
} PS3ControllerType;

typedef enum {
    PS3_FEATURE_01 = 0x01,
    PS3_FEATURE_EF = 0xEF,
    PS3_GET_PAIRING_INFO = 0xF2,
    PS3_FEATURE_F4 = 0xF4,
    PS3_FEATURE_F5 = 0xF5,
    PS3_FEATURE_F7 = 0xF7,
    PS3_FEATURE_F8 = 0xF8,
} PS3ReportTypes;

typedef enum {
    PS3_PLUGGED = 0x02,
    PS3_UNPLUGGED = 0x03
} PS3PluggedInState;

typedef enum {
    PS3_POWER_CHARGING = 0xEE,
    PS3_POWER_NOT_CHARGING = 0xF1,
    PS3_POWER_SHUTDOWN = 0x01,
    PS3_POWER_DISCHARGING = 0x02,
    PS3_POWER_LOW = 0x03,
    PS3_POWER_HIGH = 0x04,
    PS3_POWER_FULL = 0x05,
} PS3PowerState;

typedef enum {
    PS3_WIRED_RUMBLE = 0x10,
    PS3_WIRED_NO_RUMBLE = 0x12,
    PS3_WIRELESS_RUMBLE = 0x14,
    PS3_WIRELESS_NO_RUMBLE = 0x16,
} PS3WiredState;

typedef struct __attribute((packed, aligned(1)))
{
	// digital buttons, 0 = off, 1 = on
    // 0
    uint8_t reportID;
    // 1
    uint8_t reserved;

    // 2
    uint8_t buttonSelect : 1;
    uint8_t buttonL3 : 1;
    uint8_t buttonR3 : 1;
    uint8_t buttonStart : 1;

    uint8_t dpadUp : 1;
    uint8_t dpadRight : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadLeft : 1;

    // 3
    uint8_t buttonL2 : 1;
    uint8_t buttonR2 : 1;
    uint8_t buttonL1 : 1;
    uint8_t buttonR1 : 1;

    uint8_t buttonNorth : 1;
    uint8_t buttonEast : 1;
    uint8_t buttonSouth : 1;
    uint8_t buttonWest : 1;

    // 4
    uint8_t buttonPS : 1;
    uint8_t buttonTP : 1;
    uint8_t : 6;

    // 5
    uint8_t : 8;
    
    // left and right analog sticks, 0x00 left/up, 0x80 middle, 0xff right/down
    // 6
    uint8_t leftStickX;
    // 7
    uint8_t leftStickY;
    // 8
    uint8_t rightStickX;
    // 9
    uint8_t rightStickY;
    // 10
    uint8_t : 8;

    // 11
    uint8_t : 8;

    // 12
    uint8_t movePowerStatus : 8;

    // 13
    uint8_t : 8;

    // button analog values for the d-pad.
    // 14
    uint8_t dpadUpAnalog;
    // 15
    uint8_t dpadRightAnalog;
    // 16
    uint8_t dpadDownAnalog;
    // 17
    uint8_t dpadLeftAnalog;

    // button axis, 0x00 = unpressed, 0xff = fully pressed
    // 18
    uint8_t buttonL2Analog;
    // 19
    uint8_t buttonR2Analog;
    // 20
    uint8_t buttonL1Analog;
    // 21
    uint8_t buttonR1Analog;

    // 22
    uint8_t buttonNorthAnalog;
    // 23
    uint8_t buttonEastAnalog;
    // 24
    uint8_t buttonSouthAnalog;
    // 25
    uint8_t buttonWestAnalog;

    // 26
    uint8_t reserved2[3];

    // 29
    uint8_t plugged;
    // 30
    uint8_t powerStatus;

    // 31
    uint8_t rumbleStatus;

    // 32
    uint8_t reserved3[9];

    // 40
    uint16_t accelerometerX;
    // 42
    uint16_t accelerometerY;
    // 44
    uint16_t accelerometerZ;
    // 46
    uint16_t gyroscopeZ;

    // 48
    uint16_t reserved4;
} PS3Report; // 49 length

typedef struct __attribute((packed, aligned(1)))
{
    union {
        struct  __attribute((packed, aligned(1))) {
            uint8_t buttonNorth : 1;
            uint8_t buttonEast : 1;
            uint8_t buttonSouth : 1;
            uint8_t buttonWest : 1;

            uint8_t buttonL2 : 1;
            uint8_t buttonR2 : 1;
            uint8_t buttonL1 : 1;
            uint8_t buttonR1 : 1;

            uint8_t buttonStart : 1;
            uint8_t buttonSelect : 1;
            uint8_t buttonL3 : 1;
            uint8_t buttonR3 : 1;

            uint8_t buttonPS : 1;
            uint8_t buttonTP : 1;
            uint8_t : 2;

            uint8_t dpadDirection;

            uint8_t leftStickX;
            uint8_t leftStickY;
            uint8_t rightStickX;
            uint8_t rightStickY;

            uint8_t dpadRightAnalog;
            uint8_t dpadLeftAnalog;
            uint8_t dpadUpAnalog;
            uint8_t dpadDownAnalog;

            uint8_t buttonNorthAnalog;
            uint8_t buttonEastAnalog;
            uint8_t buttonSouthAnalog;
            uint8_t buttonWestAnalog;

            uint8_t buttonL2Analog;
            uint8_t buttonR2Analog;
            uint8_t buttonL1Analog;
            uint8_t buttonR1Analog;

            uint16_t accelerometerX;
            uint16_t accelerometerY;
            uint16_t accelerometerZ;
            uint16_t gyroscopeZ;
        } gamepad;

        struct  __attribute((packed, aligned(1))) {
            uint8_t blue : 1;
            uint8_t green : 1;
            uint8_t red : 1;
            uint8_t yellow : 1;

            uint8_t orange : 1;
            uint8_t tilt : 1;
            uint8_t solo : 1;
            uint8_t : 1;

            uint8_t buttonSelect : 1;
            uint8_t buttonStart : 1;
            uint8_t : 2;

            uint8_t buttonPS : 1;
            uint8_t : 3;

            uint8_t dpadDirection;

            uint8_t padding0[2];
            uint8_t whammy;
            uint8_t pickup;

            uint8_t padding1[12];
            int16_t padding2[4];
        } guitar;

        struct  __attribute((packed, aligned(1))) {
            uint8_t blue : 1;
            uint8_t green : 1;
            uint8_t red : 1;
            uint8_t yellow : 1;

            uint8_t kickPedalLeft : 1;
            uint8_t kickPedalRight : 1;
            uint8_t pad : 1;
            uint8_t cymbal : 1;

            uint8_t buttonSelect : 1;
            uint8_t buttonStart : 1;
            uint8_t : 2;

            uint8_t buttonPS : 1;
            uint8_t : 3;

            uint8_t dpadDirection;

            uint8_t padding0[2];
            uint8_t midi0;
            uint8_t padding1[5];

            uint8_t velocityYellow;
            uint8_t velocityRed;
            uint8_t velocityGreen;
            uint8_t velocityBlue;
            uint8_t velocityPedal;
            uint8_t velocityOrange;

            uint8_t padding2[2];
            uint8_t midi1;
            uint8_t unused0;
            uint16_t unused1;
            uint8_t midi2;
            uint8_t unused2;
            uint16_t unused3;
        } drums;

        struct  __attribute((packed, aligned(1))) {
            uint8_t buttonNorth : 1;
            uint8_t buttonEast : 1;
            uint8_t buttonSouth : 1;
            uint8_t buttonWest : 1;

            uint8_t buttonL2 : 1;
            uint8_t buttonR2 : 1;
            uint8_t buttonL1 : 1;
            uint8_t buttonR1 : 1;

            uint8_t buttonStart : 1;
            uint8_t buttonSelect : 1;
            uint8_t buttonL3 : 1;
            uint8_t buttonR3 : 1;

            uint8_t buttonPS : 1;
            uint8_t buttonTP : 1;
            uint8_t : 2;

            uint8_t dpadDirection : 4;

            uint8_t steeringWheel;
            uint8_t brakePedal;
            uint8_t gasPedal;
            uint8_t clutchPedal;

            uint8_t padding[20];
        } wheel;
    };
} PS3ReportAlt;

typedef struct __attribute((packed, aligned(1)))
{
    // 0
    uint8_t reserved;

    // 1
    uint8_t rightMotorDuration;
    // 2
    uint8_t rightMotorPower;

    // 3
    uint8_t leftMotorDuration;
    // 4
    uint8_t leftMotorPower;

    // 5
    uint8_t reserved2[4];

    // 9
    uint8_t playerLED : 4;
    uint8_t : 4;

    // 10
    uint8_t reserved3[38];
} PS3Features; // 48 length

typedef struct __attribute((packed, aligned(1)))
{
    uint8_t reserved[2];
    uint8_t deviceAddress[7]; // leading zero followed by address
    uint8_t hostAddress[7]; // leading zero followed by address
    uint8_t reserved1;
} PS3BTInfo;

static const uint8_t ps3_string_language[]     = { 0x09, 0x04 };
static const uint8_t ps3_string_manufacturer[] = "Open Stick Community";
static const uint8_t ps3_string_product[]      = "GP2040-CE (PS3)";
static const uint8_t ps3_string_version[]      = "1.0";

static const uint8_t *ps3_string_descriptors[] __attribute__((unused)) =
{
	ps3_string_language,
	ps3_string_manufacturer,
	ps3_string_product,
	ps3_string_version
};

static const uint8_t ps3_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass 
    0x00,        // bDeviceProtocol 
    0x40,        // bMaxPacketSize0 64
    LSB(PS3_VENDOR_ID), MSB(PS3_VENDOR_ID),	  // idVendor
    LSB(PS3_PRODUCT_ID), MSB(PS3_PRODUCT_ID), // idProduct
    0x00, 0x01,  // bcdDevice 1.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

static const uint8_t ps3_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Physical)
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0x01,        //     Report ID (1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
                       //     NOTE: reserved byte
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x13,        //     Report Count (19)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x35, 0x00,        //     Physical Minimum (0)
    0x45, 0x01,        //     Physical Maximum (1)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x13,        //     Usage Maximum (0x13)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x0D,        //     Report Count (13)
    0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
                       //     NOTE: 32 bit integer, where 0:18 are buttons and 19:31 are reserved
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x01,        //     Usage (Pointer)
    0xA1, 0x00,        //     Collection (Undefined)
    0x75, 0x08,        //       Report Size (8)
    0x95, 0x04,        //       Report Count (4)
    0x35, 0x00,        //       Physical Minimum (0)
    0x46, 0xFF, 0x00,  //       Physical Maximum (255)
    0x09, 0x30,        //       Usage (X)
    0x09, 0x31,        //       Usage (Y)
    0x09, 0x32,        //       Usage (Z)
    0x09, 0x35,        //       Usage (Rz)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
                       //       NOTE: four joysticks
    0xC0,              //     End Collection
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x27,        //     Report Count (39)
    0x09, 0x01,        //     Usage (Pointer)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0x02,        //     Report ID (2)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0xEE,        //     Report ID (238)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0xEF,        //     Report ID (239)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

static const uint8_t ps3_alt_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0xA1, 0x02,        //   Collection (Logical)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x35, 0x00,        //     Physical Minimum (0)
    0x45, 0x01,        //     Physical Maximum (1)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x0D,        //     Report Count (13)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x0D,        //     Usage Maximum (0x0D)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x25, 0x07,        //     Logical Maximum (7)
    0x46, 0x3B, 0x01,  //     Physical Maximum (315)
    0x75, 0x04,        //     Report Size (4)
    0x95, 0x01,        //     Report Count (1)
    0x65, 0x14,        //     Unit (System: English Rotation, Length: Centimeter)
    0x09, 0x39,        //     Usage (Hat switch)
    0x81, 0x42,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //     Unit (None)
    0x95, 0x01,        //     Report Count (1)
    0x81, 0x01,        //     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x46, 0xFF, 0x00,  //     Physical Maximum (255)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x32,        //     Usage (Z)
    0x09, 0x35,        //     Usage (Rz)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x04,        //     Report Count (4)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //     Usage (0x20)
    0x09, 0x21,        //     Usage (0x21)
    0x09, 0x22,        //     Usage (0x22)
    0x09, 0x23,        //     Usage (0x23)
    0x09, 0x24,        //     Usage (0x24)
    0x09, 0x25,        //     Usage (0x25)
    0x09, 0x26,        //     Usage (0x26)
    0x09, 0x27,        //     Usage (0x27)
    0x09, 0x28,        //     Usage (0x28)
    0x09, 0x29,        //     Usage (0x29)
    0x09, 0x2A,        //     Usage (0x2A)
    0x09, 0x2B,        //     Usage (0x2B)
    0x95, 0x0C,        //     Report Count (12)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x0A, 0x21, 0x26,  //     Usage (0x2621)
    0x95, 0x08,        //     Report Count (8)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x0A, 0x21, 0x26,  //     Usage (0x2621)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x26, 0xFF, 0x03,  //     Logical Maximum (1023)
    0x46, 0xFF, 0x03,  //     Physical Maximum (1023)
    0x09, 0x2C,        //     Usage (0x2C)
    0x09, 0x2D,        //     Usage (0x2D)
    0x09, 0x2E,        //     Usage (0x2E)
    0x09, 0x2F,        //     Usage (0x2F)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x04,        //     Report Count (4)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Logical)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x46, 0xFF, 0x00,  //     Physical Maximum (255)
    0x95, 0x07,        //     Report Count (7)
    0x75, 0x08,        //     Report Size (8)
    0x09, 0x03,        //     Usage (0x03)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

static const uint8_t ps3_hid_descriptor[] =
{
    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.17
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x94, 0x00,  // wDescriptorLength[0] 148
};

static const uint8_t ps3_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x29, 0x00,  // wTotalLength 41
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0x80,        // bmAttributes
    0xFA,        // bMaxPower 500mA

    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0x03,        // bInterfaceClass
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.17
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x94, 0x00,  // wDescriptorLength[0] 148

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x02,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)
};

static const uint8_t ps3_alt_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x29, 0x00,  // wTotalLength 41
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0x80,        // bmAttributes
    0xFA,        // bMaxPower 500mA

    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0x03,        // bInterfaceClass
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.17
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x9D, 0x00,  // wDescriptorLength[0] 157

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x02,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)
};

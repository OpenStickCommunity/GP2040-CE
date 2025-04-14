/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>

#define SWITCH_PRO_ENDPOINT_SIZE 64

// HAT report (4 bits)
#define SWITCH_PRO_HAT_UP        0x00
#define SWITCH_PRO_HAT_UPRIGHT   0x01
#define SWITCH_PRO_HAT_RIGHT     0x02
#define SWITCH_PRO_HAT_DOWNRIGHT 0x03
#define SWITCH_PRO_HAT_DOWN      0x04
#define SWITCH_PRO_HAT_DOWNLEFT  0x05
#define SWITCH_PRO_HAT_LEFT      0x06
#define SWITCH_PRO_HAT_UPLEFT    0x07
#define SWITCH_PRO_HAT_NOTHING   0x08

// Button report (16 bits)
//#define SWITCH_PRO_MASK_Y       (1U <<  0)
//#define SWITCH_PRO_MASK_B       (1U <<  1)
//#define SWITCH_PRO_MASK_A       (1U <<  2)
//#define SWITCH_PRO_MASK_X       (1U <<  3)
//#define SWITCH_PRO_MASK_L       (1U <<  4)
//#define SWITCH_PRO_MASK_R       (1U <<  5)
//#define SWITCH_PRO_MASK_ZL      (1U <<  6)
//#define SWITCH_PRO_MASK_ZR      (1U <<  7)
//#define SWITCH_PRO_MASK_MINUS   (1U <<  8)
//#define SWITCH_PRO_MASK_PLUS    (1U <<  9)
//#define SWITCH_PRO_MASK_L3      (1U << 10)
//#define SWITCH_PRO_MASK_R3      (1U << 11)
//#define SWITCH_PRO_MASK_HOME    (1U << 12)
//#define SWITCH_PRO_MASK_CAPTURE (1U << 13)

#define SWITCH_PRO_MASK_ZR (1U << 7)
#define SWITCH_PRO_MASK_R (1U << 6)
#define SWITCH_PRO_MASK_A (1U << 3)
#define SWITCH_PRO_MASK_B (1U << 2)
#define SWITCH_PRO_MASK_X (1U << 1)
#define SWITCH_PRO_MASK_Y 1U

#define SWITCH_PRO_MASK_CAPTURE (1U << 5)
#define SWITCH_PRO_MASK_HOME (1U << 4)
#define SWITCH_PRO_MASK_L3 (1U << 3)
#define SWITCH_PRO_MASK_R3 (1U << 2)
#define SWITCH_PRO_MASK_PLUS (1U << 1)
#define SWITCH_PRO_MASK_MINUS 1U

#define SWITCH_PRO_MASK_ZL (1U << 7)
#define SWITCH_PRO_MASK_L (1U << 6)

// Switch analog sticks only report 8 bits
#define SWITCH_PRO_JOYSTICK_MIN 0x00
#define SWITCH_PRO_JOYSTICK_MID 0x80
#define SWITCH_PRO_JOYSTICK_MAX 0xFF

typedef enum {
    REPORT_OUTPUT_01 = 0x01,
    REPORT_OUTPUT_10 = 0x10,
    REPORT_OUTPUT_21 = 0x21,
    REPORT_OUTPUT_30 = 0x30,
    REPORT_USB_OUTPUT_80 = 0x80,
    REPORT_USB_INPUT_81 = 0x81,
} SwitchReportID;

typedef enum {
    IDENTIFY = 0x01,
    HANDSHAKE,
    BAUD_RATE,
    DISABLE_USB_TIMEOUT,
    ENABLE_USB_TIMEOUT
} SwitchOutputSubtypes;

typedef enum {
    GET_CONTROLLER_STATE = 0x00,
    BLUETOOTH_PAIR_REQUEST = 0x01,
    REQUEST_DEVICE_INFO = 0x02,
    SET_MODE = 0x03,
    TRIGGER_BUTTONS = 0x04,
    SET_SHIPMENT = 0x08,
    SPI_READ = 0x10,
    SET_NFC_IR_CONFIG = 0x21,
    SET_NFC_IR_STATE = 0x22,
    SET_PLAYER_LIGHTS = 0x30,
    GET_PLAYER_LIGHTS = 0x31,
    COMMAND_UNKNOWN_33 = 0x33,
    SET_HOME_LIGHT = 0x38,
    TOGGLE_IMU = 0x40,
    IMU_SENSITIVITY = 0x41,
    READ_IMU = 0x43,
    ENABLE_VIBRATION = 0x48,
    GET_VOLTAGE = 0x50,
} SwitchCommands;

typedef struct __attribute((packed, aligned(1)))
{
    uint8_t connection_info : 4;
    uint8_t battery_level : 4;

    // byte 00
    uint8_t button_y : 1;
    uint8_t button_x : 1;
    uint8_t button_b : 1;
    uint8_t button_a : 1;
    uint8_t button_right_sl : 1;
    uint8_t button_right_sr : 1;
    uint8_t button_r : 1;
    uint8_t button_zr : 1;

    // byte 01
    uint8_t button_minus : 1;
    uint8_t button_plus : 1;
    uint8_t button_thumb_r : 1;
    uint8_t button_thumb_l : 1;
    uint8_t button_home : 1;
    uint8_t button_capture : 1;
    uint8_t dummy : 1;
    uint8_t charging_grip : 1;

    // byte 02
    uint8_t dpad_down : 1;
    uint8_t dpad_up : 1;
    uint8_t dpad_right : 1;
    uint8_t dpad_left : 1;
    uint8_t button_left_sl : 1;
    uint8_t button_left_sr : 1;
    uint8_t button_l : 1;
    uint8_t button_zl : 1;

    uint8_t analog[6];
} SwitchInputReport;

typedef struct __attribute((packed, aligned(1)))
{
    uint8_t reportID;
    uint8_t timestamp;
    SwitchInputReport inputs;
    uint8_t vibrator_input_report;
    uint8_t imu_data[36];
    uint8_t padding[15];
} SwitchProReport;

typedef enum {
    SWITCH_TYPE_LEFT_JOYCON = 0x01,
    SWITCH_TYPE_RIGHT_JOYCON,
    SWITCH_TYPE_PRO_CONTROLLER,
    SWITCH_TYPE_FAMICOM_LEFT_JOYCON = 0x07,
    SWITCH_TYPE_FAMICOM_RIGHT_JOYCON = 0x08,
    SWITCH_TYPE_NES_LEFT_JOYCON = 0x09,
    SWITCH_TYPE_NES_RIGHT_JOYCON = 0x0A,
    SWITCH_TYPE_SNES_RIGHT_JOYCON = 0x0B,
} SwitchControllerType;

typedef struct {
    uint8_t majorVersion;
    uint8_t minorVersion;
    uint8_t controllerType;
    uint8_t unknown00;
    uint8_t macAddress[6];
    uint8_t unknown01;
    uint8_t storedColors;
} SwitchDeviceInfo;

typedef struct
{
	uint16_t buttons;
	uint8_t hat;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
} SwitchProOutReport;

static const uint8_t switch_pro_string_language[]     = { 0x09, 0x04 };
static const uint8_t switch_pro_string_manufacturer[] = "Nintendo Co., Ltd.";
static const uint8_t switch_pro_string_product[]      = "Pro Controller";
static const uint8_t switch_pro_string_version[]      = "000000000001";

static const uint8_t *switch_pro_string_descriptors[] __attribute__((unused)) =
{
	switch_pro_string_language,
	switch_pro_string_manufacturer,
	switch_pro_string_product,
	switch_pro_string_version
};

static const uint8_t switch_pro_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass 
    0x00,        // bDeviceProtocol 
    0x40,        // bMaxPacketSize0 64
    0x7E, 0x05,  // idVendor 0x057E
    0x09, 0x20,  // idProduct 0x2009
    0x10, 0x02,  // bcdDevice 4.10
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x03,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

static const uint8_t switch_pro_hid_descriptor[] =
{
    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0xCB, 0x00,  // wDescriptorLength[0] 86
};

static const uint8_t switch_pro_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x29, 0x00,  // wTotalLength 41
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0xA0,        // bmAttributes Remote Wakeup
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
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0xCB, 0x00,  // wDescriptorLength[0] 203

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x08,        // bInterval 8 (unit depends on device speed)

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x01,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x08,        // bInterval 8 (unit depends on device speed)
};

static const uint8_t switch_pro_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x15, 0x00,        // Logical Minimum (0)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)

    0x85, 0x30,        //   Report ID (48)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0A,        //   Usage Maximum (0x0A)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0A,        //   Report Count (10)
    0x55, 0x00,        //   Unit Exponent (0)
    0x65, 0x00,        //   Unit (None)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x0B,        //   Usage Minimum (0x0B)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x0B, 0x01, 0x00, 0x01, 0x00,  //   Usage (0x010001)
    0xA1, 0x00,        //   Collection (Physical)
    0x0B, 0x30, 0x00, 0x01, 0x00,  //     Usage (0x010030)
    0x0B, 0x31, 0x00, 0x01, 0x00,  //     Usage (0x010031)
    0x0B, 0x32, 0x00, 0x01, 0x00,  //     Usage (0x010032)
    0x0B, 0x35, 0x00, 0x01, 0x00,  //     Usage (0x010035)
    0x15, 0x00,        //     Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
    0x75, 0x10,        //     Report Size (16)
    0x95, 0x04,        //     Report Count (4)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0x0B, 0x39, 0x00, 0x01, 0x00,  //   Usage (0x010039)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x0F,        //   Usage Minimum (0x0F)
    0x29, 0x12,        //   Usage Maximum (0x12)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x34,        //   Report Count (52)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)

    0x85, 0x21,        //   Report ID (33)
    0x09, 0x01,        //   Usage (0x01)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x81,        //   Report ID (-127)
    0x09, 0x02,        //   Usage (0x02)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

    0x85, 0x01,        //   Report ID (1)
    0x09, 0x03,        //   Usage (0x03)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x10,        //   Report ID (16)
    0x09, 0x04,        //   Usage (0x04)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x80,        //   Report ID (-128)
    0x09, 0x05,        //   Usage (0x05)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0x85, 0x82,        //   Report ID (-126)
    0x09, 0x06,        //   Usage (0x06)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x3F,        //   Report Count (63)
    0x91, 0x83,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)

    0xC0,              // End Collection
};

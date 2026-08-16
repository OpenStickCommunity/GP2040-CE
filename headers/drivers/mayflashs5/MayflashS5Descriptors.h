/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#pragma once

#include <stdint.h>
#include "drivers/ps4/PS4Descriptors.h"

#define PS5_ENDPOINT_SIZE 64

// PS5
#define PS5_VENDOR_ID     0x28B1
#define PS5_PRODUCT_ID    0x0101

// HAT report (4 bits)
#define PS5_HAT_UP        0x00
#define PS5_HAT_UPRIGHT   0x01
#define PS5_HAT_RIGHT     0x02
#define PS5_HAT_DOWNRIGHT 0x03
#define PS5_HAT_DOWN      0x04
#define PS5_HAT_DOWNLEFT  0x05
#define PS5_HAT_LEFT      0x06
#define PS5_HAT_UPLEFT    0x07
#define PS5_HAT_NOTHING   0x0F

// Button report (16 bits)
#define PS5_MASK_SQUARE   (1U <<  0)
#define PS5_MASK_CROSS    (1U <<  1)
#define PS5_MASK_CIRCLE   (1U <<  2)
#define PS5_MASK_TRIANGLE (1U <<  3)
#define PS5_MASK_L1       (1U <<  4)
#define PS5_MASK_R1       (1U <<  5)
#define PS5_MASK_L2       (1U <<  6)
#define PS5_MASK_R2       (1U <<  7)
#define PS5_MASK_SELECT   (1U <<  8)
#define PS5_MASK_START    (1U <<  9)
#define PS5_MASK_L3       (1U << 10)
#define PS5_MASK_R3       (1U << 11)
#define PS5_MASK_PS       (1U << 12)
#define PS5_MASK_TP       (1U << 13)

// PS5 analog sticks only report 8 bits
#define PS5_JOYSTICK_MIN 0x00
#define PS5_JOYSTICK_MID 0x80
#define PS5_JOYSTICK_MAX 0xFF

// touchpad resolution = 1920x943
#define PS5_TP_X_MIN 0
#define PS5_TP_X_MAX 1920
#define PS5_TP_Y_MIN 0
#define PS5_TP_Y_MAX 943

#define PS5_TP_MAX_COUNT 128

#define PS5_ACCEL_RES 8192
#define PS5_ACCEL_RANGE (PS5_ACCEL_RES * 4)
#define PS5_GYRO_RES 1024
#define PS5_GYRO_RANGE (PS5_GYRO_RES * 2048)

enum PowerState : uint8_t {
    Discharging         = 0x00, // Use PowerPercent
    Charging            = 0x01, // Use PowerPercent
    Complete            = 0x02, // PowerPercent not valid? assume 100%?
    AbnormalVoltage     = 0x0A, // PowerPercent not valid?
    AbnormalTemperature = 0x0B, // PowerPercent not valid?
    ChargingError       = 0x0F  // PowerPercent not valid?
};

typedef struct __attribute__((packed)) {
  uint8_t report_id;
  uint8_t left_stick_x;
  uint8_t left_stick_y;
  uint8_t right_stick_x;
  uint8_t right_stick_y;

  uint8_t left_trigger;
  uint8_t right_trigger;

  uint8_t SeqNo;

  uint8_t dpad : 4;
  uint8_t button_west : 1;
  uint8_t button_south : 1;
  uint8_t button_east : 1;
  uint8_t button_north : 1;

  uint8_t button_l1 : 1;
  uint8_t button_r1 : 1;
  uint8_t button_l2 : 1;
  uint8_t button_r2 : 1;
  uint8_t button_select : 1;
  uint8_t button_start : 1;
  uint8_t button_l3 : 1;
  uint8_t button_r3 : 1;

  uint8_t button_home : 1;
  uint8_t button_touchpad : 1;

  uint8_t button_mute : 1;
  uint8_t unk1 : 1;
  
  uint8_t button_left_function : 1;  // DualSense Edge
  uint8_t button_right_function : 1; // DualSense Edge
  uint8_t button_left_paddle : 1;    // DualSense Edge
  uint8_t button_right_paddle : 1;   // DualSense Edge
  
  uint8_t unk2;

  uint32_t auth_seq_number;

  PSSensor gyroscope;
  PSSensor accelerometer;

  uint32_t sensor_timestamp;
  int8_t temperature;

  TouchpadData touchpad_data;

  uint8_t trigger_right_stop_location: 4; // trigger stop can be a range from 0 to 9 (F/9.0 for Apple interface)
  uint8_t trigger_right_status: 4;
  uint8_t trigger_left_stop_location: 4;
  uint8_t trigger_left_status: 4;

  uint32_t host_timestamp;
  uint8_t trigger_right_effect: 4;
  uint8_t trigger_left_effect: 4;

  uint32_t device_timestamp;

  uint8_t power_percent : 4;    // 0x00-0x0A
  PowerState power_state : 4;

  uint8_t plugged_headphones : 1;
  uint8_t pluggedMic : 1;
  uint8_t micMuted: 1; // Mic muted by powersave/mute command
  uint8_t pluggedUsbData : 1;
  uint8_t pluggedUsbPower : 1; // appears that this cannot be 1 if PluggedUsbData is 1
  uint8_t usbPowerOnBT : 1; // appears this is only 1 if BT connected and USB powered
  uint8_t dockDetect : 1;
  uint8_t pluggedUnk : 1;
  uint8_t pluggedExternalMic : 1; // Is external mic active (automatic in mic auto mode)
  uint8_t hapticLowPassFilter : 1; // Is the Haptic Low-Pass-Filter active?
  uint8_t pluggedUnk3 : 6;

  uint8_t AesCmac[8];
} PS5Report;

// Mayflash S5 String Descriptors
static const uint8_t mayflashs5_string_language[]        = { 0x09, 0x04 };
static const uint8_t mayflashs5_string_manufacturer[]    = "Sony Interactive Entertainment";
static const uint8_t mayflashs5_string_product[]         = "DualSense Wireless Controller";
static const uint8_t mayflashs5_string_version[]         = "0.1";

static const uint8_t * mayflashs5_string_descriptors[] __attribute__((unused)) =
{
	mayflashs5_string_language,
	mayflashs5_string_manufacturer,
	mayflashs5_string_product,
	mayflashs5_string_version
};

static const uint8_t mayflashs5_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass 
    0x00,        // bDeviceProtocol 
    0x40,        // bMaxPacketSize0 64
    0x4C, 0x05,  // idVendor 0x054C
    0xE6, 0x0C,  // idProduct 0x0CE6
    0x00, 0x01,  // bcdDevice 1.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};


// Hybrid custom report descriptor
static const uint8_t mayflashs5_custom_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //   Usage (0x20)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0F,        //   Usage Maximum (0x0F)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0F,        //   Report Count (15)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x21,        //   Usage (0x21)
    0x95, 0x0D,        //   Report Count (11)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x22,        //   Usage (0x22)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x34,        //   Report Count (52)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x02,        //   Report ID (2)
    0x09, 0x23,        //   Usage (0x23)
    0x95, 0x2F,        //   Report Count (47)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x05,        //   Report ID (5)
    0x09, 0x33,        //   Usage (0x33)
    0x95, 0x28,        //   Report Count (40)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x08,        //   Report ID (8)
    0x09, 0x34,        //   Usage (0x34)
    0x95, 0x2F,        //   Report Count (47)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x09,        //   Report ID (9)
    0x09, 0x24,        //   Usage (0x24)
    0x95, 0x13,        //   Report Count (19)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x0A,        //   Report ID (10)
    0x09, 0x25,        //   Usage (0x25)
    0x95, 0x1A,        //   Report Count (26)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x20,        //   Report ID (32)
    0x09, 0x26,        //   Usage (0x26)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x21,        //   Report ID (33)
    0x09, 0x27,        //   Usage (0x27)
    0x95, 0x04,        //   Report Count (4)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x22,        //   Report ID (34)
    0x09, 0x40,        //   Usage (0x40)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
/*    0x85, 0x80,        //   Report ID (-128)
    0x09, 0x28,        //   Usage (0x28)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x81,        //   Report ID (-127)
    0x09, 0x29,        //   Usage (0x29)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile) */
    0x85, 0x82,        //   Report ID (-126)
    0x09, 0x2A,        //   Usage (0x2A)
    0x95, 0x09,        //   Report Count (9)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x83,        //   Report ID (-125)
    0x09, 0x2B,        //   Usage (0x2B)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x84,        //   Report ID (-124)
    0x09, 0x2C,        //   Usage (0x2C)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x85,        //   Report ID (-123)
    0x09, 0x2D,        //   Usage (0x2D)
    0x95, 0x02,        //   Report Count (2)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xA0,        //   Report ID (-96)
    0x09, 0x2E,        //   Usage (0x2E)
    0x95, 0x01,        //   Report Count (1)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xE0,        //   Report ID (-32)
    0x09, 0x2F,        //   Usage (0x2F)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF0,        //   Report ID (-16)
    0x09, 0x30,        //   Usage (0x30)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF1,        //   Report ID (-15)
    0x09, 0x31,        //   Usage (0x31)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF2,        //   Report ID (-14)
    0x09, 0x32,        //   Usage (0x32)
    0x95, 0x0F,        //   Report Count (15)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF4,        //   Report ID (-12)
    0x09, 0x35,        //   Usage (0x35)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF5,        //   Report ID (-11)
    0x09, 0x36,        //   Usage (0x36)
    0x95, 0x03,        //   Report Count (3)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};


static const uint8_t mayflashs5_custom_configuration_descriptor[] =
{
    0x09,        //   bLength
    0x02,        //   bDescriptorType (Configuration)
    0x29, 0x00,  //   wTotalLength 41
    0x01,        //   bNumInterfaces 1
    0x01,        //   bConfigurationValue
    0x00,        //   iConfiguration (String Index)
    0xC0,        //   bmAttributes                        //0x80,        //   bmAttributes
    0xFA,        //   bMaxPower 500mA

    0x09,        //   bLength
    0x04,        //   bDescriptorType (Interface)
    0x00,        //   bInterfaceNumber 0
    0x00,        //   bAlternateSetting
    0x02,        //   bNumEndpoints 2
    0x03,        //   bInterfaceClass
    0x00,        //   bInterfaceSubClass
    0x00,        //   bInterfaceProtocol
    0x00,        //   iInterface (String Index)

    0x09,        //   bLength
    0x21,        //   bDescriptorType (HID)
    0x11, 0x01,  //   bcdHID 1.11
    0x00,        //   bCountryCode
    0x01,        //   bNumDescriptors
    0x22,        //   bDescriptorType[0] (HID)
    0x01, 0x01,  //   wDescriptorLength[0] 257    //0x11, 0x01,  //   wDescriptorLength[0] 273

    0x07,        //   bLength
    0x05,        //   bDescriptorType (Endpoint)
    0x82,        //   bEndpointAddress (IN/D2H)
    0x03,        //   bmAttributes (Interrupt)
    0x40, 0x00,  //   wMaxPacketSize 64
    0x01,        //   bInterval 1 (unit depends on device speed)

    0x07,        //   bLength
    0x05,        //   bDescriptorType (Endpoint)
    0x01,        //   bEndpointAddress (OUT/H2D)
    0x03,        //   bmAttributes (Interrupt)
    0x40, 0x00,  //   wMaxPacketSize 64
    0x06,        //   bInterval 6 (unit depends on device speed)
};

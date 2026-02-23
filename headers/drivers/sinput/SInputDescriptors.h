/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#pragma once

#include <stdint.h>

#define HID_ENDPOINT_SIZE 64

// https://docs.handheldlegend.com/s/sinput/doc/sinput-hid-protocol-dev-SNSaEw36nc
#define VENDOR_ID		0x10C6
#define PRODUCT_ID		0x2E8A

/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

#define GAMEPAD_INTERFACE	0
#define GAMEPAD_ENDPOINT	1
#define GAMEPAD_SIZE		64

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

// https://docs.handheldlegend.com/s/sinput/doc/buttons-format-esVqGUAjpb
#define SINPUT_MASK_SOUTH       (1UL << 0)
#define SINPUT_MASK_EAST        (1UL << 1)
#define SINPUT_MASK_WEST        (1UL << 2)
#define SINPUT_MASK_NORTH       (1UL << 3)
#define SINPUT_MASK_DPAD_UP     (1UL << 4)
#define SINPUT_MASK_DPAD_DOWN   (1UL << 5)
#define SINPUT_MASK_DPAD_LEFT   (1UL << 6)
#define SINPUT_MASK_DPAD_RIGHT  (1UL << 7)
#define SINPUT_MASK_STICK_LEFT  (1UL << 8)
#define SINPUT_MASK_STICK_RIGHT (1UL << 9)
#define SINPUT_MASK_L_BUMPER    (1UL << 10)
#define SINPUT_MASK_R_BUMPER    (1UL << 11)
#define SINPUT_MASK_L_TRIGGER   (1UL << 12)
#define SINPUT_MASK_R_TRIGGER   (1UL << 13)
#define SINPUT_MASK_L_PADDLE_1  (1UL << 14)
#define SINPUT_MASK_R_PADDLE_1  (1UL << 15)
#define SINPUT_MASK_PLUS        (1UL << 16)
#define SINPUT_MASK_MINUS       (1UL << 17)
#define SINPUT_MASK_HOME        (1UL << 18)
#define SINPUT_MASK_CAPTURE     (1UL << 19)
#define SINPUT_MASK_L_PADDLE_2  (1UL << 20)
#define SINPUT_MASK_R_PADDLE_2  (1UL << 21)
#define SINPUT_MASK_TOUCHPAD_1  (1UL << 22)
#define SINPUT_MASK_TOUCHPAD_2  (1UL << 23)
#define SINPUT_MASK_MISC_3      (1UL << 24)
#define SINPUT_MASK_MISC_4      (1UL << 25)
#define SINPUT_MASK_MISC_5      (1UL << 26)
#define SINPUT_MASK_MISC_6      (1UL << 27)
#define SINPUT_MASK_MISC_7      (1UL << 28)
#define SINPUT_MASK_MISC_8      (1UL << 29)
#define SINPUT_MASK_MISC_9      (1UL << 30)
#define SINPUT_MASK_MISC_10     (1UL << 31)


// https://docs.handheldlegend.com/s/sinput/doc/input-0x01-mjqXVwnSNu
typedef struct __attribute((packed, aligned(1)))
{
	// Input report (Report ID: 1)
	uint8_t report_id;
	uint8_t plug_status;
	uint8_t charge_percent;
	uint32_t buttons;
	int16_t left_x;
	int16_t left_y;
	int16_t right_x;
	int16_t right_y;
	int16_t trigger_l;
	int16_t trigger_r;
	uint32_t imu_timestamp_us;
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
	int16_t touchpad_1_x;
	int16_t touchpad_1_y;
	int16_t touchpad_1_pressure;
	int16_t touchpad_2_x;
	int16_t touchpad_2_y;
	int16_t touchpad_2_pressure;
	uint8_t reserved_bulk[17];
} SInputReport;

static const uint8_t sinput_string_language[]     = { 0x09, 0x04 };
static const uint8_t sinput_string_manufacturer[] = "Open Stick Community";
static const uint8_t sinput_string_product[]      = "GP2040-CE (SInput)";
static const uint8_t sinput_string_version[]      = "1.0";

static const uint8_t *sinput_string_descriptors[] __attribute__((unused)) =
{
	sinput_string_language,
	sinput_string_manufacturer,
	sinput_string_product,
	sinput_string_version
};

static const uint8_t sinput_device_descriptor[] =
{
	18,								  // bLength
	1,								  // bDescriptorType
	0x00, 0x02,						  // bcdUSB
	0,								  // bDeviceClass
	0,								  // bDeviceSubClass
	0,								  // bDeviceProtocol
	HID_ENDPOINT_SIZE,				  // bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),	  // idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID), // idProduct
	0x00, 0x01,						  // bcdDevice
	1,								  // iManufacturer
	2,								  // iProduct
	0,								  // iSerialNumber
	1								  // bNumConfigurations
};

// https://docs.handheldlegend.com/s/sinput/doc/hid-descriptor-Te2bjLJk0N
static const uint8_t sinput_report_descriptor[] =
{
    0x05, 0x01,                    // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,                    // Usage (Gamepad)
    0xA1, 0x01,                    // Collection (Application)
    
    // INPUT REPORT ID 0x01 - Main gamepad data
    0x85, 0x01,                    //   Report ID (1)
    
    // Padding bytes (bytes 2-3) - Plug status and Charge Percent (0-100)
    0x06, 0x00, 0xFF,              //   Usage Page (Vendor Defined)
    0x09, 0x01,                    //   Usage (Vendor Usage 1)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0xFF,                    //   Logical Maximum (255)
    0x75, 0x08,                    //   Report Size (8)
    0x95, 0x02,                    //   Report Count (2)
    0x81, 0x02,                    //   Input (Data,Var,Abs)

    // --- 32 buttons ---
    0x05, 0x09,        // Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x20,        //   Usage Maximum (Button 32)
    0x15, 0x00,        //   Logical Min (0)
    0x25, 0x01,        //   Logical Max (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x20,        //   Report Count (32)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Analog Sticks and Triggers
    0x05, 0x01,                    // Usage Page (Generic Desktop)
    // Left Stick X (bytes 8-9)
    0x09, 0x30,                    //   Usage (X)
    // Left Stick Y (bytes 10-11)
    0x09, 0x31,                    //   Usage (Y)
    // Right Stick X (bytes 12-13)
    0x09, 0x32,                    //   Usage (Z)
    // Right Stick Y (bytes 14-15)
    0x09, 0x35,                    //   Usage (Rz)
    // Right Trigger (bytes 18-19)
    0x09, 0x33,                    //   Usage (Rx)
    // Left Trigger  (bytes 16-17)
    0x09, 0x34,                     //  Usage (Ry)
    0x16, 0x00, 0x80,              //   Logical Minimum (-32768)
    0x26, 0xFF, 0x7F,              //   Logical Maximum (32767)
    0x75, 0x10,                    //   Report Size (16)
    0x95, 0x06,                    //   Report Count (6)
    0x81, 0x02,                    //   Input (Data,Var,Abs)
    
    // Motion data and Reserved data (bytes 20-63) - 44 bytes
    // This includes gyro/accel data that apps can use if supported
    0x06, 0x00, 0xFF,              // Usage Page (Vendor Defined)
    
    // Motion Input Timestamp (Microseconds)
    0x09, 0x20,                    //   Usage (Vendor Usage 0x20)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x26, 0xFF, 0xFF,              //   Logical Maximum (655535)
    0x75, 0x20,                    //   Report Size (32)
    0x95, 0x01,                    //   Report Count (1)
    0x81, 0x02,                    //   Input (Data,Var,Abs)

    // Motion Input Accelerometer XYZ (Gs) and Gyroscope XYZ (Degrees Per Second)
    0x09, 0x21,                    //   Usage (Vendor Usage 0x21)
    0x16, 0x00, 0x80,              //   Logical Minimum (-32768)
    0x26, 0xFF, 0x7F,              //   Logical Maximum (32767)
    0x75, 0x10,                    //   Report Size (16)
    0x95, 0x06,                    //   Report Count (6)
    0x81, 0x02,                    //   Input (Data,Var,Abs)

    // Reserved padding
    0x09, 0x22,                    //   Usage (Vendor Usage 0x22)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x26, 0xFF, 0x00,              //   Logical Maximum (255)
    0x75, 0x08,                    //   Report Size (8)
    0x95, 0x1D,                    //   Report Count (29)
    0x81, 0x02,                    //   Input (Data,Var,Abs)
    
    // INPUT REPORT ID 0x02 - Vendor COMMAND data
    0x85, 0x02,                    //   Report ID (2)
    0x09, 0x23,                    //   Usage (Vendor Usage 0x23)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x26, 0xFF, 0x00,              //   Logical Maximum (255)
    0x75, 0x08,                    //   Report Size (8 bits)
    0x95, 0x3F,                    //   Report Count (63) - 64 bytes minus report ID
    0x81, 0x02,                    //   Input (Data,Var,Abs)

    // OUTPUT REPORT ID 0x03 - Vendor COMMAND data
    0x85, 0x03,                    //   Report ID (3)
    0x09, 0x24,                    //   Usage (Vendor Usage 0x24)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x26, 0xFF, 0x00,              //   Logical Maximum (255)
    0x75, 0x08,                    //   Report Size (8 bits)
    0x95, 0x2F,                    //   Report Count (47) - 48 bytes minus report ID
    0x91, 0x02,                    //   Output (Data,Var,Abs)

    0xC0                           // End Collection 
};

#define CONFIG1_DESC_SIZE		(9+9+9+7)
static const uint8_t sinput_configuration_descriptor[] =
{
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9,						       // bLength;
	2,						       // bDescriptorType;
	LSB(CONFIG1_DESC_SIZE),				       // wTotalLength
	MSB(CONFIG1_DESC_SIZE),
	1,						       // bNumInterfaces
	1,						       // bConfigurationValue
	0,						       // iConfiguration
	0x80,						       // bmAttributes
	50,						       // bMaxPower
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,						       // bLength
	4,						       // bDescriptorType
	GAMEPAD_INTERFACE,				       // bInterfaceNumber
	0,						       // bAlternateSetting
	1,						       // bNumEndpoints
	0x03,						       // bInterfaceClass (0x03 = HID)
	0x00,						       // bInterfaceSubClass (0x00 = No Boot)
	0x00,						       // bInterfaceProtocol (0x00 = No Protocol)
	0,						       // iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,						       // bLength
	0x21,						       // bDescriptorType
	0x11, 0x01,					       // bcdHID
	0,						       // bCountryCode
	1,						       // bNumDescriptors
	0x22,						       // bDescriptorType
	LSB(sizeof(sinput_report_descriptor)),			       // wDescriptorLength
	MSB(sizeof(sinput_report_descriptor)),
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,						       // bLength
	5,						       // bDescriptorType
	GAMEPAD_ENDPOINT | 0x80,			       // bEndpointAddress
	0x03,						       // bmAttributes (0x03=intr)
	GAMEPAD_SIZE, 0,				       // wMaxPacketSize
	1						       // bInterval (1 ms)
};

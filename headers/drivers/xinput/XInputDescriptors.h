/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>
#include <pico/unique_id.h>

#define XINPUT_ENDPOINT_SIZE 20

// Buttons 1 (8 bits)
// TODO: Consider using an enum class here.
#define XBOX_MASK_UP    (1U << 0)
#define XBOX_MASK_DOWN  (1U << 1)
#define XBOX_MASK_LEFT  (1U << 2)
#define XBOX_MASK_RIGHT (1U << 3)
#define XBOX_MASK_START (1U << 4)
#define XBOX_MASK_BACK  (1U << 5)
#define XBOX_MASK_LS    (1U << 6)
#define XBOX_MASK_RS    (1U << 7)

// Buttons 2 (8 bits)
// TODO: Consider using an enum class here.
#define XBOX_MASK_LB    (1U << 0)
#define XBOX_MASK_RB    (1U << 1)
#define XBOX_MASK_HOME  (1U << 2)
//#define UNUSED        (1U << 3)
#define XBOX_MASK_A     (1U << 4)
#define XBOX_MASK_B     (1U << 5)
#define XBOX_MASK_X     (1U << 6)
#define XBOX_MASK_Y     (1U << 7)

typedef struct __attribute((packed, aligned(1)))
{
    uint8_t report_id;
    uint8_t report_size;
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t lt;
    uint8_t rt;
    int16_t lx;
    int16_t ly;
    int16_t rx;
    int16_t ry;
    uint8_t _reserved[6];
} XInputReport;

static const uint8_t xinput_string_language[]    = { 0x09, 0x04 };
static const uint8_t xinput_string_serial[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t xinput_string_manfacturer[] = "\xa9Microsoft Corporation";
static const uint8_t xinput_string_product[]     = "Controller";
static uint8_t xinput_string_version[]     = "08FEC93"; // Serial that is overridden by Pico ID
static const uint8_t xinput_string_xsm3[]        = "Xbox Security Method 3, Version 1.00, \xa9 2005 Microsoft Corporation. All rights reserved.";

static const uint8_t *xinput_string_descriptors[] __attribute__((unused)) =
{
    xinput_string_serial,
    xinput_string_manfacturer,
    xinput_string_product,
    xinput_string_version,
    xinput_string_xsm3
};

static const uint8_t * xinput_get_string_descriptor(int index) {
    if ( index == 3 ) {
        // Generate a serial number of hex bytes from the pico's unique ID
        pico_unique_board_id_t unique_id;
        pico_get_unique_board_id(&unique_id);
        for(int i = 0; i < 3; i++) {
            sprintf((char*)(&xinput_string_version[i*2+1]), "%02X", (uint8_t)unique_id.id[i+5]);
        }
    }

    return xinput_string_descriptors[index];
}

static const uint8_t xinput_device_descriptor[] =
{
    0x12,       // bLength
    0x01,       // bDescriptorType (Device)
    0x00, 0x02, // bcdUSB 2.00
    0xFF,          // bDeviceClass
    0xFF,          // bDeviceSubClass
    0xFF,          // bDeviceProtocol
    0x40,          // bMaxPacketSize0 64
    0x5E, 0x04, // idVendor 0x045E
    0x8E, 0x02, // idProduct 0x028E
    0x14, 0x01, // bcdDevice 2.14
    0x01,       // iManufacturer (String Index)
    0x02,       // iProduct (String Index)
    0x03,       // iSerialNumber (String Index)
    0x01,       // bNumConfigurations 1
};

// This needs to be:
 // 4 interfaces
 // remote wakeup enabled
static const uint8_t xinput_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x99, 0x00,  // wTotalLength 0x99
    0x04,        // bNumInterfaces 4
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0xA0,        // bmAttributes (remote wakeup)
    0xFA,        // bMaxPower 500mA

    // Control Interface (0x5D 0xFF)
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0xFF,        // bInterfaceClass
    0x5D,        // bInterfaceSubClass
    0x01,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    // Gamepad Descriptor
    0x11,        // bLength
    0x21,        // bDescriptorType (HID)
    0x00, 0x01,  // bcdHID 1.10
    0x01,        // SUB_TYPE
    0x25,        // reserved2
    0x81,        // DEVICE_EPADDR_IN
    0x14,        // bMaxDataSizeIn
    0x00, 0x00, 0x00, 0x00, 0x13, // reserved3
    0x02,        // DEVICE_EPADDR_OUT is this right?
    0x08,        // bMaxDataSizeOut
    0x00, 0x00,  // reserved4

    // Report IN Endpoint 1.1
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x01,        // bInterval 1 (unit depends on device speed)

    // Report OUT Endpoint 1.2
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x02,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x08,        // bInterval 8 (unit depends on device speed)

    // Interface Audio
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x01,        // bInterfaceNumber 1
    0x00,        // bAlternateSetting
    0x04,        // bNumEndpoints 4
    0xFF,        // bInterfaceClass
    0x5D,        // bInterfaceSubClass
    0x03,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    // Audio Descriptor
    0x1B,        // bLength
    0x21,
    0x00,        
    0x01,
    0x01,
    0x01,
    0x83,        // XINPUT_MIC_IN
    0x40,        // ??
    0x01,        // ??
    0x04,        // XINPUT_AUDIO_OUT
    0x20,        // ??
    0x16,        // ??
    0x85,        // XINPUT_UNK_IN
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x16,
    0x06,        // XINPUT_UNK_OUT
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    // Report IN Endpoint 2.1
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x83,        // bEndpointAddress (XINPUT_MIC_IN)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x02,        // bInterval 2 (unit depends on device speed)

    // Report OUT Endpoint 2.2
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x04,        // bEndpointAddress (XINPUT_AUDIO_OUT)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x04,        // bInterval 4 (unit depends on device speed)

    // Report IN Endpoint 2.3
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x85,        // bEndpointAddress (XINPUT_UNK_IN)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x40,        // bInterval 128

    // Report OUT Endpoint 2.4
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x06,        // bEndpointAddress (XINPUT_UNK_OUT)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x10,        // bInterval 16

    // Interface Plugin Module
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x02,        // bInterfaceNumber 2
    0x00,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    0xFF,        // bInterfaceClass
    0x5D,        // bInterfaceSubClass
    0x02,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    //PluginModuleDescriptor : {
    0x09,        // bLength
    0x21,        // bDescriptorType
    0x00, 0x01,  // version 1.00
    0x01,        // ??
    0x22,        // ??
    0x86,        // XINPUT_PLUGIN_MODULE_IN,
    0x03,        // ??
    0x00,        // ??

    // Report IN Endpoint 3.1
    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x86,        // bEndpointAddress (XINPUT_PLUGIN_MODULE_IN)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x10,        // bInterval 8 (unit depends on device speed)

    // Interface Security
    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x03,        // bInterfaceNumber 3
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    0xFF,        // bInterfaceClass
    0xFD,        // bInterfaceSubClass
    0x13,        // bInterfaceProtocol
    0x04,        // iInterface (String Index)

    // SecurityDescriptor (XSM3)
    0x06,        // bLength
    0x41,        // bDescriptType (Xbox 360)
    0x00,
    0x01,
    0x01,
    0x03,  
};

typedef enum
{
    XSM360_AUTH_NONE                  = 0x00,    // None
    XSM360_GET_SERIAL                 = 0x81,    // Xbox 360 Get Controller Serial
    XSM360_INIT_AUTH                  = 0x82,    // Xbox 360 Initialize Authentication
    XSM360_RESPOND_CHALLENGE          = 0x83,    // Xbox 360 Respond with Challenge
    XSM360_AUTH_KEEPALIVE             = 0x84,    // Xbox 360 Keep Authentication Alive
    XSM360_REQUEST_STATE              = 0x86,    // Xbox 360 Request Authentication State
    XSM360_VERIFY_AUTH                = 0x87,    // Xbox 360 Verify Authentication
    
} XSM360AuthRequest;

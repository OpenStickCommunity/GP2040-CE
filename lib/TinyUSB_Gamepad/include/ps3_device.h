#ifndef PS3_DEVICE_H_
#define PS3_DEVICE_H_

#include <stdint.h>

#define PS3_MASK_SELECT   (1U <<  0)
#define PS3_MASK_L3       (1U <<  1)
#define PS3_MASK_R3       (1U <<  2)
#define PS3_MASK_START    (1U <<  3)
#define PS3_MASK_UP       (1U <<  4)
#define PS3_MASK_RIGHT    (1U <<  5)
#define PS3_MASK_DOWN     (1U <<  6)
#define PS3_MASK_LEFT     (1U <<  7)
#define PS3_MASK_L2       (1U <<  8)
#define PS3_MASK_R1       (1U <<  9)
#define PS3_MASK_L1       (1U << 10)
#define PS3_MASK_R2       (1U << 11)
#define PS3_MASK_TRIANGLE (1U << 12)
#define PS3_MASK_CIRCLE   (1U << 13)
#define PS3_MASK_CROSS    (1U << 14)
#define PS3_MASK_SQUARE   (1U << 15)
#define PS3_MASK_PS       (1U << 16)

#define PS3_JOYSTICK_MIN 0x00
#define PS3_JOYSTICK_MID 0x80
#define PS3_JOYSTICK_MAX 0xFF

#define PS3_ACCEL_GYRO_MID 0x01FF

typedef struct __attribute((packed))
{
	uint8_t reportId;
	uint32_t buttons;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
	uint8_t analogDpadRight;
	uint8_t analogDpadLeft;
	uint8_t analogDpadUp;
	uint8_t analogDpadDown;
	uint8_t analogTriangle;
	uint8_t analogCircle;
	uint8_t analogCross;
	uint8_t analogSquare;
	uint8_t analogL1;
	uint8_t analogL2;
	uint8_t analogR1;
	uint8_t analogR2;
	uint8_t _reserved[19];
	uint16_t accelX;        // 10 bit
	uint16_t accellY;       // 10 bit
	uint16_t accelZ;        // 10 bit
	uint16_t gyro;          // 10 bit
} PS3Report;

extern PS3Report ps3_report;

static const char ps3_string_manufacturer[] = "Sony";
static const char ps3_string_product[]      = "PLAYSTATION(R)3 Controller";
// static const char ps3_string_manufacturer[] = "Generic";
// static const char ps3_string_product[]      = "PS3 Controller";
static const char ps3_string_version[]      = "1.0";

static const char *ps3_string_descriptors[] =
{
	(const char[]){ 0x09, 0x04 },
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
	// 0xFE, 0xCA,  // Test VID...
	// 0xEA, 0x01,  // ...and PID
	0x4C, 0x05,  // idVendor 0x054C
	0x68, 0x02,  // idProduct 0x0268
	0x00, 0x01,  // bcdDevice 1.00
	0x01,        // iManufacturer (String Index)
	0x02,        // iProduct (String Index)
	0x00,        // iSerialNumber (String Index)
	0x01,        // bNumConfigurations 1
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

#endif

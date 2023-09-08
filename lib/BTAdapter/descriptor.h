#include <stdint.h>
#include <string.h>

#ifndef BT_DESCRIPTORS_H_
#define BT_DESCRIPTORS_H_


// This is a combination of a personnaly Wireshark dumped Xbone One HID Descriptor
// and one I found online (that was slightly different, not sure if it matters)
// If you break this, check device manager for a broken HID device, it'll tell you what's wrong
const uint8_t xboneReport[] = {
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x05,        // Usage (Game Controls)
0xA1, 0x01,        // Collection (Application)
0x85, 0x01,        //   Report ID (1)
0xA1, 0x00,        //   Collection (Physical)
0x09, 0x30,        //     Usage (0x30)
0x09, 0x31,        //     Usage (0x31)
0x15, 0x00,        //     Logical Minimum (0)
0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
0x95, 0x02,        //     Report Count (2)
0x75, 0x10,        //     Report Size (16)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0xA1, 0x00,        //   Collection (Physical)
0x09, 0x33,        //     Usage (0x33)
0x09, 0x34,        //     Usage (0x34)
0x15, 0x00,        //     Logical Minimum (0)
0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
0x95, 0x02,        //     Report Count (2)
0x75, 0x10,        //     Report Size (16)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
0x09, 0x32,        //   Usage (Z)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
0x95, 0x01,        //   Report Count (1)
0x75, 0x0A,        //   Report Size (10)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x00,        //   Logical Maximum (0)
0x75, 0x06,        //   Report Size (6)
0x95, 0x01,        //   Report Count (1)
0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
0x09, 0x35,        //   Usage (Rz)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
0x95, 0x01,        //   Report Count (1)
0x75, 0x0A,        //   Report Size (10)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x00,        //   Logical Maximum (0)
0x75, 0x06,        //   Report Size (6)
0x95, 0x01,        //   Report Count (1)
0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
0x09, 0x39,        //   Usage (Hat switch)
0x15, 0x01,        //   Logical Minimum (1)
0x25, 0x08,        //   Logical Maximum (8)
0x35, 0x00,        //   Physical Minimum (0)
0x46, 0x3B, 0x01,  //   Physical Maximum (315)
0x66, 0x14, 0x00,  //   Unit (System: English Rotation, Length: Centimeter)
0x75, 0x04,        //   Report Size (4)
0x95, 0x01,        //   Report Count (1)
0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
0x75, 0x04,        //   Report Size (4)
0x95, 0x01,        //   Report Count (1)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x00,        //   Logical Maximum (0)
0x35, 0x00,        //   Physical Minimum (0)
0x45, 0x00,        //   Physical Maximum (0)
0x65, 0x00,        //   Unit (None)
0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x09,        //   Usage Page (Button)
0x19, 0x01,        //   Usage Minimum (0x01)
0x29, 0x0A,        //   Usage Maximum (0x0A)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x0A,        //   Report Count (10)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x00,        //   Logical Maximum (0)
0x75, 0x06,        //   Report Size (6)
0x95, 0x01,        //   Report Count (1)
0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
0x09, 0x80,        //   Usage (Sys Control)
0x85, 0x02,        //   Report ID (2)
0xA1, 0x00,        //   Collection (Physical)
0x09, 0x85,        //     Usage (Sys Main Menu)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x95, 0x01,        //     Report Count (1)
0x75, 0x01,        //     Report Size (1)
0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x00,        //     Logical Maximum (0)
0x75, 0x07,        //     Report Size (7)
0x95, 0x01,        //     Report Count (1)
0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              //   End Collection
0x05, 0x0F,        //   Usage Page (PID Page)
0x09, 0x21,        //   Usage (0x21)
0x85, 0x03,        //   Report ID (3)
0xA1, 0x02,        //   Collection (Logical)
0x09, 0x97,        //     Usage (0x97)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x01,        //     Logical Maximum (1)
0x75, 0x04,        //     Report Size (4)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x00,        //     Logical Maximum (0)
0x75, 0x04,        //     Report Size (4)
0x95, 0x01,        //     Report Count (1)
0x91, 0x03,        //     Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x70,        //     Usage (0x70)
0x15, 0x00,        //     Logical Minimum (0)
0x25, 0x64,        //     Logical Maximum (100)
0x75, 0x08,        //     Report Size (8)
0x95, 0x04,        //     Report Count (4)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0x50,        //     Usage (0x50)
0x66, 0x01, 0x10,  //     Unit (System: SI Linear, Time: Seconds)
0x55, 0x0E,        //     Unit Exponent (-2)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x09, 0xA7,        //     Usage (0xA7)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x65, 0x00,        //     Unit (None)
0x55, 0x00,        //     Unit Exponent (0)
0x09, 0x7C,        //     Usage (0x7C)
0x15, 0x00,        //     Logical Minimum (0)
0x26, 0xFF, 0x00,  //     Logical Maximum (255)
0x75, 0x08,        //     Report Size (8)
0x95, 0x01,        //     Report Count (1)
0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              //   End Collection
0x85, 0x04,        //   Report ID (4)
0x05, 0x06,        //   Usage Page (Generic Dev Ctrls)
0x09, 0x20,        //   Usage (Battery Strength)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x75, 0x08,        //   Report Size (8)
0x95, 0x01,        //   Report Count (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
};

/* Xbone Reports:
1:
  X,Y [IN]: uint16_t[2] [0,65534]
  rX,rY [IN]: uint16_t[2] [0,65534]
  Z [IN]: bit[10]
  Const: bit[6]
  rZ [IN]: bit[10]
  Const: bit[6]
  Hat switch a: uint4[1] (centimeters?)
  Const: bit[4]
  Buttons: bit[10]
  Const: bit[6]
2: 
  Sys Main Menu [IN]: bit[1]
  Const: bit[7]
3:
  "0x97 A" [OUT]: bit[4] (enable actuators)
  Const: bit[4]
  "0x70" [OUT]: uint8_t[4] [0,100] (actuator magnitudes)
  "0x50" [OUT]: uint8_t[1] [0,255] (time?) (actuator duration)
  "0xA7" [OUT]: uint8_t[1] [0,255] (time?) (acturator delay)
  "0x7C" [OUT]: uint8_t[1] [0,255] (actuator loop count)
4:
  Battery Strength [IN]: uint8_t[1] [0,255]

*/

// For 2 Refined, we'll encode your directional inputs in an even worse way
uint8_t makeHat(bool up, bool right, bool down, bool left){
  constexpr const int shift = 0; 
  if(up){
    if(right) return 2u << shift;
    else if(left) return 8u << shift;
    else return 1u << shift;
  }
  else if(down){
    if(right) return 4u << shift;
    else if(left) return 6u << shift;
    else return 5u << shift;
  }
  else {
    if(right) return 3u << shift;
    else if(left) return 7u << shift;
    else return 0u << shift;
  }
}

uint8_t makeSMM(bool smm){
  return smm ? 127 : 0;
}

struct XBoneData {
  uint16_t X = (1<<15), Y = (1<<15), Rx = (1<<15), Ry = (1<<15), Z = (1<<9), Rz = (1<<9);
  uint8_t Hat = 0x00;
  uint16_t Buttons = 0x00;
  uint8_t SMM = 0x00;
  uint8_t Battery = 255;

  void makeReportOne(uint8_t (&buffer)[17]) {
    // there's probably a better way to do this, but its like 2AM
    buffer[0] = 0xa1;
    buffer[1] = 0x01;

    buffer[2] = X & 0xff;
    buffer[3] = (X >> 8) & 0xff;
    buffer[4] = Y & 0xff;
    buffer[5] = (Y >> 8) & 0xff;
    buffer[6] = Rx & 0xff;
    buffer[7] = (Rx >> 8) & 0xff;
    buffer[8] = Ry & 0xff;
    buffer[9] = (Ry >> 8) & 0xff;

    buffer[10] = (Z >> 6) & 0xff;
    buffer[11] = (Z >> 14) & 0xff;
    buffer[12] = (Rz >> 6) & 0xff;
    buffer[13] = (Rz >> 14) & 0xff;

    buffer[14] = Hat;

    buffer[15] = Buttons & 0xff;
    buffer[16] = 0x00;
  }
  void makeReportTwo(uint8_t (&buffer)[4]) {
    buffer[0] = 0xa1;
    buffer[1] = 0x02;

    buffer[2] = SMM;

    buffer[3] = 0x00;
  }
  void makeReportFour(uint8_t (&buffer)[4]) {
    buffer[0] = 0xa1;
    buffer[1] = 0x03;

    buffer[2] = Battery;

    buffer[3] = 0x00;
  }
};

#endif
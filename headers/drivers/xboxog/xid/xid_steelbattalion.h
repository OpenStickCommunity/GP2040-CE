#ifndef XID_STEELBATTALION_H_
#define XID_STEELBATTALION_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <tusb.h>

/* https://github.com/Cxbx-Reloaded/Cxbx-Reloaded/blob/master/src/core/hle/XAPI/XapiCxbxr.h */
#define    CXBX_SBC_GAMEPAD_W0_RIGHTJOYMAINWEAPON      0x0001
#define    CXBX_SBC_GAMEPAD_W0_RIGHTJOYFIRE            0x0002
#define    CXBX_SBC_GAMEPAD_W0_RIGHTJOYLOCKON          0x0004
#define    CXBX_SBC_GAMEPAD_W0_EJECT                   0x0008
#define    CXBX_SBC_GAMEPAD_W0_COCKPITHATCH            0x0010
#define    CXBX_SBC_GAMEPAD_W0_IGNITION                0x0020
#define    CXBX_SBC_GAMEPAD_W0_START                   0x0040
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONOPENCLOSE       0x0080
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONMAPZOOMINOUT    0x0100
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONMODESELECT      0x0200
#define    CXBX_SBC_GAMEPAD_W0_MULTIMONSUBMONITOR      0x0400
#define    CXBX_SBC_GAMEPAD_W0_MAINMONZOOMIN           0x0800
#define    CXBX_SBC_GAMEPAD_W0_MAINMONZOOMOUT          0x1000
#define    CXBX_SBC_GAMEPAD_W0_FUNCTIONFSS             0x2000
#define    CXBX_SBC_GAMEPAD_W0_FUNCTIONMANIPULATOR     0x4000
#define    CXBX_SBC_GAMEPAD_W0_FUNCTIONLINECOLORCHANGE 0x8000

#define    CXBX_SBC_GAMEPAD_W1_WASHING                 0x0001
#define    CXBX_SBC_GAMEPAD_W1_EXTINGUISHER            0x0002
#define    CXBX_SBC_GAMEPAD_W1_CHAFF                   0x0004
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONTANKDETACH      0x0008
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONOVERRIDE        0x0010
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONNIGHTSCOPE      0x0020
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONF1              0x0040
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONF2              0x0080
#define    CXBX_SBC_GAMEPAD_W1_FUNCTIONF3              0x0100
#define    CXBX_SBC_GAMEPAD_W1_WEAPONCONMAIN           0x0200
#define    CXBX_SBC_GAMEPAD_W1_WEAPONCONSUB            0x0400
#define    CXBX_SBC_GAMEPAD_W1_WEAPONCONMAGAZINE       0x0800
#define    CXBX_SBC_GAMEPAD_W1_COMM1                   0x1000
#define    CXBX_SBC_GAMEPAD_W1_COMM2                   0x2000
#define    CXBX_SBC_GAMEPAD_W1_COMM3                   0x4000
#define    CXBX_SBC_GAMEPAD_W1_COMM4                   0x8000

#define    CXBX_SBC_GAMEPAD_W2_COMM5                   0x0001
#define    CXBX_SBC_GAMEPAD_W2_LEFTJOYSIGHTCHANGE      0x0002
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEFILTERCONTROL     0x0004
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEOXYGENSUPPLY      0x0008
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEFUELFLOWRATE      0x0010
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEBUFFREMATERIAL    0x0020
#define    CXBX_SBC_GAMEPAD_W2_TOGGLEVTLOCATION        0x0040


typedef struct __attribute__((packed))
{
	uint8_t zero;
	uint8_t bLength;
	uint16_t dButtons[3];
	uint16_t aimingX;       //0 to 2^16 left to right
	uint16_t aimingY;       //0 to 2^16 top to bottom
	int16_t rotationLever;
	int16_t sightChangeX;
	int16_t sightChangeY;
	uint16_t leftPedal;      //Sidestep, 0x0000 to 0xFF00
	uint16_t middlePedal;    //Brake, 0x0000 to 0xFF00
	uint16_t rightPedal;     //Acceleration, 0x0000 to oxFF00
	int8_t tunerDial;        //0-15 is from 9oclock, around clockwise
	int8_t gearLever;        //7-13 is gears R,1,2,3,4,5
} USB_SteelBattalion_InReport_t;

typedef struct
{
	uint8_t zero;
	uint8_t bLength;
	uint8_t CockpitHatch_EmergencyEject;
	uint8_t Start_Ignition;
	uint8_t MapZoomInOut_OpenClose;
	uint8_t SubMonitorModeSelect_ModeSelect;
	uint8_t MainMonitorZoomOut_MainMonitorZoomIn;
	uint8_t Manipulator_ForecastShootingSystem;
	uint8_t Washing_LineColorChange;
	uint8_t Chaff_Extinguisher;
	uint8_t Override_TankDetach;
	uint8_t F1_NightScope;
	uint8_t F3_F2;
	uint8_t SubWeaponControl_MainWeaponControl;
	uint8_t Comm1_MagazineChange;
	uint8_t Comm3_Comm2;
	uint8_t Comm5_Comm4;
	uint8_t GearR_;
	uint8_t Gear1_GearN;
	uint8_t Gear3_Gear2;
	uint8_t Gear5_Gear4;
	uint8_t dummy;
} USB_SteelBattalion_OutReport_t;

#define TUD_XID_SB_DESC_LEN  (9+7+7)

#define TUD_XID_SB_DESCRIPTOR(_itfnum, _epout, _epin) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 2, XID_INTERFACE_CLASS, XID_INTERFACE_SUBCLASS, 0x00, 0x00,\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(32), 4, \
  /* Endpoint Out */\
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(32), 4

static const uint8_t STEELBATTALION_DESC_XID[] = {
    0x10,
    0x42,
    0x00, 0x01,
    0x80,
    0x01,
    0x1A,
    0x16,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static const uint8_t STEELBATTALION_CAPABILITIES_IN[] = {
    0x00,
    0x1A,
    0xFF,
    0xFF,
    0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF
};

static const uint8_t STEELBATTALION_CAPABILITIES_OUT[] = {
    0x00,
    0x16,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF
};

#ifdef __cplusplus
}
#endif

#endif
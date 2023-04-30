#ifndef _WIIExtensionAddon_H
#define _WIIExtensionAddon_H

#include <string>
#include <stdint.h>
#include <hardware/i2c.h>
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"
#include "WiiExtension.h"

// WiiExtension Module Name
#define WiiExtensionName "WiiExtension"

#ifndef WII_EXTENSION_ENABLED
#define WII_EXTENSION_ENABLED 1
#endif

#ifndef WII_EXTENSION_I2C_ADDR
#define WII_EXTENSION_I2C_ADDR 0x52
#endif

#ifndef WII_EXTENSION_I2C_SDA_PIN
#define WII_EXTENSION_I2C_SDA_PIN 16
#endif

#ifndef WII_EXTENSION_I2C_SCL_PIN
#define WII_EXTENSION_I2C_SCL_PIN 17
#endif

#ifndef WII_EXTENSION_I2C_BLOCK
#define WII_EXTENSION_I2C_BLOCK i2c0
#endif

#ifndef WII_EXTENSION_I2C_SPEED
#define WII_EXTENSION_I2C_SPEED 400000
#endif

/*
111                - controller id
   111111111111111 - controller button offset
111111111111111111 - gamepad set bit mask


const WII_DECODE_INPUT = (eo,to,values) => {
    if ((!values.wiiExtensionTypeButtons) || (typeof values.wiiExtensionTypeButtons !== 'object')) {
        values.wiiExtensionTypeButtons = [{id:0,controllerMask:[],buttonMask:[]},{id:1,controllerMask:[],buttonMask:[]},{id:2,controllerMask:[],buttonMask:[]}];
    }
    var isSelectedInput = values.wiiExtensionTypeButtons !== undefined && 
        values.wiiExtensionTypeButtons.filter((bo,bi) => bo.id === eo.id) !== undefined &&
        values.wiiExtensionTypeButtons.filter((bo,bi) => bo.id === eo.id).length > 0 &&
        values.wiiExtensionTypeButtons.filter((bo,bi) => bo.id === eo.id)[0].value !== -1;
    var resultButtonMask = -1;

    var controller = values.wiiExtensionTypeButtons.filter((bo,bi) => bo.id === eo.id)[0];
    // input on attached extension controller
    var controllerMask = parseInt(controller.controllerMask);
    // index of set bit relates to position in enum
    var buttonMask = parseInt(controller.buttonMask);
    // used input bits on controller
    var controllerInputBit = parseInt(to.value);
    // position of controller bit in enum
    var controllerInputBitIndex = eo.value.map((bo,bi) => (bo.value === controllerInputBit) ? bi : -1).filter((bo,bi) => bo !== -1)[0];

    if (isSelectedInput && controllerMask !== -1 && ((controllerMask & controllerInputBit) === (controllerInputBit))) {
        // masks in use by defined button mask
        var selectedMasks = BUTTON_MASKS.map((o,i) => ((o.value !== 0) && ((buttonMask & o.value) === o.value)) ? i : -1).filter((o,i) => o !== -1);
        // buttons in used by controller mask
        var selectedButtons = eo.value.map((o,i) => ((o.value !== 0) && ((controllerMask & o.value) === o.value)) ? i : -1).filter((o,i) => o !== -1);

        // return the value of the button defined in the mask
        if (controllerInputBitIndex !== undefined && selectedMasks !== undefined) {
            // return the mask of the index of button mask enum based on the position of the controller in the controller enum
            //console.dir(controllerInputBit);
            //console.dir(controllerInputBitIndex);
            //console.dir(selectedButtons);
            //console.dir(selectedMasks);
            //console.dir(selectedButtons.indexOf(controllerInputBitIndex));
            console.dir(selectedMasks[selectedButtons.indexOf(controllerInputBitIndex)]);
            if (selectedMasks[selectedButtons.indexOf(controllerInputBitIndex)] === undefined) {
            } else {
                resultButtonMask = BUTTON_MASKS[selectedMasks[selectedButtons.indexOf(controllerInputBitIndex)]].value;
            }
        }
    }

    return (isSelectedInput ? resultButtonMask : -1);
};
*/

typedef struct {
    int8_t id = -1;
    int16_t controllerMask = -1;
    int16_t buttonMask = -1;
} WiiExtensionProfileEntry;

typedef struct {
    WiiExtensionProfileEntry controller1;
    WiiExtensionProfileEntry controller2;
    WiiExtensionProfileEntry controller3;
} WiiExtensionProfiles;

static WiiExtensionProfileEntry WII_EXTENSION_DEFAULT_PROFILE = {};

class WiiExtensionInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // WiiExtension Setup
	virtual void process();     // WiiExtension Process
	virtual void preprocess() {}
	virtual std::string name() { return WiiExtensionName; }
private:
    WiiExtension * wii;
    uint32_t uIntervalMS;
    uint32_t nextTimer;

    bool buttonC = false;
    bool buttonZ = false;

    bool buttonA = false;
    bool buttonB = false;
    bool buttonX = false;
    bool buttonY = false;
    bool buttonL = false;
    bool buttonZL = false;
    bool buttonR = false;
    bool buttonZR = false;

    bool buttonSelect = false;
    bool buttonStart = false;
    bool buttonHome = false;

    bool dpadUp     = false;
    bool dpadDown   = false;
    bool dpadLeft   = false;
    bool dpadRight  = false;

    uint16_t triggerLeft  = 0;
    uint16_t triggerRight = 0;

    uint16_t leftX = 0;
    uint16_t leftY = 0;
    uint16_t rightX = 0;
    uint16_t rightY = 0;

    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
};

#endif  // _WIIExtensionAddon_H
#include "addons/i2canalog1219.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#define ADS_MAX (float)((1 << 23) - 1)
#define VREF_VOLTAGE 2.048f

bool I2CAnalog1219Input::available() {
    const AnalogADS1219Options& options = Storage::getInstance().getAddonOptions().analogADS1219Options;
    if (options.enabled) {
        ads = new ADS1219Device();
        PeripheralI2CScanResult result = PeripheralManager::getInstance().scanForI2CDevice(ads->getDeviceAddresses());
        if (result.address > -1) {
            ads->setAddress(result.address);
            ads->setI2C(PeripheralManager::getInstance().getI2C(result.block));
            return true;
        } else {
            delete ads;
        }
    }
    return false;
}

void I2CAnalog1219Input::setup() {
    const AnalogADS1219Options& options = Storage::getInstance().getAddonOptions().analogADS1219Options;

    memset(&pins, 0, sizeof(ADS_PINS));
    channelHop = 0;

    uIntervalMS = 1;
    nextTimer = getMillis();

    // Init our ADS1219 library
    ads->begin();                               // setup I2C and chip start
    ads->setChannel(0);                         // Start on Channel 0
    ads->setConversionMode(CONTINUOUS);         // Read analog continuously
    ads->setGain(ONE);                          // Set gain to 1
    ads->setDataRate(1000);                     // 1mhz (1.1ms delay)
    ads->setVoltageReference(REF_INTERNAL);     // Use internal VREF for now
    ads->start();                               // START/SYNC command
}

void I2CAnalog1219Input::process()
{
    if (nextTimer < getMillis()) {
        float result;
        uint32_t readValue;
        if ( ads->readRegister(STATUS) & REGISTER_STATUS_DRDY ) {
            readValue = ads->readConversionResult();
            result = readValue / float(ADS_MAX); // gives us 0.0f to 1.0f (actual voltage is times voltage)
            pins.A[channelHop] = result;
            channelHop = (channelHop+1) % 4; // Loop 0-3
            ads->setChannel(channelHop);
            nextTimer = getMillis() + uIntervalMS; // interval for read (we can't be too fast)
        }
    }

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    gamepad->state.lx = (uint16_t)(65535.f*pins.A[0]);
    gamepad->state.ly = (uint16_t)(65535.f*pins.A[1]);
    gamepad->state.rx = (uint16_t)(65535.f*pins.A[2]);
    gamepad->state.ry = (uint16_t)(65535.f*pins.A[3]);

}

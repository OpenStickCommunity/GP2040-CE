#ifndef _Amiibo_H
#define _Amiibo_H

#include "gpaddon.h"

#define AmiiboName "Amiibo"

#define AMIIBO_SLOT_COUNT     4
#define AMIIBO_DATA_SIZE      540
#define AMIIBO_SIGNATURE_SIZE 32
#define AMIIBO_FILE_SIZE      (AMIIBO_DATA_SIZE + AMIIBO_SIGNATURE_SIZE)
#define AMIIBO_NAME_SIZE      32
#define AMIIBO_TAP_MS         2000
#define AMIIBO_KEY_SIZE       80

class AmiiboAddon : public GPAddon {
public:
    virtual bool available();
    virtual void setup();
    virtual void preprocess() {}
    virtual void process();
    virtual void postprocess(bool sent) {}
    virtual void reinit();
    virtual std::string name() { return AmiiboName; }

    static void tap(uint8_t slot);
    static bool tagPresent();
    static const uint8_t * tagData();
    static const uint8_t * tagSignature();
    static bool slotFilled(uint8_t slot);
    static uint16_t slotSize(uint8_t slot);
    static const char * slotName(uint8_t slot);
    static bool slotRandomizeSerial(uint8_t slot);
    static bool setSlotRandomizeSerial(uint8_t slot, bool enabled);
    static bool writeSlot(uint8_t slot, const uint8_t * data, uint16_t length, const char * name);
    static bool clearSlot(uint8_t slot);
    static bool keyPresent(bool locked);
    static bool writeKey(bool locked, const uint8_t * data, size_t length);
    static bool clearKeys();
    static uint32_t tapGeneration();
private:
    Mask_t slotPinMasks[AMIIBO_SLOT_COUNT] = { };
    Mask_t pressedPins = 0;
};

#endif

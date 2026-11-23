#ifndef _NFCReader_H
#define _NFCReader_H

#include "gpaddon.h"
#include "addons/amiibo.h"

#include "hardware/i2c.h"

#define NFCReaderName      "NFCReader"
#define NFCReaderInputName "NFCReaderInput"

#ifndef NFC_READER_I2C_ADDRESS
#define NFC_READER_I2C_ADDRESS 0x24
#endif

#define NFC_UID_MAX_SIZE 10
#define NFC_TAG_COUNT    8
#define NFC_FRAME_MAX    64

struct NFCReaderStatus {
    bool enabled;
    bool detected;
    char firmware[8];
    bool tagPresent;
    uint8_t uid[NFC_UID_MAX_SIZE];
    uint8_t uidLength;
    bool amiibo;
    bool hasSignature;
    char i2cScan[64];
    char note[48];
};

// Drives a PN532 on an enabled I2C block. Runs on core1 and performs at most one
// short I2C transaction per process() call.
class NFCReaderAddon : public GPAddon {
public:
    virtual bool available();
    virtual void setup();
    virtual void preprocess() {}
    virtual void process();
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return NFCReaderName; }

    static NFCReaderStatus status();
    // Increments each time a tag is placed on the reader.
    static uint32_t tapCount();
    static uint8_t tapUid(uint8_t * uid);
private:
    enum class Phase : uint8_t {
        Find,
        GetFirmware,
        SamConfig,
        SetRetries,
        FieldOff,
        FieldOn,
        ListTarget,
        CheckPresent,
        ReadPages,
        ReadSignature,
    };
    enum class Io : uint8_t { Idle, WaitAck, WaitResponse };

    bool startCommand(const uint8_t * cmd, uint8_t length);
    int8_t pollResponse();
    void abortCommand();
    bool i2cWrite(const uint8_t * data, size_t length);
    bool i2cRead(uint8_t * data, size_t length);

    void issue();
    void handleResponse();
    void fail();
    void schedule(Phase next, uint32_t delayMs);
    void idle();
    void tagPlaced(const uint8_t * tagUid, uint8_t length, uint8_t sak);
    void tagMissing();
    void finishAmiiboRead(const uint8_t * signature);
    void scanBuses();

    i2c_inst_t * i2c = nullptr;
    uint8_t address = NFC_READER_I2C_ADDRESS;
    bool readAmiibo = false;

    Phase phase = Phase::Find;
    Io io = Io::Idle;
    uint8_t command = 0;
    uint32_t ioDeadlineMs = 0;
    uint32_t nextPollUs = 0;
    uint32_t nextActionMs = 0;
    uint32_t nextScanMs = 0;
    uint8_t errors = 0;

    uint8_t frame[NFC_FRAME_MAX];
    const uint8_t * response = nullptr;
    uint8_t responseLength = 0;

    bool tagLoaded = false;
    bool checkByRead = false;
    bool amiiboPending = false;
    uint8_t misses = 0;
    uint8_t uid[NFC_UID_MAX_SIZE] = {};
    uint8_t uidLength = 0;
    uint8_t page = 0;
    uint8_t dump[AMIIBO_DATA_SIZE];
};

// Runs the action saved for a tag when it is tapped. Runs on core0.
class NFCReaderInputAddon : public GPAddon {
public:
    virtual bool available();
    virtual void setup();
    virtual void preprocess();
    virtual void process() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return NFCReaderInputName; }
private:
    uint32_t lastTapCount = 0;
    int8_t activeTag = -1;
    uint32_t holdUntilMs = 0;
};

#endif

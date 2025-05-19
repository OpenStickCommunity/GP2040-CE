#ifndef _GAMECUBEDRIVER_H_
#define _GAMECUBEDRIVER_H_

#include "gpdriver.h"
#include "gamepad.h"

// GameCube report structure (ported from Haybox)
typedef struct {
    uint8_t a : 1;
    uint8_t b : 1;
    uint8_t x : 1;
    uint8_t y : 1;
    uint8_t start : 1;
    uint8_t padding : 3;
    uint8_t dpadLeft : 1;
    uint8_t dpadRight : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadUp : 1;
    uint8_t z : 1;
    uint8_t r : 1;
    uint8_t l : 1;
    uint8_t padding2 : 1;
    uint8_t xAxis;      // -128 to 127
    uint8_t yAxis;      // -128 to 127
    uint8_t cxAxis;     // -128 to 127
    uint8_t cyAxis;     // -128 to 127
    uint8_t left;       // 0-255 for analog trigger
    uint8_t right;      // 0-255 for analog trigger
} __attribute__((packed)) GameCubeReport;

class GameCubeDriver : public GPDriver {
public:
    virtual void initialize() override;
    virtual void initializeAux() override;
    virtual bool process(Gamepad * gamepad) override;
    virtual void processAux() override;
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) override;
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) override;
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) override;
    virtual const uint16_t* get_descriptor_string_cb(uint8_t index, uint16_t langid) override;
    virtual const uint8_t* get_descriptor_device_cb() override;
    virtual const uint8_t* get_hid_descriptor_report_cb(uint8_t itf) override;
    virtual const uint8_t* get_descriptor_configuration_cb(uint8_t index) override;
    virtual const uint8_t* get_descriptor_device_qualifier_cb() override;
    virtual uint16_t GetJoystickMidValue() override { return GAMEPAD_JOYSTICK_MID; }
    virtual USBListener* get_usb_auth_listener() override { return nullptr; }

private:
    void sendReport();
    void handlePoll();
    GameCubeReport report;
    uint data_pin;      // GameCube data pin
    PIO pio;           // PIO instance for GameCube protocol
    int sm;            // State machine index
    int offset;        // Program offset
};

#endif

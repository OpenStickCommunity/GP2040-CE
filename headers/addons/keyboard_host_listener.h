#ifndef _KeyboardHostListener_H
#define _KeyboardHostListener_H

#include "usblistener.h"
#include "gamepad.h"
#include "class/hid/hid.h"

struct KeyboardButtonMapping
{
    uint8_t key;
    uint16_t buttonMask;

    inline void setMask(uint16_t m) {
        buttonMask = m;
    }

    inline void setKey(uint8_t p)
    {
        key = (p > HID_KEY_NONE && p <= HID_KEY_GUI_RIGHT) ? p : 0xff;
    }

    bool isAssigned() const { return key != 0xff; }
};


class KeyboardHostListener : public USBListener {
public:// USB Listener Features
    virtual void setup();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
    virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {}
    virtual void unmount(uint8_t dev_addr);
    virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
    virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
    void process();
private:
    uint8_t getKeycodeFromModifier(uint8_t modifier);
    void preprocess_report();
    void process_kbd_report(uint8_t dev_addr, hid_keyboard_report_t const *report);
    void process_mouse_report(uint8_t dev_addr, hid_mouse_report_t const *report);
    KeyboardButtonMapping _keyboard_host_mapDpadUp;
    KeyboardButtonMapping _keyboard_host_mapDpadDown;
    KeyboardButtonMapping _keyboard_host_mapDpadLeft;
    KeyboardButtonMapping _keyboard_host_mapDpadRight;
    KeyboardButtonMapping _keyboard_host_mapButtonB1;
    KeyboardButtonMapping _keyboard_host_mapButtonB2;
    KeyboardButtonMapping _keyboard_host_mapButtonB3;
    KeyboardButtonMapping _keyboard_host_mapButtonB4;
    KeyboardButtonMapping _keyboard_host_mapButtonL1;
    KeyboardButtonMapping _keyboard_host_mapButtonR1;
    KeyboardButtonMapping _keyboard_host_mapButtonL2;
    KeyboardButtonMapping _keyboard_host_mapButtonR2;
    KeyboardButtonMapping _keyboard_host_mapButtonS1;
    KeyboardButtonMapping _keyboard_host_mapButtonS2;
    KeyboardButtonMapping _keyboard_host_mapButtonL3;
    KeyboardButtonMapping _keyboard_host_mapButtonR3;
    KeyboardButtonMapping _keyboard_host_mapButtonA1;
    KeyboardButtonMapping _keyboard_host_mapButtonA2;
    KeyboardButtonMapping _keyboard_host_mapButtonA3;
    KeyboardButtonMapping _keyboard_host_mapButtonA4;
    GamepadState _keyboard_host_state;
    bool _keyboard_host_mounted;
    uint8_t _keyboard_dev_addr;
    uint8_t _keyboard_instance;
    bool _mouse_host_mounted;
    uint8_t _mouse_dev_addr;
    uint8_t _mouse_instance;
    uint16_t mouseLeftMapping;
    uint16_t mouseMiddleMapping;
    uint16_t mouseRightMapping;
    int16_t mouseX;
    int16_t mouseY;
    int16_t mouseZ;
    bool mouseActive;
};

#endif  // _KeyboardHost_H_
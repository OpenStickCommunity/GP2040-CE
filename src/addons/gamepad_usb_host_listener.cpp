#include "addons/gamepad_usb_host_listener.h"
#include "storagemanager.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"

#include "hosts/DualsensePS5Host.h"
#include "hosts/DualshockPS4Host.h"
#include "hosts/GoogleStadiaHost.h"
#include "hosts/DrivingForceHost.h"
#include "hosts/UltraStik360Host.h"
#include "hosts/SwitchProHost.h"
#include "hosts/Xbox360Host.h"

/*
#include "hosts/XboxOneHost.h"
*/

void GamepadUSBHostListener::setup() {
    _controller_host = nullptr;
    _controller_dev_addr = 0;
    _controller_instance = 0;
    _next_update = 0;
}

void GamepadUSBHostListener::update() {
    if ( _controller_host == nullptr ) return;

    // Controller host updates (rumble, LEDs, etc. outside of gamepad input reports)
    if (getMillis() > _next_update) {
        _controller_host->update();
        _next_update = getMillis() + GAMEPAD_HOST_POLL_INTERVAL_MS;
    }
    
    // Controller host driver changes gamepad
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    _controller_host->gamepad(gamepad);    
}

void GamepadUSBHostListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    // Only host one controller at a time
    if ( _controller_host != nullptr ) return;

    uint16_t vid = 0, pid = 0;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    _controller_host = nullptr;
     if ( DualshockPS4Host::match(dev_addr, instance, vid, pid, desc_report, desc_len) ) {
        _controller_host = new DualshockPS4Host();
    }else if ( DualsensePS5Host::match(dev_addr, instance, vid, pid, desc_report, desc_len) ) {
        _controller_host = new DualsensePS5Host();
    } else if ( GoogleStadiaHost::match(dev_addr, instance, vid, pid, desc_report, desc_len) ) {
        _controller_host = new GoogleStadiaHost();
    } else if ( DrivingForceHost::match(dev_addr, instance, vid, pid, desc_report, desc_len) ) {
        _controller_host = new DrivingForceHost();
    } else if ( Ultrastik360Host::match(dev_addr, instance, vid, pid, desc_report, desc_len) ) {
        _controller_host = new Ultrastik360Host();
    } else if ( SwitchProHost::match(dev_addr, instance, vid, pid, desc_report, desc_len) ) {
        _controller_host = new SwitchProHost();
    } else {
        // Better way to determine if its a generic HID?

        //if ( GenericHIDHost::match(vid, pid) ) { // do we want this one?
        //controllerHost = new GenericHIDHost();
    }

    if ( _controller_host != nullptr ) {
        _controller_dev_addr = dev_addr;
        _controller_instance = instance;
        _controller_host->initialize(dev_addr, instance, vid, pid, desc_report, desc_len);
    } else { 
        // send message to the main screen that we have an unknown controller
        
        // if auto-detect is enabled, let's try to determine controller type by sending some special packets
        // Order: ps4
        // Order: ps5
        // Order: switch pro
        // Order: generic HID
    }
}

void GamepadUSBHostListener::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    // Only host one controller at a time
    if ( _controller_host != nullptr ) return;

    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    // Match is a static function, setup and init happens after we've established something
    _controller_host = nullptr;
    if ( Xbox360Host::match(dev_addr, instance, vid, pid, controllerType) ) {
        _controller_host = new Xbox360Host();
    }
    /* else if ( XboxOneHost::match(vid, pid && controllerType == 2 ) {
        _controller_host = new XboxOneHost();
    }*/

    if ( _controller_host != nullptr ) {
        _controller_dev_addr = dev_addr;
        _controller_instance = instance;
        _controller_host->initialize(dev_addr, instance, vid, pid, nullptr, 0);
    } else { 
        // send message to the main screen that we have an unknown controller

        // if auto-magic
        // check for xbox 360 markers
        // check for xbox one markers
    }
}

void GamepadUSBHostListener::unmount(uint8_t dev_addr) {
    if ( _controller_host == nullptr ) return;
    
    _controller_host->shutdown();
    delete _controller_host;
    _controller_host = nullptr;
}

void GamepadUSBHostListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if ( _controller_host != nullptr && 
        _controller_dev_addr == dev_addr &&
        _controller_instance == instance ) {
        _controller_host->process(report, len);
    } 
}

void GamepadUSBHostListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( _controller_host != nullptr &&
        _controller_dev_addr == dev_addr &&
        _controller_instance == instance ) {
        _controller_host->set_report_complete(dev_addr, instance, report_id, report_type, len);
    } 
}

void GamepadUSBHostListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( _controller_host != nullptr &&
        _controller_dev_addr == dev_addr &&
        _controller_instance == instance ) {
        _controller_host->get_report_complete(dev_addr, instance, report_id, report_type, len);
    } 
}

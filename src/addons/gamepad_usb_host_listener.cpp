#include "addons/gamepad_usb_host_listener.h"
#include "storagemanager.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"

//#include "hosts/DualsensePS5Host.h"
#include "hosts/DualshockPS4Host.h"

/*
#include "hosts/DrivingForceHost.h"
#include "hosts/GoogleStadiaHost.h"
#include "hosts/SwitchProHost.h"
#include "hosts/UltraStik360Host.h"
#include "hosts/Xbox360Host.h"
#include "hosts/XboxOneHost.h"
*/

void GamepadUSBHostListener::setup() {
    _controller_host_mounted = false;
    _controller_host = nullptr;
    _controller_dev_addr = 0;
    _controller_instance = 0;
    _next_update = 0;
}

void GamepadUSBHostListener::gamepadFeatureUpdate() {
    if ( _controller_host_mounted == false ) return;

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
    if ( _controller_host_mounted == true ) {
        return;
    }

    uint16_t vid = 0, pid = 0;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    // TODO: Make a list of static functions + pointers
    // Match is a static function, setup and init happens after we've established something
    _controller_host = nullptr;
     if ( DualshockPS4Host::match(vid, pid) ) {
        _controller_host = (GPHost*)(new DualshockPS4Host());
    }/* else if ( DualsensePS5Host::match(vid, pid) ) {
        _controller_host = new DualsensePS5Host();
    }else if ( GoogleStadiaHost::match(vid, pid) ) {
        _controller_host = new GoogleStadiaHost();
    } else if ( SwitchProHost::match(vid, pid) ) {
        _controller_host = new SwitchProHost();
    } else if ( DrivingForceHost::match(vid, pid) ) {
        _controller_host = new DrivingForceHost();
    } else if ( Ultrastik360Host::match(vid, pid) ) {
        _controller_host = new Ultrastik360Host();
    } else {
        // Better way to determine if its a generic HID?

        //if ( GenericHIDHost::match(vid, pid) ) { // do we want this one?
        //controllerHost = new GenericHIDHost();
    }*/

    if ( _controller_host != nullptr ) {
        _controller_host_mounted = true;
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
    if ( _controller_host_mounted == true ) {
        return;
    }

    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    // Match is a static function, setup and init happens after we've established something
    _controller_host = nullptr;
    /*if ( Xbox360Host::match(vid, pid) && controllerType == 1 ) {
        _controller_host = new Xbox360Host();
    } else if ( XboxOneHost::match(vid, pid && controllerType == 2 ) {
        _controller_host = new XboxOneHost();
    }*/

    if ( _controller_host != nullptr ) {
        _controller_host_mounted = true;
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
    if ( _controller_host_mounted == true ) {
        _controller_host->shutdown();
        delete _controller_host;
        _controller_host = nullptr;
        _controller_host_mounted = false;
    }
}

void GamepadUSBHostListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    // if a hid device hasn't been mounted
    if ( _controller_host_mounted == false ) return;

    if ( _controller_dev_addr == dev_addr &&
        _controller_instance == instance ) {
        _controller_host->process(report, len);
    } 
}

/*
void GamepadUSBHostListener::process_ctrlr_report(uint8_t dev_addr, uint8_t const* report, uint16_t len) {
#if GAMEPAD_HOST_DEBUG
    //printf("\033[1;0H\nHost (%d):\n", len);
    //for (uint8_t i = 0; i < len; i++) {
    //    printf("%02x ", report[i]);
    //    if (((i+1) % 16) == 0) printf("\n");
    //}
    //printf("----\n");
#endif

    switch(controller_pid)
    {
        case DS4_ORG_PRODUCT_ID:   // Sony Dualshock 4 controller
        case DS4_PRODUCT_ID:       // Sony Dualshock 4 controller
        case PS4_PRODUCT_ID:       // Razer Panthera
        case PS4_WHEEL_PRODUCT_ID: // G29
        case 0xB67B:               // T-Flight
        case 0x00EE:               // Hori Minipad
            if (isDS4Identified) {
                process_ds4(report, len);
            }
            break;
        case 0x0CE6:               // DualSense
            process_ds(report, len);
            break;
        case SWITCH_PRO_PRODUCT_ID: // Switch Pro controller
            process_switch_pro(report, len);
            break;
        case 0x9400:               // Google Stadia controller
            process_stadia(report, len);
            break;

        case 0xC294:               // Driving Force
            if (!isDFInit) setup_df_wheel();
            break;

        case 0xC29A:
            process_dfgt(report, len);
            break;

        case 0x0510:               // pre-2015 Ultrakstik 360
        case 0x0511:               // Ultrakstik 360
            process_ultrastik360(report, len);
            break;
        default:
            break;
    }
}

// this is primarily for xinput updates as the controller refuses the send unnecessary
// data, so update wouldn't be called otherwise. but works for every controller and
// provides a more consistent way of quick rumble updates
void GamepadUSBHostListener::update_ctrlr() {
    switch (_controller_type) {
        case xinput_type_t::XBOX360:
            update_xinput(_controller_dev_addr, _controller_instance);
            break;
        case xinput_type_t::UNKNOWN:
            switch(controller_pid)
            {
                case DS4_ORG_PRODUCT_ID:   // Sony Dualshock 4 controller
                case DS4_PRODUCT_ID:       // Sony Dualshock 4 controller
                case PS4_PRODUCT_ID:       // Razer Panthera
                case PS4_WHEEL_PRODUCT_ID: // G29
                case 0xB67B:               // T-Flight
                case 0x00EE:               // Hori Minipad
                    if (isDS4Identified) {
                        update_ds4();
                    }
                    break;
                case SWITCH_PRO_PRODUCT_ID: // Switch Pro controller
                    update_switch_pro();
                    break;
                default:
                    break;
            }
        default:
            break;
    }
}


bool GamepadUSBHostListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    return tuh_hid_get_report(_controller_dev_addr, _controller_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

bool GamepadUSBHostListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    return tuh_hid_set_report(_controller_dev_addr, _controller_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}
*/
void GamepadUSBHostListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
}

void GamepadUSBHostListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
}

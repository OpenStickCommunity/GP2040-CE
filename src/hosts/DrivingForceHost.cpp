#include "hosts/DrivingForceHost.h"


/*
    // Logitech G29 Driving Force
    if ( vendor_id == ) {
        switch(product_id) {
            case 0xC260:
                return true;
        }
    }

    // Thrustmaster T.Flight Racing Wheel
    if ( vendor_id == 0x44F) {
        switch(product_id) {
            case 0xB67B:
                return true;
        }
    }
*/

void DrivingForceHost::intialize() {
    // send commands to see if can be reset to Driving Force GT mode for more compatibility
    uint8_t command[8] = {0xF8, 0x09, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00};
    uint16_t commandSize = sizeof(command);

    if (tuh_hid_send_report(_controller_dev_addr, _controller_instance, 0, command, commandSize)) {
        isDFInit = true;
    }
}

void DrivingForceHost::process(uint8_t const* report, uint16_t len) {
    PS3ReportAlt ps3Report;
    memcpy(&ps3Report, report, len);
#if GAMEPAD_HOST_DEBUG
    //printf("\033[2;0H");
    //for (uint8_t i = 0; i < len; i++) {
    //    printf("%02x ", report[i]);
    //    if (((i+1) % 16) == 0) printf("\n");
    //}
    //printf("\033[2;0H Ste:%5d Gas:%3d Brk:%3d", ps3Report.wheel.steeringWheel, ps3Report.wheel.gasPedal, ps3Report.wheel.brakePedal);
    //printf("\n-----\n");
    //printf("\033[2;0HDPad: %1d", ps3Report.wheel.dpadDirection);
    //printf("\033[3;0HWheel: %5d", ps3Report.wheel.steeringWheel);
    //printf("\033[4;0HGas: %3d", ps3Report.wheel.gasPedal);
    //printf("\033[5;0HBrake: %3d", ps3Report.wheel.brakePedal);
#endif
}
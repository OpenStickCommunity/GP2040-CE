#include "hosts/Ultrastik360Host.h"

void Ultrastik360Host::process(uint8_t const* report, uint16_t len) {

    ultrastik360_t controller_report;

    memcpy(&controller_report, report, sizeof(controller_report));

    _controller_host_state.lx = map(controller_report.GD_GamePadPointerX, 0, 255, GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ly = map(controller_report.GD_GamePadPointerY, 0, 255, GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_analog = true;

    if (controller_report.BTN_GamePadButton1 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
    if (controller_report.BTN_GamePadButton2 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
    if (controller_report.BTN_GamePadButton3 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
    if (controller_report.BTN_GamePadButton4 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
    if (controller_report.BTN_GamePadButton5 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
    if (controller_report.BTN_GamePadButton6 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L2;
    if (controller_report.BTN_GamePadButton7 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
    if (controller_report.BTN_GamePadButton8 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
}

#include "hosts/GoogleStadiaHost.h"

void GoogleStadioHost::process(uint8_t const *report, uint16_t len) {
    google_stadia_report_t controller_report;

    memcpy(&controller_report, report, sizeof(controller_report));

    _controller_host_state.lx = map(controller_report.GD_GamePadPointerX ,1,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ly = map(controller_report.GD_GamePadPointerY,1 ,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.rx = map(controller_report.GD_GamePadPointerZ,1 ,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ry = map(controller_report.GD_GamePadPointerRz,1 ,255,GAMEPAD_JOYSTICK_MIN,GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.lt = controller_report.SIM_GamePadBrake;
    _controller_host_state.rt = controller_report.SIM_GamePadAccelerator;
    _controller_host_analog = true;

    if (controller_report.BTN_GamePadButton18 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_A2;
    if (controller_report.BTN_GamePadButton17 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_A3;
    if (controller_report.BTN_GamePadButton11 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
    if (controller_report.BTN_GamePadButton15 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
    if (controller_report.BTN_GamePadButton14 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
    if (controller_report.BTN_GamePadButton13 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
    if (controller_report.BTN_GamePadButton12 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
    if (controller_report.BTN_GamePadButton8 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
    if (controller_report.BTN_GamePadButton7 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
    if (controller_report.BTN_GamePadButton5 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
    if (controller_report.BTN_GamePadButton4 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
    if (controller_report.BTN_GamePadButton2 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
    if (controller_report.BTN_GamePadButton1 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
    if (controller_report.BTN_GamePadButton19 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
    if (controller_report.BTN_GamePadButton20 == 1) _controller_host_state.buttons |= GAMEPAD_MASK_L2;

    if (controller_report.GD_GamePadHatSwitch == 0) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
    if (controller_report.GD_GamePadHatSwitch == 1) _controller_host_state.dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT;
    if (controller_report.GD_GamePadHatSwitch == 2) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;
    if (controller_report.GD_GamePadHatSwitch == 3) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT | GAMEPAD_MASK_DOWN;
    if (controller_report.GD_GamePadHatSwitch == 4) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
    if (controller_report.GD_GamePadHatSwitch == 5) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT;
    if (controller_report.GD_GamePadHatSwitch == 6) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
    if (controller_report.GD_GamePadHatSwitch == 7) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT | GAMEPAD_MASK_UP;
}

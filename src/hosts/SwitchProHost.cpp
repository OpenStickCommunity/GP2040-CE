#include "hosts/SwitchProHost.h"
#include "tusb_config.h"
#include "tusb.h"
#include "class/hid/hid.h"
#include "device/usbd_pvt.h"
#include "storagemanager.h"

// Match
bool SwitchProHost::match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    // Nintendo
    if ( vendor_id == 0x057E) {
        switch(product_id) {
            case 0x2009:
                return true;
        }
    }
    
    return false;
}

// init phase
// https://github.com/wiredopposite/OGX-Mini/blob/ccccf6651b54bcca3c94b5d4e73e8f2796fc8c05/Firmware/RP2040/src/USBHost/HostDriver/SwitchPro/SwitchPro.cpp#L23
void SwitchProHost::initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) {
    // Setup our vars
    _dev_addr = dev_addr;
    _instance = instance;
    _vendor_id = vendor_id;
    _product_id = product_id;

    _controller_host_state.buttons = 0;
    _controller_host_state.dpad = 0;
    _controller_host_state.lx = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.ly = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.rx = GAMEPAD_JOYSTICK_MID;
    _controller_host_state.ry = GAMEPAD_JOYSTICK_MID;
    
    switchProFinished = false;
    switchReportCounter = 0;
    switchProState = SwitchOutputSubtypes::IDENTIFY;
    lastSwitchLed = 0;
    memset(&prevReport, 0, sizeof(SwitchProReport));
}


void SwitchProHost::update()
{
    if (!switchProFinished) return;

    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

    SwitchProHostReport out_report{
        .command = SwitchCommands::SPI_READ,
        .counter = switchReportCounter++,
        .rumble_l = {0x00, 0x01, 0x40, 0x40}, // default rumble states
        .rumble_r = {0x00, 0x01, 0x40, 0x40},
    };
    uint8_t report_size = 10; // no subcommand

    // rumble
    gamepad->auxState.haptics.leftActuator.enabled = 1;
    gamepad->auxState.haptics.rightActuator.enabled = 1;
    if (gamepad->auxState.haptics.leftActuator.active
        && gamepad->auxState.haptics.leftActuator.intensity > 0) {
        // personally I felt this a bit too strong on PS4 games, might need to be adjusted for personal preference
        uint8_t amplitude_l = static_cast<uint8_t>(((gamepad->auxState.haptics.leftActuator.intensity / 255.0f) * 0.8f + 0.5f) * (0xC0 - 0x40) + 0x40);
#ifdef GAMEPAD_HOST_DEBUG
        printf("Switch Pro Left Rumble Intensity: %d -> %d\n", gamepad->auxState.haptics.leftActuator.intensity, amplitude_l);
#endif

        out_report.rumble_l[0] = amplitude_l;
        out_report.rumble_l[1] = 0x88;
        out_report.rumble_l[2] = amplitude_l / 2;
        out_report.rumble_l[3] = 0x61;
    }
    if (gamepad->auxState.haptics.rightActuator.active
        && gamepad->auxState.haptics.leftActuator.intensity > 0) {
        uint8_t amplitude_r = static_cast<uint8_t>(((gamepad->auxState.haptics.rightActuator.intensity / 255.0f) * 0.8f + 0.5f) * (0xC0 - 0x40) + 0x40);
#ifdef GAMEPAD_HOST_DEBUG
        printf("Switch Pro Right Rumble Intensity: %d -> %d\n", gamepad->auxState.haptics.rightActuator.intensity, amplitude_r);
#endif
        out_report.rumble_r[0] = amplitude_r;
        out_report.rumble_r[1] = 0x88;
        out_report.rumble_r[2] = amplitude_r / 2;
        out_report.rumble_r[3] = 0x61;
    }

    // Player light indicator
    if (gamepad->auxState.playerID.active) {
        Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
        uint8_t shifted = (1u << (gamepad->auxState.playerID.value+1u)) - 1u;
        if (gamepad->auxState.playerID.value < 1 && gamepad->auxState.playerID.value > 4) {
            shifted = 1;
        }
        if (shifted != lastSwitchLed) {
            SwitchProHostReport led_out_report{
                .command = SwitchOutputSubtypes::IDENTIFY,
                .counter = switchReportCounter++,
                .rumble_l = {0x00, 0x01, 0x40, 0x40},
                .rumble_r = {0x00, 0x01, 0x40, 0x40},
                .subcommand = SwitchCommands::SET_PLAYER_LIGHTS,
                .subcommand_args = {shifted, 0x00, 0x00},
            };
            uint8_t report_size = 12; // 10 + 2 for subcommand

            tuh_hid_send_report(_dev_addr, _instance, 0, &led_out_report, report_size);
        }
        lastSwitchLed = shifted;
    }

    tuh_hid_send_report(_dev_addr, _instance, 0, &out_report, report_size);
}

void SwitchProHost::process(uint8_t const* report, uint16_t len) {
    if (len == 0)
        return;

    if (!switchProFinished) {
#ifdef GAMEPAD_HOST_DEBUG
        printf("received report id %x during initialization\n", report[0]);
#endif
        setup_switch_pro(report, len);
        return;
    }

    SwitchProReport controller_report;

    if (len < sizeof(SwitchProReport)) {
#ifdef GAMEPAD_HOST_DEBUG
        printf("ignoring report with len %d (%x)...\n", len, report[0]);
#endif
        return;
    }
    if (report[0] != 0x30) {
#ifdef GAMEPAD_HOST_DEBUG
        printf("ignoring report with id %d (%d len)...\n", report[0], len);
#endif
        return;
    }
    memcpy(&controller_report, report, sizeof(controller_report));

    if (memcmp(&prevReport, &controller_report, sizeof(SwitchProReport)) == 0)
        return;

    _controller_host_state.dpad = 0;
    if (controller_report.inputs.dpadUp) _controller_host_state.dpad |= GAMEPAD_MASK_UP;
    if (controller_report.inputs.dpadDown) _controller_host_state.dpad |= GAMEPAD_MASK_DOWN;
    if (controller_report.inputs.dpadLeft) _controller_host_state.dpad |= GAMEPAD_MASK_LEFT;
    if (controller_report.inputs.dpadRight) _controller_host_state.dpad |= GAMEPAD_MASK_RIGHT;

    _controller_host_state.buttons = 0;
    if (controller_report.inputs.buttonY) _controller_host_state.buttons |= GAMEPAD_MASK_B3;
    if (controller_report.inputs.buttonX) _controller_host_state.buttons |= GAMEPAD_MASK_B4;
    if (controller_report.inputs.buttonB) _controller_host_state.buttons |= GAMEPAD_MASK_B1;
    if (controller_report.inputs.buttonA) _controller_host_state.buttons |= GAMEPAD_MASK_B2;
    if (controller_report.inputs.buttonR) _controller_host_state.buttons |= GAMEPAD_MASK_R1;
    if (controller_report.inputs.buttonZR) _controller_host_state.buttons |= GAMEPAD_MASK_R2;
    if (controller_report.inputs.buttonMinus) _controller_host_state.buttons |= GAMEPAD_MASK_S1;
    if (controller_report.inputs.buttonPlus) _controller_host_state.buttons |= GAMEPAD_MASK_S2;
    if (controller_report.inputs.buttonThumbR) _controller_host_state.buttons |= GAMEPAD_MASK_R3;
    if (controller_report.inputs.buttonThumbL) _controller_host_state.buttons |= GAMEPAD_MASK_L3;
    if (controller_report.inputs.buttonHome) _controller_host_state.buttons |= GAMEPAD_MASK_A1;
    if (controller_report.inputs.buttonCapture) _controller_host_state.buttons |= GAMEPAD_MASK_A2;
    if (controller_report.inputs.buttonL) _controller_host_state.buttons |= GAMEPAD_MASK_L1;
    if (controller_report.inputs.buttonZL) _controller_host_state.buttons |= GAMEPAD_MASK_L2;

    uint16_t lx12 = controller_report.inputs.leftStick.getX() & 0x0FFFu;
    uint16_t ly12 = (0x1000u - (controller_report.inputs.leftStick.getY() & 0x0FFFu)) & 0x0FFFu;
    uint16_t rx12 = controller_report.inputs.rightStick.getX() & 0x0FFFu;
    uint16_t ry12 = (0x1000u - (controller_report.inputs.rightStick.getY() & 0x0FFFu)) & 0x0FFFu;

    _controller_host_state.lx = map(lx12, 0, 4095, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ly = map(ly12, 0, 4095, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.rx = map(rx12, 0, 4095, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);
    _controller_host_state.ry = map(ry12, 0, 4095, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX);

    memcpy(&prevReport, &controller_report, sizeof(SwitchProReport));
}


// init phase
// https://github.com/wiredopposite/OGX-Mini/blob/ccccf6651b54bcca3c94b5d4e73e8f2796fc8c05/Firmware/RP2040/src/USBHost/HostDriver/SwitchPro/SwitchPro.cpp#L23
void SwitchProHost::setup_switch_pro(uint8_t const *report, uint16_t len) {
    SwitchProHostReport out_report{
        .rumble_l = {0x00, 0x01, 0x40, 0x40}, // default rumble states
        .rumble_r = {0x00, 0x01, 0x40, 0x40},
    };
    uint8_t report_size = 10; // no subcommand

#ifdef GAMEPAD_HOST_DEBUG
    printf("Switch Pro init state: %d\n.", switchProState);
#endif

    switch (switchProState)
    {
    case SwitchOutputSubtypes::IDENTIFY: {
        if (len < 10
            || report[0] != SwitchReportID::REPORT_USB_INPUT_81
            || report[1] != SwitchOutputSubtypes::IDENTIFY) {
            tuh_hid_send_report(_dev_addr, _instance, 0, &SWITCH_INIT_REPORT, sizeof(SWITCH_INIT_REPORT));
            break;
        }

        out_report.command = SwitchReportID::REPORT_CONFIGURATION;
        out_report.counter = SwitchOutputSubtypes::HANDSHAKE;
        switchReportCounter++; // skip counter
        tuh_hid_send_report(_dev_addr, _instance, 0, &out_report, report_size);
        // might turn up the home LED as well
        switchProState = SwitchOutputSubtypes::DISABLE_USB_TIMEOUT;
        break;
    }
    case SwitchOutputSubtypes::DISABLE_USB_TIMEOUT: {
        if (len < 2 ||
            (report[0] != SwitchReportID::REPORT_USB_INPUT_81
                && report[0] != SwitchReportID::REPORT_OUTPUT_30)) {
            // reset
            tuh_hid_send_report(_dev_addr, _instance, 0, &SWITCH_INIT_REPORT, sizeof(SWITCH_INIT_REPORT));
            switchProState = SwitchOutputSubtypes::IDENTIFY;
            break;
        }
        out_report.command = SwitchReportID::REPORT_CONFIGURATION;
        out_report.counter = SwitchOutputSubtypes::DISABLE_USB_TIMEOUT;
        switchReportCounter++; // skip counter
        tuh_hid_send_report(_dev_addr, _instance, 0, &out_report, report_size);
        switchProState = SwitchOutputSubtypes::IDENTIFY; // done
        switchProFinished = true;
#ifdef GAMEPAD_HOST_DEBUG
        printf("Switch Pro controller initialized\n.");
#endif
        Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
        gamepad->auxState.playerID.enabled = true;
        break;
    }

    default:
        switchProState = SwitchOutputSubtypes::IDENTIFY; // reset
        switchProFinished = false;
        break;
    }
}

void SwitchProHost::gamepad(Gamepad * gamepad) {
    // Override the Gamepad
    gamepad->hasLeftAnalogStick  = true;
    gamepad->hasRightAnalogStick = true;
    gamepad->state.dpad     |= _controller_host_state.dpad;
    gamepad->state.buttons  |= _controller_host_state.buttons;
    gamepad->state.lx       = _controller_host_state.lx;
    gamepad->state.ly       = _controller_host_state.ly;
    gamepad->state.rx       = _controller_host_state.rx;
    gamepad->state.ry       = _controller_host_state.ry;
}

uint32_t SwitchProHost::map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


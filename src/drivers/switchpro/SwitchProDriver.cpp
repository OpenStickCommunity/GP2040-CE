#include "drivers/switchpro/SwitchProDriver.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"
#include "pico/rand.h"

// force a report to be sent every X ms
#define SWITCH_PRO_KEEPALIVE_TIMER 5

static void setSwitchProRumble(const uint8_t *rumbleData) {
    // rumbleData points to 8 bytes of HD rumble data:
    // bytes 0..3: Left motor
    // bytes 4..7: Right motor
    auto decode_motor = [](const uint8_t *d) -> uint8_t {
        uint8_t hf_amp = d[1] & 0xFE;
        uint8_t lf_amp = (d[3] >= 0x40) ? (d[3] - 0x40) : 0;
        uint16_t scaled_hf = (hf_amp * 255) / 200;
        uint16_t scaled_lf = (lf_amp * 255) / 50;
        uint16_t max_amp = (scaled_hf > scaled_lf) ? scaled_hf : scaled_lf;
        return (max_amp > 255) ? 255 : (uint8_t)max_amp;
    };

    uint8_t left = decode_motor(rumbleData);
    uint8_t right = decode_motor(rumbleData + 4);

    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
    if (gamepad->auxState.haptics.leftActuator.enabled) {
        gamepad->auxState.haptics.leftActuator.active = (left > 0);
        gamepad->auxState.haptics.leftActuator.intensity = left;
    }
    if (gamepad->auxState.haptics.rightActuator.enabled) {
        gamepad->auxState.haptics.rightActuator.active = (right > 0);
        gamepad->auxState.haptics.rightActuator.intensity = right;
    }
}

void SwitchProDriver::initialize() {
    playerID = 0;
    last_report_counter = 0;
    handshakeCounter = 0;
    isReady = false;

    deviceInfo = {
        .majorVersion = 0x04,
        .minorVersion = 0x91,
        .controllerType = SwitchControllerType::SWITCH_TYPE_PRO_CONTROLLER,
        .unknown00 = 0x02,
        // MAC address in reverse
        .macAddress = {0x7c, 0xbb, 0x8a, (uint8_t)(get_rand_32() % 0xff), (uint8_t)(get_rand_32() % 0xff), (uint8_t)(get_rand_32() % 0xff)},
        .unknown01 = 0x01,
        .storedColors = 0x02,
    };

	switchReport = {
        .reportID = 0x30,
        .timestamp = 0,

        .inputs {
            .connectionInfo = 0x01,
            .batteryLevel = 0x08,

            // byte 00
            .buttonY = 0,
            .buttonX = 0,
            .buttonB = 0,
            .buttonA = 0,
            .buttonRightSR = 0,
            .buttonRightSL = 0,
            .buttonR = 0,
            .buttonZR = 0,

            // byte 01
            .buttonMinus = 0,
            .buttonPlus = 0,
            .buttonThumbR = 0,
            .buttonThumbL = 0,
            .buttonHome = 0,
            .buttonCapture = 0,
            .dummy = 0,
            .chargingGrip = 0,

            // byte 02
            .dpadDown = 0,
            .dpadUp = 0,
            .dpadRight = 0,
            .dpadLeft = 0,
            .buttonLeftSL = 0,
            .buttonLeftSR = 0,
            .buttonL = 0,
            .buttonZL = 0,
            .leftStick = {0xFF, 0xF7, 0x7F},
            .rightStick = {0xFF, 0xF7, 0x7F},
        },
        .rumbleReport = 0x80,
        .imuData = {0x00},
        .padding = {0x00}
    };

    last_report_timer = to_ms_since_boot(get_absolute_time());

    factoryConfig->leftStickCalibration.getRealMin(leftMinX, leftMinY);
    factoryConfig->leftStickCalibration.getCenter(leftCenX, leftCenY);
    factoryConfig->leftStickCalibration.getRealMax(leftMaxX, leftMaxY);
    factoryConfig->rightStickCalibration.getRealMin(rightMinX, rightMinY);
    factoryConfig->rightStickCalibration.getCenter(rightCenX, rightCenY);
    factoryConfig->rightStickCalibration.getRealMax(rightMaxX, rightMaxY);

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "SWITCHPRO",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hidd_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

bool SwitchProDriver::process(Gamepad * gamepad) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    reportSent = false;

    switchReport.inputs.dpadUp =    ((gamepad->state.dpad & GAMEPAD_MASK_UP) == GAMEPAD_MASK_UP);
    switchReport.inputs.dpadDown =  ((gamepad->state.dpad & GAMEPAD_MASK_DOWN) == GAMEPAD_MASK_DOWN);
    switchReport.inputs.dpadLeft =  ((gamepad->state.dpad & GAMEPAD_MASK_LEFT) == GAMEPAD_MASK_LEFT);
    switchReport.inputs.dpadRight = ((gamepad->state.dpad & GAMEPAD_MASK_RIGHT) == GAMEPAD_MASK_RIGHT);

    switchReport.inputs.chargingGrip = 1;

    switchReport.inputs.buttonY = gamepad->pressedB3();
    switchReport.inputs.buttonX = gamepad->pressedB4();
    switchReport.inputs.buttonB = gamepad->pressedB1();
    switchReport.inputs.buttonA = gamepad->pressedB2();
    switchReport.inputs.buttonRightSR = 0;
    switchReport.inputs.buttonRightSL = 0;
    switchReport.inputs.buttonR = gamepad->pressedR1();
    switchReport.inputs.buttonZR = gamepad->pressedR2();
    if (gamepad->hasAnalogTriggers || gamepad->hasRightAnalogStick)
        switchReport.inputs.buttonZR |= gamepad->state.rt > 0;
    switchReport.inputs.buttonMinus = gamepad->pressedS1();
    switchReport.inputs.buttonPlus = gamepad->pressedS2();
    switchReport.inputs.buttonThumbR = gamepad->pressedR3();
    switchReport.inputs.buttonThumbL = gamepad->pressedL3();
    switchReport.inputs.buttonHome = gamepad->pressedA1();
    switchReport.inputs.buttonCapture = gamepad->pressedA2();
    switchReport.inputs.buttonLeftSR = 0;
    switchReport.inputs.buttonLeftSL = 0;
    switchReport.inputs.buttonL = gamepad->pressedL1();
    switchReport.inputs.buttonZL = gamepad->pressedL2();
    if (gamepad->hasAnalogTriggers || gamepad->hasLeftAnalogStick)
        switchReport.inputs.buttonZL |= gamepad->state.lt > 0;

    // analog
    uint16_t scaleLeftStickX = scale16To12(gamepad->state.lx);
    uint16_t scaleLeftStickY = scale16To12(gamepad->state.ly);
    uint16_t scaleRightStickX = scale16To12(gamepad->state.rx);
    uint16_t scaleRightStickY = scale16To12(gamepad->state.ry);
    
    switchReport.inputs.leftStick.setX(std::min(std::max(scaleLeftStickX,leftMinX), leftMaxX));
    switchReport.inputs.leftStick.setY(4095 - std::min(std::max(scaleLeftStickY,leftMinY), leftMaxY));
    switchReport.inputs.rightStick.setX(std::min(std::max(scaleRightStickX,rightMinX), rightMaxX));
    switchReport.inputs.rightStick.setY(4095 - std::min(std::max(scaleRightStickY,rightMinY), rightMaxY));

    switchReport.rumbleReport = 0x80;
    //switchReport.reportID = inputMode;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

    if (isReportQueued) {
        if ((now - last_report_timer) > SWITCH_PRO_KEEPALIVE_TIMER) {
            if (tud_hid_ready() && sendReport(0, report, 64) == true ) {
            }
            isReportQueued = false;
            last_report_timer = now;
        }
        reportSent = true;
    }

    Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
    processedGamepad->auxState.playerID.active = true;
    processedGamepad->auxState.playerID.ledValue = playerID;
    processedGamepad->auxState.playerID.value = playerID;

    if (isReady && !reportSent) {
        if ((now - last_report_timer) > SWITCH_PRO_KEEPALIVE_TIMER) {
            switchReport.timestamp = last_report_counter;
            void * inputReport = &switchReport;
            uint16_t report_size = sizeof(switchReport);
            if (memcmp(last_report, inputReport, report_size) != 0) {
                // HID ready + report sent, copy previous report
                if (tud_hid_ready() && sendReport(0, inputReport, report_size) == true ) {
                    memcpy(last_report, inputReport, report_size);
                    reportSent = true;
                }

                last_report_timer = now;
            }
        }
    } else {
        if (!isInitialized) {
            // send identification
            sendIdentify();
            if (tud_hid_ready() && tud_hid_report(0, report, 64) == true) {
                isInitialized = true;
                reportSent = true;
            }

            last_report_timer = now;
        }
    }

    return reportSent;
}

// tud_hid_get_report_cb
uint16_t SwitchProDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    if (report_type == HID_REPORT_TYPE_INPUT) {
        uint16_t len = (reqlen < sizeof(switchReport)) ? reqlen : sizeof(switchReport);
        memcpy(buffer, &switchReport, len);
        return len;
    }
    return 0;
}

void SwitchProDriver::sendIdentify() {
    memset(report, 0x00, 64);
    report[0] = SwitchReportID::REPORT_USB_INPUT_81;
    report[1] = SwitchOutputSubtypes::IDENTIFY;
    report[2] = 0x00;
    report[3] = deviceInfo.controllerType;
    // MAC address
    for (uint8_t i = 0; i < 6; i++) {
        report[4+i] = deviceInfo.macAddress[5-i];
    }
}

void SwitchProDriver::sendSubCommand(uint8_t subCommand) {

}

bool SwitchProDriver::sendReport(uint8_t reportID, void const* reportData, uint16_t reportLength) {
    bool result = tud_hid_report(reportID, reportData, reportLength);
    if (last_report_counter < 255) {
        last_report_counter++;
    } else {
        last_report_counter = 0;
    }
    return result;
}

void SwitchProDriver::handleConfigReport(const uint8_t *data, uint16_t dataLen) {
    uint8_t switchReportSubID = data[0];
    bool canSend = false;

    switch (switchReportSubID) {
        case SwitchOutputSubtypes::IDENTIFY:
            sendIdentify();
            canSend = true;
            break;
        case SwitchOutputSubtypes::HANDSHAKE:
            report[0] = SwitchReportID::REPORT_USB_INPUT_81;
            report[1] = SwitchOutputSubtypes::HANDSHAKE;
            canSend = true;
            break;
        case SwitchOutputSubtypes::BAUD_RATE:
            report[0] = SwitchReportID::REPORT_USB_INPUT_81;
            report[1] = SwitchOutputSubtypes::BAUD_RATE;
            canSend = true;
            break;
        case SwitchOutputSubtypes::DISABLE_USB_TIMEOUT:
            isReady = true;
            canSend = false;
            break;
        case SwitchOutputSubtypes::ENABLE_USB_TIMEOUT:
            canSend = false;
            break;
        default:
            report[0] = SwitchReportID::REPORT_OUTPUT_30;
            report[1] = switchReportSubID;
            canSend = true;
            break;
    }

    if (canSend) isReportQueued = true;
}

void SwitchProDriver::handleFeatureReport(const uint8_t *data, uint16_t dataLen) {
    uint8_t commandID = data[9];
    uint32_t spiReadAddress = 0;
    uint8_t spiReadSize = 0;
    bool canSend = false;

    report[0] = SwitchReportID::REPORT_OUTPUT_21;
    report[1] = last_report_counter;
    memcpy(report + 2, &switchReport.inputs, sizeof(SwitchInputReport));
    report[12] = 0x80;

    switch (commandID) {
        case SwitchCommands::GET_CONTROLLER_STATE:
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::BLUETOOTH_PAIR_REQUEST:
            report[13] = 0x81;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::REQUEST_DEVICE_INFO:
            report[13] = 0x82;
            report[14] = 0x02;
            memcpy(&report[15], &deviceInfo, sizeof(deviceInfo));
            canSend = true;
            break;
        case SwitchCommands::SET_MODE:
            inputMode = data[10];
            report[13] = 0x80;
            report[14] = 0x03;
            report[15] = inputMode;
            canSend = true;
            break;
        case SwitchCommands::TRIGGER_BUTTONS:
            report[13] = 0x83;
            report[14] = 0x04;
            canSend = true;
            break;
        case SwitchCommands::SET_SHIPMENT:
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::SPI_READ:
            spiReadAddress = (data[13] << 24) | (data[12] << 16) | (data[11] << 8) | (data[10]);
            spiReadSize = data[14];
            report[13] = 0x90;
            report[14] = data[9];
            report[15] = data[10];
            report[16] = data[11];
            report[17] = data[12];
            report[18] = data[13];
            report[19] = data[14];
            readSPIFlash(&report[20], spiReadAddress, spiReadSize);
            canSend = true;
            break;
        case SwitchCommands::SET_NFC_IR_CONFIG:
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::SET_NFC_IR_STATE:
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::SET_PLAYER_LIGHTS:
            playerID = data[10];
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::GET_PLAYER_LIGHTS:
            playerID = data[10];
            report[13] = 0xB0;
            report[14] = commandID;
            report[15] = playerID;
            canSend = true;
            break;
        case SwitchCommands::COMMAND_UNKNOWN_33:
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::SET_HOME_LIGHT:
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            break;
        case SwitchCommands::TOGGLE_IMU:
            isIMUEnabled = data[10];
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            break;
        case SwitchCommands::IMU_SENSITIVITY:
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::ENABLE_VIBRATION:
            isVibrationEnabled = data[10];
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            break;
        case SwitchCommands::READ_IMU:
            report[13] = 0xC0;
            report[14] = commandID;
            report[15] = data[10];
            report[16] = data[11];
            canSend = true;
            break;
        case SwitchCommands::GET_VOLTAGE:
            report[13] = 0xD0;
            report[14] = 0x50;
            report[15] = 0x83;
            report[16] = 0x06;
            canSend = true;
            break;
        default:
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
    }

    if (canSend) isReportQueued = true;
}

void SwitchProDriver::set_report(uint8_t report_id, hid_report_type_t report_type, const uint8_t *buffer, uint16_t bufsize) {
    if (report_type != HID_REPORT_TYPE_OUTPUT && report_type != HID_REPORT_TYPE_FEATURE) return;

    uint8_t switchReportID;
    const uint8_t *data;
    uint16_t dataLen;

    if (report_id != 0) {
        switchReportID = report_id;
        data = buffer;
        dataLen = bufsize;
    } else {
        if (bufsize == 0) return;
        switchReportID = buffer[0];
        data = buffer + 1;
        dataLen = bufsize - 1;
    }

    if (switchReportID == SwitchReportID::REPORT_OUTPUT_10) {
        if (dataLen >= 9) {
            setSwitchProRumble(data + 1);
        }
    } else if (switchReportID == SwitchReportID::REPORT_FEATURE) {
        if (dataLen >= 9) {
            setSwitchProRumble(data + 1);
        }
        if (dataLen >= 10) {
            memset(report, 0x00, sizeof(report));
            handleFeatureReport(data, dataLen);
        }
    } else if (switchReportID == SwitchReportID::REPORT_CONFIGURATION) {
        if (dataLen >= 1) {
            memset(report, 0x00, sizeof(report));
            handleConfigReport(data, dataLen);
        }
    }
}

void SwitchProDriver::readSPIFlash(uint8_t* dest, uint32_t address, uint8_t size) {
    memset(dest, 0xFF, size);

    if (address >= 0x6000 && address < (0x6000 + sizeof(factoryConfigData))) {
        uint32_t offset = address - 0x6000;
        uint32_t available = sizeof(factoryConfigData) - offset;
        uint32_t bytesToCopy = (size < available) ? size : available;
        memcpy(dest, factoryConfigData + offset, bytesToCopy);
    } else if (address >= 0x8000 && address < (0x8000 + sizeof(userCalibrationData))) {
        uint32_t offset = address - 0x8000;
        uint32_t available = sizeof(userCalibrationData) - offset;
        uint32_t bytesToCopy = (size < available) ? size : available;
        memcpy(dest, userCalibrationData + offset, bytesToCopy);
    }
}

// Only XboxOG and Xbox One use vendor control xfer cb
bool SwitchProDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return false;
}

const uint16_t * SwitchProDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)switch_pro_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * SwitchProDriver::get_descriptor_device_cb() {
    return switch_pro_device_descriptor;
}

const uint8_t * SwitchProDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return switch_pro_report_descriptor;
}

const uint8_t * SwitchProDriver::get_descriptor_configuration_cb(uint8_t index) {
    return switch_pro_configuration_descriptor;
}

const uint8_t * SwitchProDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t SwitchProDriver::GetJoystickMidValue() {
    return SWITCH_PRO_JOYSTICK_MID;
}

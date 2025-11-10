#include "drivers/switchpro/SwitchProDriver.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"
#include "pico/rand.h"

// force a report to be sent every X ms
#define SWITCH_PRO_KEEPALIVE_TIMER 5

void SwitchProDriver::initialize() {
    //stdio_init_all();

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
            .connectionInfo = 0,
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
        .rumbleReport = 0,
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

    // analog
    uint16_t scaleLeftStickX = scale16To12(gamepad->state.lx);
    uint16_t scaleLeftStickY = scale16To12(gamepad->state.ly);
    uint16_t scaleRightStickX = scale16To12(gamepad->state.rx);
    uint16_t scaleRightStickY = scale16To12(gamepad->state.ry);
    
    switchReport.inputs.leftStick.setX(std::min(std::max(scaleLeftStickX,leftMinX), leftMaxX));
    switchReport.inputs.leftStick.setY(-std::min(std::max(scaleLeftStickY,leftMinY), leftMaxY));
    switchReport.inputs.rightStick.setX(std::min(std::max(scaleRightStickX,rightMinX), rightMaxX));
    switchReport.inputs.rightStick.setY(-std::min(std::max(scaleRightStickY,rightMinY), rightMaxY));

    switchReport.rumbleReport = 0x09;
    //switchReport.reportID = inputMode;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

    if (isReportQueued) {
        if ((now - last_report_timer) > SWITCH_PRO_KEEPALIVE_TIMER) {
            if (tud_hid_ready() && sendReport(queuedReportID, report, 64) == true ) {
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
    //printf("SwitchProDriver::get_report Rpt: %02x, Type: %d, Len: %d\n", report_id, report_type, reqlen);
//    if (isReady) {
//        memcpy(buffer, &switchReport, sizeof(SwitchProReport));
//        return sizeof(SwitchProReport);
//    }

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

void SwitchProDriver::handleConfigReport(uint8_t switchReportID, uint8_t switchReportSubID, const uint8_t *reportData, uint16_t reportLength) {
    bool canSend = false;

    switch (switchReportSubID) {
        case SwitchOutputSubtypes::IDENTIFY:
            //printf("SwitchProDriver::set_report: IDENTIFY\n");
            sendIdentify();
            canSend = true;
            break;
        case SwitchOutputSubtypes::HANDSHAKE:
            //printf("SwitchProDriver::set_report: HANDSHAKE\n");
            report[0] = SwitchReportID::REPORT_USB_INPUT_81;
            report[1] = SwitchOutputSubtypes::HANDSHAKE;
            canSend = true;
            break;
        case SwitchOutputSubtypes::BAUD_RATE:
            //printf("SwitchProDriver::set_report: BAUD_RATE\n");
            report[0] = SwitchReportID::REPORT_USB_INPUT_81;
            report[1] = SwitchOutputSubtypes::BAUD_RATE;
            canSend = true;
            break;
        case SwitchOutputSubtypes::DISABLE_USB_TIMEOUT:
            //printf("SwitchProDriver::set_report: DISABLE_USB_TIMEOUT\n");
            report[0] = SwitchReportID::REPORT_OUTPUT_30;
            report[1] = switchReportSubID;
            //if (handshakeCounter < 4) {
            //    handshakeCounter++;
            //} else {
                isReady = true;
            //}
            canSend = true;
            break;
        case SwitchOutputSubtypes::ENABLE_USB_TIMEOUT:
            //printf("SwitchProDriver::set_report: ENABLE_USB_TIMEOUT\n");
            report[0] = SwitchReportID::REPORT_OUTPUT_30;
            report[1] = switchReportSubID;
            canSend = true;
            break;
        default:
            //printf("SwitchProDriver::set_report: Unknown Sub ID %02x\n", switchReportSubID);
            report[0] = SwitchReportID::REPORT_OUTPUT_30;
            report[1] = switchReportSubID;
            canSend = true;
            break;
    }

    if (canSend) isReportQueued = true;
}

void SwitchProDriver::handleFeatureReport(uint8_t switchReportID, uint8_t switchReportSubID, const uint8_t *reportData, uint16_t reportLength) {
    uint8_t commandID = reportData[10];
    uint32_t spiReadAddress = 0;
    uint8_t spiReadSize = 0;
    bool canSend = false;

    //uint8_t inputReportSize = sizeof(SwitchInputReport);
    //printf("inputReportSize: %d\n", inputReportSize);

    report[0] = SwitchReportID::REPORT_OUTPUT_21;
    report[1] = last_report_counter;
    memcpy(report+2,&switchReport.inputs,sizeof(SwitchInputReport));

    switch (commandID) {
        case SwitchCommands::GET_CONTROLLER_STATE:
            //printf("SwitchProDriver::set_report: Rpt 0x01 GET_CONTROLLER_STATE\n");
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::BLUETOOTH_PAIR_REQUEST:
            //printf("SwitchProDriver::set_report: Rpt 0x01 BLUETOOTH_PAIR_REQUEST\n");
            report[13] = 0x81;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::REQUEST_DEVICE_INFO:
            //printf("SwitchProDriver::set_report: Rpt 0x01 REQUEST_DEVICE_INFO\n");
            report[13] = 0x82;
            report[14] = 0x02;
            memcpy(&report[15], &deviceInfo, sizeof(deviceInfo));
            canSend = true;
            break;
        case SwitchCommands::SET_MODE:
            //printf("SwitchProDriver::set_report: Rpt 0x01 SET_MODE\n");
            inputMode = reportData[11];
            report[13] = 0x80;
            report[14] = 0x03;
            report[15] = inputMode;
            canSend = true;
            //printf("Input Mode set to ");
            switch (inputMode) {
                case 0x00:
                    //printf("NFC/IR Polling Data");
                    break;
                case 0x01:
                    //printf("NFC/IR Polling Config");
                    break;
                case 0x02:
                    //printf("NFC/IR Polling Data+Config");
                    break;
                case 0x03:
                    //printf("IR Scan");
                    break;
                case 0x23:
                    //printf("MCU Update");
                    break;
                case 0x30:
                    //printf("Full Input");
                    break;
                case 0x31:
                    //printf("NFC/IR");
                    break;
                case 0x3F:
                    //printf("Simple HID");
                    break;
                case 0x33:
                case 0x35:
                default:
                    //printf("Unknown");
                    break;
            }
            //printf("\n");
            break;
        case SwitchCommands::TRIGGER_BUTTONS:
            //printf("SwitchProDriver::set_report: Rpt 0x01 TRIGGER_BUTTONS\n");
            report[13] = 0x83;
            report[14] = 0x04;
            canSend = true;
            break;
        case SwitchCommands::SET_SHIPMENT:
            //printf("SwitchProDriver::set_report: Rpt 0x01 SET_SHIPMENT\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            //for (uint8_t i = 2; i < bufsize; i++) {
            //    //printf("%02x ", reportData[i]);
            //}
            //printf("\n");
            break;
        case SwitchCommands::SPI_READ:
            //printf("SwitchProDriver::set_report: Rpt 0x01 SPI_READ\n");
            spiReadAddress = (reportData[14] << 24) | (reportData[13] << 16) | (reportData[12] << 8) | (reportData[11]);
            spiReadSize = reportData[15];
            //printf("Read From: 0x%08x Size %d\n", spiReadAddress, spiReadSize);
            report[13] = 0x90;
            report[14] = reportData[10];
            report[15] = reportData[11];
            report[16] = reportData[12];
            report[17] = reportData[13];
            report[18] = reportData[14];
            report[19] = reportData[15];
            readSPIFlash(&report[20], spiReadAddress, spiReadSize);
            canSend = true;
            //printf("----------------------------------------------\n");
            break;
        case SwitchCommands::SET_NFC_IR_CONFIG:
            //printf("SwitchProDriver::set_report: Rpt 0x01 SET_NFC_IR_CONFIG\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::SET_NFC_IR_STATE:
            //printf("SwitchProDriver::set_report: Rpt 0x01 SET_NFC_IR_STATE\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::SET_PLAYER_LIGHTS:
            //printf("SwitchProDriver::set_report: Rpt 0x01 SET_PLAYER_LIGHTS\n");
            playerID = reportData[11];
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            //printf("Player set to %d\n", playerID);
            //printf("----------------------------------------------\n");
            break;
        case SwitchCommands::GET_PLAYER_LIGHTS:
            //printf("SwitchProDriver::set_report: Rpt 0x01 GET_PLAYER_LIGHTS\n");
            playerID = reportData[11];
            report[13] = 0xB0;
            report[14] = commandID;
            report[15] = playerID;
            canSend = true;
            //printf("Player is %d\n", playerID);
            //printf("----------------------------------------------\n");
            break;
        case SwitchCommands::COMMAND_UNKNOWN_33:
            //printf("SwitchProDriver::set_report: Rpt 0x01 COMMAND_UNKNOWN_33\n");
            // Command typically thrown by Chromium to detect if a Switch controller exists. Can ignore.
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
        case SwitchCommands::SET_HOME_LIGHT:
            //printf("SwitchProDriver::set_report: Rpt 0x01 SET_HOME_LIGHT\n");
            // NYI
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            break;
        case SwitchCommands::TOGGLE_IMU:
            //printf("SwitchProDriver::set_report: Rpt 0x01 TOGGLE_IMU\n");
            isIMUEnabled = reportData[11];
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            //printf("IMU set to %d\n", isIMUEnabled);
            //printf("----------------------------------------------\n");
            break;
        case SwitchCommands::IMU_SENSITIVITY:
            //printf("SwitchProDriver::set_report: Rpt 0x01 IMU_SENSITIVITY\n");
            report[13] = 0x80;
            report[14] = commandID;
            canSend = true;
            break;
        case SwitchCommands::ENABLE_VIBRATION:
            //printf("SwitchProDriver::set_report: Rpt 0x01 ENABLE_VIBRATION\n");
            isVibrationEnabled = reportData[11];
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x00;
            canSend = true;
            //printf("Vibration set to %d\n", isVibrationEnabled);
            //printf("----------------------------------------------\n");
            break;
        case SwitchCommands::READ_IMU:
            //printf("SwitchProDriver::set_report: Rpt 0x01 READ_IMU\n");
            report[13] = 0xC0;
            report[14] = commandID;
            report[15] = reportData[11];
            report[16] = reportData[12];
            canSend = true;
            //printf("IMU Addr: %02x, Size: %02x\n", reportData[11], reportData[12]);
            //printf("----------------------------------------------\n");
            break;
        case SwitchCommands::GET_VOLTAGE:
            //printf("SwitchProDriver::set_report: Rpt 0x01 GET_VOLTAGE\n");
            report[13] = 0xD0;
            report[14] = 0x50;
            report[15] = 0x83;
            report[16] = 0x06;
            canSend = true;
            break;
        default:
            //printf("SwitchProDriver::set_report: Rpt 0x01 Unknown 0x%02x\n", commandID);
            report[13] = 0x80;
            report[14] = commandID;
            report[15] = 0x03;
            canSend = true;
            break;
    }

    if (canSend) isReportQueued = true;
}

void SwitchProDriver::set_report(uint8_t report_id, hid_report_type_t report_type, const uint8_t *buffer, uint16_t bufsize) {
    if (report_type != HID_REPORT_TYPE_OUTPUT) return;

    memset(report, 0x00, bufsize);

    uint8_t switchReportID = buffer[0];
    uint8_t switchReportSubID = buffer[1];
    //printf("SwitchProDriver::set_report Rpt: %02x, Type: %d, Len: %d :: SID: %02x, SSID: %02x\n", report_id, report_type, bufsize, switchReportID, switchReportSubID);
    if (switchReportID == SwitchReportID::REPORT_OUTPUT_00) {
    } else if (switchReportID == SwitchReportID::REPORT_FEATURE) {
        queuedReportID = report_id;
        handleFeatureReport(switchReportID, switchReportSubID, buffer, bufsize);
    } else if (switchReportID == SwitchReportID::REPORT_CONFIGURATION) {
        queuedReportID = report_id;
        handleConfigReport(switchReportID, switchReportSubID, buffer, bufsize);
    } else {
        //printf("SwitchProDriver::set_report Rpt: %02x, Type: %d, Len: %d :: SID: %02x, SSID: %02x\n", report_id, report_type, bufsize, switchReportID, switchReportSubID);
    }
}

void SwitchProDriver::readSPIFlash(uint8_t* dest, uint32_t address, uint8_t size) {
    uint32_t addressBank = address & 0xFFFFFF00;
    uint32_t addressOffset = address & 0x000000FF;
    //printf("Address: %08x, Bank: %04x, Offset: %04x, Size: %d\n", address, addressBank, addressOffset, size);
    std::map<uint32_t, const uint8_t*>::iterator it = spiFlashData.find(addressBank);

    if (it != spiFlashData.end()) {
        // address found
        const uint8_t* data = it->second;
        memcpy(dest, data+addressOffset, size);
        //for (uint8_t i = 0; i < size; i++) printf("%02x ", dest[i]);
        //printf("\n---\n");
    } else {
        // could not find defined address
        //printf("Not Found\n");
        memset(dest, 0xFF, size);
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

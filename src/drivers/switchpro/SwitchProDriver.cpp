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
        .storedColors = 0x01,
    };

	switchReport = {
        .reportID = 0x30,
        .timestamp = 0,

        .inputs {
            .connection_info = 0,
            .battery_level = 0x08,

            // byte 00
            .button_y = 0,
            .button_x = 0,
            .button_b = 0,
            .button_a = 0,
            .button_right_sl = 0,
            .button_right_sr = 0,
            .button_r = 0,
            .button_zr = 0,

            // byte 01
            .button_minus = 0,
            .button_plus = 0,
            .button_thumb_r = 0,
            .button_thumb_l = 0,
            .button_home = 0,
            .button_capture = 0,
            .dummy = 0,
            .charging_grip = 0,

            // byte 02
            .dpad_down = 0,
            .dpad_up = 0,
            .dpad_right = 0,
            .dpad_left = 0,
            .button_left_sl = 0,
            .button_left_sr = 0,
            .button_l = 0,
            .button_zl = 0,
            .analog = {0x00},
        },
        .vibrator_input_report = 0,
        .imu_data = {0x00},
        .padding = {0x00}
    };

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
    switchReport.inputs.dpad_up =    ((gamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_UP);
    switchReport.inputs.dpad_down =  ((gamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_DOWN);
    switchReport.inputs.dpad_left =  ((gamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_LEFT);
    switchReport.inputs.dpad_right = ((gamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_RIGHT);

    switchReport.inputs.charging_grip = 1;

    switchReport.inputs.button_y = gamepad->pressedB3();
    switchReport.inputs.button_x = gamepad->pressedB4();
    switchReport.inputs.button_b = gamepad->pressedB1();
    switchReport.inputs.button_a = gamepad->pressedB2();
    switchReport.inputs.button_right_sr = 0;
    switchReport.inputs.button_right_sl = 0;
    switchReport.inputs.button_r = gamepad->pressedR1();
    switchReport.inputs.button_zr = gamepad->pressedR2();
    switchReport.inputs.button_minus = gamepad->pressedS1();
    switchReport.inputs.button_plus = gamepad->pressedS2();
    switchReport.inputs.button_thumb_r = gamepad->pressedR3();
    switchReport.inputs.button_thumb_l = gamepad->pressedL3();
    switchReport.inputs.button_home = gamepad->pressedA1();
    switchReport.inputs.button_capture = gamepad->pressedA2();
    switchReport.inputs.button_left_sr = 0;
    switchReport.inputs.button_left_sl = 0;
    switchReport.inputs.button_l = gamepad->pressedL1();
    switchReport.inputs.button_zl = gamepad->pressedL2();

    // analog
    //switchReport.inputs.analog[0] = (gamepad->state.lx & 0xFF);
    //switchReport.inputs.analog[1] = ((gamepad->state.lx >> 8) & 0x0F) | ((gamepad->state.ly & 0x0F) << 4);
    //switchReport.inputs.analog[2] = (gamepad->state.ly >> 4) & 0xFF;
    //
    //switchReport.inputs.analog[3] = (gamepad->state.rx & 0xFF);
    //switchReport.inputs.analog[4] = ((gamepad->state.rx >> 8) & 0x0F) | ((gamepad->state.ry & 0x0F) << 4);
    //switchReport.inputs.analog[5] = (gamepad->state.ry >> 4) & 0xFF;

    switchReport.vibrator_input_report = 0x09;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

    if (isReportQueued) {
        if (tud_hid_ready() && sendReport(queuedReportID, report, 64) == true ) {
        }
        isReportQueued = false;
    }

    Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
    processedGamepad->auxState.playerID.active = true;
    processedGamepad->auxState.playerID.ledValue = playerID;
    processedGamepad->auxState.playerID.value = playerID;

    if (isReady) {
        switchReport.timestamp = last_report_counter;
        void * inputReport = &switchReport;
        uint16_t report_size = sizeof(switchReport);
        if (memcmp(last_report, inputReport, report_size) != 0) {
            // HID ready + report sent, copy previous report
            if (tud_hid_ready() && sendReport(0, inputReport, report_size) == true ) {
                memcpy(last_report, inputReport, report_size);
                return true;
            }
        }
    } else {
        if (!isInitialized) {
            // send identification
            sendIdentify();
            if (tud_hid_ready() && tud_hid_report(0, report, 64) == true) {
                isInitialized = true;
            }
        }
    }

    return false;
}

// tud_hid_get_report_cb
uint16_t SwitchProDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    if (isReady) {
        //printf("SwitchProDriver::get_report Rpt: %02x, Type: %d, Len: %d\n", report_id, report_type, reqlen);
        memcpy(buffer, &switchReport, sizeof(SwitchProReport));
        return sizeof(SwitchProReport);
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

void SwitchProDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    if (isInitialized) {
        uint8_t switchReportID = buffer[0];
        uint8_t switchReportSubID = buffer[1];
      //printf("SwitchProDriver::set_report Rpt: %02x, Type: %d, Len: %d :: SID: %02x, SSID: %02x\n", report_id, report_type, bufsize, switchReportID, switchReportSubID);
      //for (uint8_t i = 2; i < bufsize; i++) {
      //    printf("%02x ", buffer[i]);
      //}
      //printf("\n");
        bool canSend = false;
        switch (report_type) {
            case HID_REPORT_TYPE_OUTPUT:
                memset(report, 0x00, bufsize);

                if (switchReportID == SwitchReportID::REPORT_USB_OUTPUT_80) {
                    if (switchReportSubID == SwitchOutputSubtypes::IDENTIFY) {
                        //printf("SwitchProDriver::set_report: IDENTIFY\n");
                        sendIdentify();
                        canSend = true;
                    } else if (switchReportSubID == SwitchOutputSubtypes::HANDSHAKE) {
                        //printf("SwitchProDriver::set_report: HANDSHAKE\n");
                        report[0] = SwitchReportID::REPORT_USB_INPUT_81;
                        report[1] = SwitchOutputSubtypes::HANDSHAKE;
                        canSend = true;
                    } else if (switchReportSubID == SwitchOutputSubtypes::BAUD_RATE) {
                        //printf("SwitchProDriver::set_report: BAUD_RATE\n");
                        report[0] = SwitchReportID::REPORT_USB_INPUT_81;
                        report[1] = SwitchOutputSubtypes::BAUD_RATE;
                        canSend = true;
                    } else if (switchReportSubID == SwitchOutputSubtypes::DISABLE_USB_TIMEOUT) {
                        //printf("SwitchProDriver::set_report: DISABLE_USB_TIMEOUT\n");
                        report[0] = SwitchReportID::REPORT_OUTPUT_30;
                        report[1] = switchReportSubID;
                        //if (handshakeCounter < 4) {
                        //    handshakeCounter++;
                        //} else {
                            isReady = true;
                        //}
                        canSend = true;
                    } else if (switchReportSubID == SwitchOutputSubtypes::ENABLE_USB_TIMEOUT) {
                        //printf("SwitchProDriver::set_report: ENABLE_USB_TIMEOUT\n");
                        report[0] = SwitchReportID::REPORT_OUTPUT_30;
                        report[1] = switchReportSubID;
                        canSend = true;
                    } else {
                        //printf("SwitchProDriver::set_report: Unknown Sub ID %02x\n", switchReportSubID);
                        report[0] = SwitchReportID::REPORT_OUTPUT_30;
                        report[1] = switchReportSubID;
                        canSend = true;
                    }
                } else if (switchReportID == SwitchReportID::REPORT_OUTPUT_01) {
                    uint8_t commandID = buffer[10];
                    uint32_t spiReadAddress = 0;
                    uint8_t spiReadSize = 0;

                    uint8_t inputReportSize = sizeof(SwitchInputReport);

                    report[0] = SwitchReportID::REPORT_OUTPUT_21;
                    report[1] = last_report_counter;
                    memcpy(report+2,&switchReport.inputs,sizeof(SwitchInputReport));
                    //printf("inputReportSize: %d\n", inputReportSize);

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
                            inputMode = buffer[11];
                            report[13] = 0x80;
                            report[14] = 0x03;
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
                            //printf("\n----------------------------------------------\n");
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
                            //    //printf("%02x ", buffer[i]);
                            //}
                            //printf("\n");
                            break;
                        case SwitchCommands::SPI_READ:
                            //printf("SwitchProDriver::set_report: Rpt 0x01 SPI_READ\n");
                            spiReadAddress = (buffer[14] << 24) | (buffer[13] << 16) | (buffer[12] << 8) | (buffer[11]);
                            spiReadSize = buffer[15];
                            //printf("Read From: 0x%08x Size %d\n", spiReadAddress, spiReadSize);
                            report[13] = 0x90;
                            report[14] = buffer[10];
                            report[15] = buffer[11];
                            report[16] = buffer[12];
                            report[17] = buffer[13];
                            report[18] = buffer[14];
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
                            playerID = buffer[11];
                            report[13] = 0x80;
                            report[14] = commandID;
                            canSend = true;
                            //printf("Player set to %d\n", playerID);
                            //printf("----------------------------------------------\n");
                            break;
                        case SwitchCommands::GET_PLAYER_LIGHTS:
                            //printf("SwitchProDriver::set_report: Rpt 0x01 GET_PLAYER_LIGHTS\n");
                            playerID = buffer[11];
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
                            isIMUEnabled = buffer[11];
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
                            isVibrationEnabled = buffer[11];
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
                            report[15] = buffer[11];
                            report[16] = buffer[12];
                            canSend = true;
                            //printf("IMU Addr: %02x, Size: %02x\n", buffer[11], buffer[12]);
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
                } else {
                    //printf("SwitchProDriver::set_report: Rpt %02x Unknown ID %02x\n", switchReportID, switchReportSubID);
                }

                if (canSend) {
                    //sendReport(report_id, report, bufsize);
                    queuedReportID = report_id;
                    isReportQueued = true;
                }

                break;
            default:
                //printf("SwitchProDriver::set_report: Unknown HID Report %02x\n", switchReportID);
                break;
        }
    }
}

void SwitchProDriver::readSPIFlash(uint8_t* dest, uint32_t address, uint8_t size) {
    uint32_t addressBank = address & 0xFFFFFF00;
    uint32_t addressOffset = address & 0x000000FF;
    //printf("Address: %08x, Bank: %04x, Offset: %04x\n", address, addressBank, addressOffset);
    std::map<uint32_t, std::vector<uint8_t>>::iterator it = spiFlashData.find(addressBank);

    if (it != spiFlashData.end()) {
        // address found
        for (it = spiFlashData.begin(); it != spiFlashData.end(); ++it) {
            std::vector<uint8_t>& data = it->second;
            memcpy(dest, data.data()+addressOffset, size);
        }
    } else {
        // could not find defined address
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
	return SWITCH_PRO_JOYSTICK_MID << 8;
}

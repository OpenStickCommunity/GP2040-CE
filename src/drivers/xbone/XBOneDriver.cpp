#include "drivers/xbone/XBOneDriver.h"
#include "drivers/shared/driverhelper.h"

#include "drivers/xbone/XBOneAuth.h"
#include "peripheralmanager.h"
#include "storagemanager.h"

#define XBONE_KEEPALIVE_TIMER 15000

#define USB_SETUP_DEVICE_TO_HOST 0x80
#define USB_SETUP_HOST_TO_DEVICE 0x00
#define USB_SETUP_TYPE_VENDOR    0x40
#define USB_SETUP_TYPE_CLASS     0x20
#define USB_SETUP_TYPE_STANDARD  0x00
#define USB_SETUP_RECIPIENT_INTERFACE   0x01
#define USB_SETUP_RECIPIENT_DEVICE      0x00
#define USB_SETUP_RECIPIENT_ENDPOINT    0x02
#define USB_SETUP_RECIPIENT_OTHER       0x03

#define REQ_GET_OS_FEATURE_DESCRIPTOR 0x20
#define DESC_EXTENDED_COMPATIBLE_ID_DESCRIPTOR 0x0004
#define DESC_EXTENDED_PROPERTIES_DESCRIPTOR 0x0005
#define REQ_GET_XGIP_HEADER 0x90

// Check report queue every 35 milliseconds
static uint32_t lastReportQueue = 0;
#define REPORT_QUEUE_INTERVAL 35

typedef enum {
    READY_ANNOUNCE,
    WAIT_DESCRIPTOR_REQUEST,
    SEND_DESCRIPTOR,
    SETUP_AUTH,
    AUTH_DONE,
    NOT_READY
} XboxOneDriverState;

static XboxOneDriverState xboneDriverState = NOT_READY;

static uint8_t xb1_guide_on[] = { 0x01, 0x5b };
static uint8_t xb1_guide_off[] = { 0x00, 0x5b };

// Neutral 14-byte gamepad core: buttons, triggers, LX/LY/RX/RY
static uint8_t xboneIdle[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
                       0x00, 0x00, 0xff, 0xff, 0x00, 0x00};


// Security status: 01 00 = authenticated; 01 01 = retry; 01 02 = failed
const uint8_t authReady[] = {0x01, 0x00};

// Xbox One Announce
static uint8_t announcePacket[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Device ID; filled from board ID
    0x6F, 0x0E, 0xA4, 0x02,                         // USB vendor/product IDs
    0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, // Firmware major/minor/build/revision
    0x01, 0x00,                                     // Hardware version
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00};           // RF, security and GIP versions

// Xbox One Descriptor
const uint8_t xboxOneDescriptor[] = {
    // Metadata header: device block at 0x10, format 1.0, total 182 bytes
    0x10, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB6, 0x00,
    // Device block: size, firmware/hardware/command/class/interface offsets, reserved
    0x77, 0x00, 0x16, 0x00, 0x1B, 0x00, 0x1C, 0x00,
    0x23, 0x00, 0x29, 0x00, 0x46, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // Supported firmware: one major/minor pair, 1.2. No hardware-version filter
    0x01, 0x01, 0x00, 0x02, 0x00, 0x00,
    // System commands: device to host, then host to device
    0x06, 0x01, 0x02, 0x03, 0x04, 0x06, 0x07, 0x05,
    0x01, 0x04, 0x05, 0x06, 0x0A,
    // One class name: Windows.Xbox.Input.Gamepad (26 UTF-8 bytes)
    0x01, 0x1A, 0x00, 0x57, 0x69, 0x6E, 0x64, 0x6F,
    0x77, 0x73, 0x2E, 0x58, 0x62, 0x6F, 0x78, 0x2E,
    0x49, 0x6E, 0x70, 0x75, 0x74, 0x2E, 0x47, 0x61,
    0x6D, 0x65, 0x70, 0x61, 0x64,
    // Three interfaces: controller, gamepad and navigation (GUIDs in wire byte order)
    0x03, 0x56, 0xFF, 0x76, 0x97, 0xFD, 0x9B, 0x81,
    0x45, 0xAD, 0x45, 0xB6, 0x45, 0xBB, 0xA5, 0x26,
    0xD6, 0x2C, 0x40, 0x2E, 0x08, 0xDF, 0x07, 0xE1,
    0x45, 0xA5, 0xAB, 0xA3, 0x12, 0x7A, 0xF1, 0x97,
    0xB5, 0xE7, 0x1F, 0xF3, 0xB8, 0x86, 0x73, 0xE9,
    0x40, 0xA9, 0xF8, 0x2F, 0x21, 0x26, 0x3A, 0xCF,
    0xB7,
    // Two message descriptors: type, payload size, data type, flags, timing/reserved
    0x02,
    // 0x20 input: 14-byte core, upstream. No CFM or DLI extension
    0x17, 0x00, 0x20, 0x0E, 0x00, 0x01, 0x00, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    // 0x09 vibration: up to 60 bytes, downstream
    0x17, 0x00, 0x09, 0x3C, 0x00, 0x01, 0x00, 0x08,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static bool input_report_sent = false;
static bool waiting_ack = false;
static uint32_t waiting_ack_timeout=0;
static uint32_t timer_wait_for_announce;
static bool xbox_one_powered_on;
static uint8_t report_led_mode;
static uint8_t report_led_brightness;

// Report Queue for big report sizes from dongle
#include <queue>
typedef struct {
    uint8_t report[XBONE_ENDPOINT_SIZE];
    uint16_t len;
} report_queue_t;

static std::queue<report_queue_t> report_queue;

#define XGIP_ACK_WAIT_TIMEOUT 2000

#define CFG_TUD_XBONE 8
#define CFG_TUD_XINPUT_TX_BUFSIZE 64
#define CFG_TUD_XINPUT_RX_BUFSIZE 64

typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;         // optional Out endpoint
    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[CFG_TUD_XINPUT_TX_BUFSIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[CFG_TUD_XINPUT_RX_BUFSIZE];
} xboned_interface_t;

CFG_TUSB_MEM_SECTION static xboned_interface_t _xboned_itf[CFG_TUD_XBONE];

static XGIPProtocol * outgoingXGIP = nullptr;
static XGIPProtocol * incomingXGIP = nullptr;
static XboxOneAuthData * xboxOneAuthData = nullptr;

// Windows requires a Descriptor Single for Xbox One
typedef struct {
    uint32_t TotalLength;
    uint16_t Version;
    uint16_t Index;
    uint8_t TotalSections;
    uint8_t Reserved[7];
    uint8_t FirstInterfaceNumber;
    uint8_t Reserved2;
    uint8_t CompatibleID[8];
    uint8_t SubCompatibleID[8];
    uint8_t Reserved3[6];
} __attribute__((packed)) OS_COMPATIBLE_ID_DESCRIPTOR_SINGLE;

const OS_COMPATIBLE_ID_DESCRIPTOR_SINGLE DevCompatIDsOne = {
    TotalLength : sizeof(OS_COMPATIBLE_ID_DESCRIPTOR_SINGLE),
    Version : 0x0100,
    Index : DESC_EXTENDED_COMPATIBLE_ID_DESCRIPTOR,
    TotalSections : 1,
    Reserved : {0},
    Reserved2 : 0x01,
    CompatibleID : {'X','G','I','P','1','0',0,0},
    SubCompatibleID : {0},
    Reserved3 : {0}
};

static void xbone_reset(uint8_t rhport) {
    (void)rhport;
    timer_wait_for_announce = to_ms_since_boot(get_absolute_time());
    xbox_one_powered_on = false;
    input_report_sent = false;
    waiting_ack = false;
    if (xboxOneAuthData != nullptr) xboxOneAuthData->authCompleted = false;
    report_led_mode = 0; // 0 = OFF
    while(!report_queue.empty())
        report_queue.pop();

    // close any endpoints that are open
    tu_memclr(&_xboned_itf, sizeof(_xboned_itf));
}

static void xbone_init(void) {
    xbone_reset(TUD_OPT_RHPORT);
}

static uint16_t xbone_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
    uint16_t drv_len = 0;
    if (TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass) {
        TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass, 0);
        
        drv_len = sizeof(tusb_desc_interface_t) +
                  (itf_desc->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_VERIFY(max_len >= drv_len, 0);

        // Find available interface
        xboned_interface_t *p_xbone = NULL;
        for (uint8_t i = 0; i < CFG_TUD_XBONE; i++) {
            if (_xboned_itf[i].ep_in == 0 && _xboned_itf[i].ep_out == 0) {
                p_xbone = &_xboned_itf[i];
                break;
            }
        }

        TU_VERIFY(p_xbone, 0);
        uint8_t const *p_desc = (uint8_t const *)itf_desc;

        // Xbox One interface  (subclass = 0x47, protocol = 0xD0)
        if (itf_desc->bInterfaceSubClass == 0x47 &&
                   itf_desc->bInterfaceProtocol == 0xD0) {
            p_desc = tu_desc_next(p_desc);
            TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, itf_desc->bNumEndpoints, TUSB_XFER_INTERRUPT, &p_xbone->ep_out, &p_xbone->ep_in), 0);

            p_xbone->itf_num = itf_desc->bInterfaceNumber;

            // Prepare for output endpoint
            if (p_xbone->ep_out) {
                if (!usbd_edpt_xfer(rhport, p_xbone->ep_out, p_xbone->epout_buf, sizeof(p_xbone->epout_buf), false)) {
                    TU_LOG_FAILED();
                    TU_BREAKPOINT();
                }
            }

            // Setting up XGIPs and driver state
            if (incomingXGIP != nullptr && outgoingXGIP != nullptr ) {
                xboneDriverState = XboxOneDriverState::READY_ANNOUNCE;
                incomingXGIP->reset();
                outgoingXGIP->reset();
            }
        }
    }

    return drv_len;
}

static void queue_xbone_report(void *report, uint16_t report_size) {
    report_queue_t item;
    memcpy(item.report, report, report_size);
    item.len = report_size;
    report_queue.push(item);
}

// DevCompatIDsOne sends back XGIP10 data when requested by Windows
bool xbone_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request) {
    if (stage != CONTROL_STAGE_SETUP) return true;
    if (request->bmRequestType != (USB_SETUP_DEVICE_TO_HOST | USB_SETUP_RECIPIENT_DEVICE | USB_SETUP_TYPE_VENDOR) ||
        request->bRequest != REQ_GET_OS_FEATURE_DESCRIPTOR || request->wValue != 0 ||
        request->wIndex != DESC_EXTENDED_COMPATIBLE_ID_DESCRIPTOR) return false;
    return tud_control_xfer(rhport, request, (void *)&DevCompatIDsOne, sizeof(DevCompatIDsOne));
}

bool xbone_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result,
                     uint32_t xferred_bytes) {
    // Do nothing if we couldn't setup our auth listener
    if ( xboxOneAuthData == nullptr || incomingXGIP == nullptr ||
            outgoingXGIP == nullptr) {
        return true;
    }
    
    uint8_t itf = 0;
    xboned_interface_t *p_xbone = _xboned_itf;

    for (;; itf++, p_xbone++) {
        if (itf >= TU_ARRAY_SIZE(_xboned_itf)) return false;
        if (ep_addr == p_xbone->ep_out || ep_addr == p_xbone->ep_in) break;
    }

    if (ep_addr == p_xbone->ep_out) {
        if (result != XFER_RESULT_SUCCESS || xferred_bytes > sizeof(p_xbone->epout_buf)) {
            return usbd_edpt_xfer(rhport, p_xbone->ep_out, p_xbone->epout_buf, sizeof(p_xbone->epout_buf), false);
        }
        // Parse incoming packet and verify its valid
        incomingXGIP->parse(p_xbone->epout_buf, xferred_bytes);

        uint8_t command = incomingXGIP->getCommand();

        if ( (command == GIP_AUTH || command == GIP_FINAL_AUTH) && !xboxOneAuthData->deviceMounted ) {
            xboxOneAuthData->consoleAuthOrphaned = true;
        }

        // Relay auth traffic unchanged; the controller generates the ACKs
        if ( xboxOneAuthData->auth_passthrough_enabled &&
            xbone_is_auth_packet(p_xbone->epout_buf, xferred_bytes) ) {
            xboxOneAuthData->auth_passthrough = true;
            if ( (command == GIP_AUTH || command == GIP_FINAL_AUTH) &&
                incomingXGIP->getChunked() == false &&
                incomingXGIP->getDataLength() == sizeof(authReady) &&
                memcmp(incomingXGIP->getData(), authReady, sizeof(authReady)) == 0 ) {
                xboxOneAuthData->authCompleted = true;
                input_report_sent = false;
                xboneDriverState = AUTH_DONE;
            }

            XBOneRelayPacket packet;
            packet.len = TU_MIN((uint16_t)xferred_bytes, (uint16_t)XBONE_RELAY_PACKET_SIZE);
            memcpy(packet.data, p_xbone->epout_buf, packet.len);
            if ( !queue_try_add(&xboxOneAuthData->relayToDevice, &packet) )
                xboxOneAuthData->relayDropped++;

            incomingXGIP->reset();
            TU_ASSERT(usbd_edpt_xfer(rhport, p_xbone->ep_out, p_xbone->epout_buf,
                                    sizeof(p_xbone->epout_buf), false));
            return true;
        }

        if (!incomingXGIP->validate()) {
            return usbd_edpt_xfer(rhport, p_xbone->ep_out, p_xbone->epout_buf, sizeof(p_xbone->epout_buf), false);
        }

        // Setup an ack before we change anything about the incoming packet
        if ( incomingXGIP->ackRequired() == true ) {
            queue_xbone_report((uint8_t*)incomingXGIP->generateAckPacket(), incomingXGIP->getPacketLength());
        }

        if ( command == GIP_ACK_RESPONSE ) {
            waiting_ack = false;
        } else if ( command == GIP_DEVICE_DESCRIPTOR ) {
            // setup descriptor packet
            outgoingXGIP->reset(); // reset if anything was in there
            outgoingXGIP->setAttributes(GIP_DEVICE_DESCRIPTOR, incomingXGIP->getSequence(), 1, 1, 0);
            outgoingXGIP->setData(xboxOneDescriptor, sizeof(xboxOneDescriptor));
            xboneDriverState = XboxOneDriverState::SEND_DESCRIPTOR;
        } else if ( command == GIP_POWER_MODE_DEVICE_CONFIG ) {
            // Power Mode On!
            xbox_one_powered_on = true;
        } else if ( command == GIP_CMD_LED_ON && incomingXGIP->getDataLength() >= 3 ) {
            // Set all player LEDs to on
            report_led_mode = incomingXGIP->getData()[1]; // 1 - turn LEDs on
            report_led_brightness = incomingXGIP->getData()[2]; // 2 - brightness (ignored for now)

            // Send our descriptor if descriptor is waiting (Player 2)
            if ( xboneDriverState == XboxOneDriverState::WAIT_DESCRIPTOR_REQUEST ) {
                outgoingXGIP->reset(); // reset if anything was in there
                outgoingXGIP->setAttributes(GIP_DEVICE_DESCRIPTOR, incomingXGIP->getSequence(), 1, 1, 0);
                outgoingXGIP->setData(xboxOneDescriptor, sizeof(xboxOneDescriptor));
                xboneDriverState = XboxOneDriverState::SEND_DESCRIPTOR;
            }
        } else if ( command == GIP_CMD_RUMBLE ) {
            // TO-DO
        } else if ( command == GIP_AUTH || command == GIP_FINAL_AUTH) {
            if (incomingXGIP->getDataLength() == 2 && memcmp(incomingXGIP->getData(), authReady, sizeof(authReady))==0 ) {
                xboxOneAuthData->authCompleted = true;
                input_report_sent = false;
                xboneDriverState = AUTH_DONE;
            }
            if ( (incomingXGIP->getChunked() == true && incomingXGIP->endOfChunk() == true) ||
                    (incomingXGIP->getChunked() == false )) {
                xboxOneAuthData->consoleBuffer.setBuffer(incomingXGIP->getData(), incomingXGIP->getDataLength(),
                    incomingXGIP->getSequence(), incomingXGIP->getCommand());
                xboxOneAuthData->xboneState = GPAuthState::send_auth_console_to_dongle;
                incomingXGIP->reset();
            }
        }

        TU_ASSERT(usbd_edpt_xfer(rhport, p_xbone->ep_out, p_xbone->epout_buf,
                                 sizeof(p_xbone->epout_buf), false));
    } else if (ep_addr == p_xbone->ep_in) {
        // Nothing needed
    }
    return true;
}

void XBOneDriver::initialize() {
    xboneReport = {
        .reserved = 0,
        .keepAlive = 0,
        .start = 0,
        .back = 0,
        .a = 0,
        .b = 0,
        .x = 0,
        .y = 0,
        .dpadUp = 0,
        .dpadDown = 0,
        .dpadLeft = 0,
        .dpadRight = 0,
        .leftShoulder = 0,
        .rightShoulder = 0,
        .leftThumbClick = 0,
        .rightThumbClick = 0,
        .leftTrigger = 0,
        .rightTrigger = 0,
        .leftStickX = GAMEPAD_JOYSTICK_MID,
        .leftStickY = GAMEPAD_JOYSTICK_MID,
        .rightStickX = GAMEPAD_JOYSTICK_MID,
        .rightStickY = GAMEPAD_JOYSTICK_MID
    };
    class_driver = 	{
#if CFG_TUSB_DEBUG >= 2
        .name = "XBONE",
#endif
        .init = xbone_init,
        .reset = xbone_reset,
        .open = xbone_open,
        .control_xfer_cb = xbone_vendor_control_xfer_cb,
        .xfer_cb = xbone_xfer_cb,
        .sof = NULL
    };

    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);
    memcpy(announcePacket, id.id + PICO_UNIQUE_BOARD_ID_SIZE_BYTES - 6, 6);

    keep_alive_timer = to_ms_since_boot(get_absolute_time());
    keep_alive_sequence = 1; // GIP sequence zero is reserved.
    virtual_keycode_sequence = 0;
    xb1_guide_pressed = false;
    last_report_counter = 0;

    incomingXGIP = new XGIPProtocol();
    outgoingXGIP = new XGIPProtocol();

    xboxOneAuthData = nullptr;

    xbone_led_mode = 0;
}


void XBOneDriver::initializeAux() {
    authDriver = new XBOneAuth();
    if ( authDriver->available() ) {
        authDriver->initialize();
        xboxOneAuthData = ((XBOneAuth*)authDriver)->getAuthData();
    } else {
        xboxOneAuthData = nullptr;
    }
}

USBListener * XBOneDriver::get_usb_auth_listener() {
    if ( authDriver->available() ) {
        return authDriver->getListener();
    }
    return nullptr;
}

bool XBOneDriver::process(Gamepad * gamepad) {
    // Do nothing if we couldn't setup our auth listener
    if ( xboxOneAuthData == nullptr) {
        return false;
    }

    uint16_t xboneReportSize = 0;

    // Perform update
    this->update();

    // Check if LEDs need to turn on
    if ( xbone_led_mode != report_led_mode ) {
        Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
        processedGamepad->auxState.playerID.active = true;
        processedGamepad->auxState.playerID.ledValue = report_led_mode;
        processedGamepad->auxState.playerID.ledBlinkOn = report_led_brightness;
    }

    // Publish one neutral report while authentication is pending.
    if ( xboxOneAuthData->authCompleted == false ) {
        if (input_report_sent) return false;
        GIP_HEADER((&xboneReport), GIP_INPUT_REPORT, false, last_report_counter + 1);
        if (xboneReport.Header.sequence == 0) xboneReport.Header.sequence = 1;
        memcpy((uint8_t *)&xboneReport + sizeof(GipHeader_t), xboneIdle, sizeof(xboneIdle));
        if (!send_xbone_usb((uint8_t *)&xboneReport, sizeof(xboneReport))) return false;
        last_report_counter = xboneReport.Header.sequence;
        input_report_sent = true;
        return true;
    }

    uint32_t now = to_ms_since_boot(get_absolute_time());
    // Send Keep-Alive every 15 seconds (keep_alive_timer updates if send is successful)
    if ( (now - keep_alive_timer) > XBONE_KEEPALIVE_TIMER) {
        memset(&xboneReport.Header, 0, sizeof(GipHeader_t));
        GIP_HEADER((&xboneReport), GIP_KEEPALIVE, 1, keep_alive_sequence);
        xboneReport.Header.length = 4;
        static uint8_t keepAlive[] = { 0x80, 0x00, 0x00, 0x00 };
        memcpy(&((uint8_t*)&xboneReport)[4], &keepAlive, sizeof(keepAlive));
        xboneReportSize = sizeof(GipHeader_t) + sizeof(keepAlive);
        // If successful, update our keep alive timer/sequence
        if ( send_xbone_usb((uint8_t*)&xboneReport, xboneReportSize) == true ) {
            keep_alive_timer = to_ms_since_boot(get_absolute_time());
            keep_alive_sequence++; // will rollover
            if ( keep_alive_sequence == 0 )
                keep_alive_sequence = 1;
        }
        return true;
    }
    
    // Virtual Keycode for Guide Button
    bool virtual_keycode_change = false;
    if ( (xb1_guide_pressed == true && !gamepad->pressedA1())||
        (xb1_guide_pressed == false && gamepad->pressedA1()) ) {
        virtual_keycode_change = true;
    }

    // Virtual Keycode Triggered (Pressed or Released)
    if ( virtual_keycode_change == true ) {
        uint8_t new_sequence = virtual_keycode_sequence;
        new_sequence++; // will rollover
        if ( new_sequence == 0 )
            new_sequence = 1;
        GIP_HEADER((&xboneReport), GIP_VIRTUAL_KEYCODE, 1, new_sequence);
        if ( xb1_guide_pressed == false ) {
            xboneReport.Header.length = sizeof(xb1_guide_on);
            memcpy(&((uint8_t*)&xboneReport)[4], &xb1_guide_on, sizeof(xb1_guide_on));
            xboneReportSize = sizeof(GipHeader_t) + sizeof(xb1_guide_on);
        } else {
            xboneReport.Header.length = sizeof(xb1_guide_off);
            memcpy(&((uint8_t*)&xboneReport)[4], &xb1_guide_off, sizeof(xb1_guide_off));
            xboneReportSize = sizeof(GipHeader_t) + sizeof(xb1_guide_off);
        }
        if ( send_xbone_usb((uint8_t*)&xboneReport, xboneReportSize) == true ) {
            // On success, update our guide pressed state and virtual key code state
            virtual_keycode_sequence = new_sequence;
            xb1_guide_pressed = !xb1_guide_pressed;
        }
        return true;
    }

    // Only change xbox one input report if we have different inputs!
    XboxOneGamepad_Data_t newInputReport;

    // This is due to our tusb_driver.cpp checking memcmp(last_report, report, size)
    memset(&newInputReport, 0, sizeof(XboxOneGamepad_Data_t));
    GIP_HEADER((&newInputReport), GIP_INPUT_REPORT, false, last_report_counter);

    newInputReport.a = gamepad->pressedB1();
    newInputReport.b = gamepad->pressedB2();
    newInputReport.x = gamepad->pressedB3();
    newInputReport.y = gamepad->pressedB4();
    newInputReport.leftShoulder = gamepad->pressedL1();
    newInputReport.rightShoulder = gamepad->pressedR1();
    newInputReport.leftThumbClick = gamepad->pressedL3();
    newInputReport.rightThumbClick = gamepad->pressedR3();
    newInputReport.start = gamepad->pressedS2();
    newInputReport.back = gamepad->pressedS1();
    newInputReport.keepAlive = 0; // Guide uses system 0x07 / virtual key 0x5B.
    newInputReport.reserved = 0;
    newInputReport.dpadUp = gamepad->pressedUp();
    newInputReport.dpadDown = gamepad->pressedDown();
    newInputReport.dpadLeft = gamepad->pressedLeft();
    newInputReport.dpadRight = gamepad->pressedRight();

    newInputReport.leftStickX = static_cast<int16_t>(gamepad->state.lx) + INT16_MIN;
    newInputReport.leftStickY = static_cast<int16_t>(~gamepad->state.ly) + INT16_MIN;
    newInputReport.rightStickX = static_cast<int16_t>(gamepad->state.rx) + INT16_MIN;
    newInputReport.rightStickY = static_cast<int16_t>(~gamepad->state.ry) + INT16_MIN;

    // Expand 8-bit triggers to 10 bits by replicating the high bits.
    if (gamepad->hasAnalogTriggers)
    {
        newInputReport.leftTrigger = gamepad->pressedL2() ? 0x03FF : ((gamepad->state.lt << 2) | (gamepad->state.lt >> 6));
        newInputReport.rightTrigger = gamepad->pressedR2() ? 0x03FF : ((gamepad->state.rt << 2) | (gamepad->state.rt >> 6));
    }
    else
    {
        newInputReport.leftTrigger = gamepad->pressedL2() ? 0x03FF : 0;
        newInputReport.rightTrigger = gamepad->pressedR2() ? 0x03FF : 0;
    }

    // We changed inputs since generating our last report, increment last report counter (but don't update until success)
    if ( !input_report_sent || memcmp(&last_report[4], &((uint8_t*)&newInputReport)[4], sizeof(XboxOneGamepad_Data_t)-4) != 0 ) {
        xboneReportSize = sizeof(XboxOneGamepad_Data_t);
        memcpy(&xboneReport, &newInputReport, xboneReportSize);
        xboneReport.Header.sequence = last_report_counter + 1;
        if ( xboneReport.Header.sequence == 0 )
            xboneReport.Header.sequence = 1;

        // Successfully sent report, actually increment last report counter!
        if ( send_xbone_usb((uint8_t*)&xboneReport, xboneReportSize) == true ) {
            last_report_counter = xboneReport.Header.sequence;
            memcpy(last_report, &xboneReport, xboneReportSize);
            input_report_sent = true;
            return true;
        }
    }
    
    return false;
}

void XBOneDriver::processAux() {
    if ( authDriver != nullptr && authDriver->available() ) {
        ((XBOneAuth*)authDriver)->process();
    }
}

bool XBOneDriver::getAuthSent() {
    if ( xboxOneAuthData == nullptr ) {
        return false;
    }
    return xboxOneAuthData->authCompleted;
}

bool XBOneDriver::send_xbone_usb(uint8_t const *report, uint16_t report_size) {
    uint8_t itf = 0;
    xboned_interface_t *p_xbone = _xboned_itf;
    for (;; itf++, p_xbone++) {
        if (itf >= TU_ARRAY_SIZE(_xboned_itf)) {
            return false;
        }
        if (p_xbone->ep_in)
            break;
    }
    if (report_size > sizeof(p_xbone->epin_buf) || !tud_ready() ||
        !usbd_edpt_claim(TUD_OPT_RHPORT, p_xbone->ep_in)) return false;
    memcpy(p_xbone->epin_buf, report, report_size);
    if (usbd_edpt_xfer(TUD_OPT_RHPORT, p_xbone->ep_in, p_xbone->epin_buf, report_size, false)) return true;
    usbd_edpt_release(TUD_OPT_RHPORT, p_xbone->ep_in);
    return false;
}

// tud_hid_get_report_cb
uint16_t XBOneDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    const uint16_t len = TU_MIN(reqlen, sizeof(xboneReport));
    memcpy(buffer, &xboneReport, len);
    return len;
}

// Only PS4 does anything with set report
void XBOneDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool XBOneDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return xbone_vendor_control_xfer_cb(rhport, stage, request);
}

const uint16_t * XBOneDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    const char *value = (const char *)xbone_get_string_descriptor(index);
    return value ? getStringDescriptor(value, index) : nullptr; // getStringDescriptor returns a static array
}

const uint8_t * XBOneDriver::get_descriptor_device_cb() {
    return xbone_device_descriptor;
}

const uint8_t * XBOneDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return nullptr;
}

const uint8_t * XBOneDriver::get_descriptor_configuration_cb(uint8_t index) {
    return xbone_configuration_descriptor;
}

const uint8_t * XBOneDriver::get_descriptor_device_qualifier_cb() {
    return xbone_device_qualifier;
}

void XBOneDriver::set_ack_wait() {
    waiting_ack = true;
    waiting_ack_timeout = to_ms_since_boot(get_absolute_time()); // 2 second time-out
}

void XBOneDriver::update() {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Process our report queue
    process_report_queue(now);

    // Late plug-in: soft reconnect so the console restarts authentication
    static uint32_t reconnectAt = 0;
    if ( xboxOneAuthData != nullptr && xboxOneAuthData->reconnectRequested ) {
        xboxOneAuthData->reconnectRequested = false;
        xboxOneAuthData->consoleAuthOrphaned = false;
        xboxOneAuthData->consoleBuffer.reset();
        xboxOneAuthData->dongleBuffer.reset();
        xboxOneAuthData->xboneState = GPAuthState::auth_idle_state;
        xboxOneAuthData->auth_passthrough = false;
        xboneDriverState = NOT_READY;
        waiting_ack = false;
        tud_disconnect();
        reconnectAt = now + 250;
    }
    if ( reconnectAt != 0 && (int32_t)(now - reconnectAt) >= 0 ) {
        reconnectAt = 0;
        tud_connect();
    }

    if ( xboxOneAuthData != nullptr && authDriver != nullptr && authDriver->available() ) {
        ((XBOneAuth*)authDriver)->processHost();
        XBOneRelayPacket packet;
        while ( queue_try_remove(&xboxOneAuthData->relayToConsole, &packet) ) {
            queue_xbone_report(packet.data, packet.len);
        }
    }

    // Do not add logic until our ACK returns
    if ( waiting_ack == true ) {
        if ((now - waiting_ack_timeout) < XGIP_ACK_WAIT_TIMEOUT) {
            return;
        } else { // ACK wait time out
            waiting_ack = false;
        }
    }

    switch(xboneDriverState) {
        case READY_ANNOUNCE:
            // Xbox One announce must wait around 0.5s before sending
            if ( now - timer_wait_for_announce > 500 ) {
                // Stable per-board Device ID; do not replace it with a shared boot time.
                outgoingXGIP->setAttributes(GIP_ANNOUNCE, 1, 1, 0, 0);
                outgoingXGIP->setData(announcePacket, sizeof(announcePacket));
                queue_xbone_report(outgoingXGIP->generatePacket(), outgoingXGIP->getPacketLength());
                xboneDriverState = WAIT_DESCRIPTOR_REQUEST;
            }
            break;
        case SEND_DESCRIPTOR:
            queue_xbone_report(outgoingXGIP->generatePacket(), outgoingXGIP->getPacketLength());
            if ( outgoingXGIP->endOfChunk() == true ) {
                xboneDriverState = SETUP_AUTH;
            }
            if ( outgoingXGIP->getPacketAck() == 1 ) { // ACK can happen at different chunks
                set_ack_wait();
            }
            break;
        case SETUP_AUTH:
            // Received packet from dongle to console / PC
            if ( xboxOneAuthData->xboneState == GPAuthState::send_auth_dongle_to_console ) {
                uint16_t len = xboxOneAuthData->dongleBuffer.length;
                uint8_t type = xboxOneAuthData->dongleBuffer.type;
                uint8_t sequence = xboxOneAuthData->dongleBuffer.sequence;
                uint8_t * buffer = xboxOneAuthData->dongleBuffer.data;
                bool isChunked = (len > GIP_MAX_CHUNK_SIZE);
                outgoingXGIP->reset();
                outgoingXGIP->setAttributes(type, sequence, 1, isChunked, 1);
                outgoingXGIP->setData(buffer, len);
                xboxOneAuthData->xboneState = wait_auth_dongle_to_console;
                xboxOneAuthData->dongleBuffer.reset();
            }

            if ( xboxOneAuthData->xboneState == GPAuthState::wait_auth_dongle_to_console ) {
                queue_xbone_report(outgoingXGIP->generatePacket(), outgoingXGIP->getPacketLength());
                if ( outgoingXGIP->getChunked() == false || outgoingXGIP->endOfChunk() == true ) {
                    xboxOneAuthData->xboneState = GPAuthState::auth_idle_state;
                }
                if ( outgoingXGIP->getPacketAck() == 1 ) {
                    set_ack_wait();
                }
            }
            break;
        case AUTH_DONE:
        case NOT_READY:
        default:
            break;
    };
}

void XBOneDriver::process_report_queue(uint32_t now) {
    if ( !report_queue.empty() &&
        ((xboxOneAuthData != nullptr && xboxOneAuthData->auth_passthrough) ||
         (now - lastReportQueue) > REPORT_QUEUE_INTERVAL) ) {
        if ( send_xbone_usb(report_queue.front().report, report_queue.front().len) ) {
            report_queue.pop();
            lastReportQueue = now;
        } // Retry on the next update if USB is busy, keep the input loop running
    }
}

uint16_t XBOneDriver::GetJoystickMidValue() {
    return GAMEPAD_JOYSTICK_MID;
}

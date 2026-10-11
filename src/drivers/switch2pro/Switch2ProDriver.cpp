/*
 * SPDX-License-Identifier: MIT
 * Nintendo Switch 2 Pro Controller (057E:2069) wired USB emulation.
 */

#include "drivers/switch2pro/Switch2ProDriver.h"
#include "storagemanager.h"
#include "addons/amiibo.h"
#include "mbedtls/aes.h"
#include "pico/rand.h"

#include <string.h>

#define CMD_DIRECTION_REQUEST 0x91
#define CMD_DIRECTION_REPLY   0x01
#define CMD_TRANSPORT_USB     0x00
#define CMD_ACK               0xF8
#define CMD_HEADER_LEN        8
#define CMD_BUFFER_LEN        64
#define CMD_REPLY_MAX         640

#define VENDOR_REQ_DEVICE_INFO  0x02
#define VENDOR_REQ_FACTORY_DATA 0x03
#define VENDOR_REQ_SET_MAX_LEN  0x04


typedef enum {
    CMD_NFC          = 0x01,
    CMD_FLASH_MEMORY = 0x02,
    CMD_INIT         = 0x03,
    CMD_UNKNOWN07    = 0x07,
    CMD_CHARGE_GRIP  = 0x08,
    CMD_PLAYER_LED   = 0x09,
    CMD_VIBRATION    = 0x0A,
    CMD_BATTERY      = 0x0B,
    CMD_FEATURE      = 0x0C,
    CMD_FW_INFO      = 0x10,
    CMD_UNKNOWN11    = 0x11,
    CMD_PAIRING      = 0x15,
    CMD_UNKNOWN16    = 0x16,
    CMD_UNKNOWN18    = 0x18,
} Switch2ProCommand;

static uint8_t cmdRequest[CMD_BUFFER_LEN];
static uint8_t cmdReply[CMD_REPLY_MAX];
static uint8_t nfcCounter;
static uint8_t nfcState;
static bool nfcRead;
static uint16_t cmdReplyLen;
static bool cmdReplyPending;
static bool reportsEnabled;
static uint8_t playerLedMask;
static uint8_t btLongTermKey[16];
static uint8_t hostAddress[6];


static uint8_t deviceDescriptor[sizeof(switch2pro_device_descriptor)];
static uint8_t factoryData[sizeof(switch2pro_factory_data)];
static uint8_t deviceInfo[sizeof(switch2pro_device_info)];
static uint8_t btAddressReversed[sizeof(switch2pro_bt_address_reversed)];
static uint8_t deviceKey[sizeof(switch2pro_device_key)];
static uint8_t chargingGripData[sizeof(switch2pro_charging_grip_data)];
static const char *productString;
static uint8_t firmwareType;

static uint8_t otherSpeedConfiguration[sizeof(switch2pro_configuration_descriptor)];
static uint16_t stringDescriptor[64];

static void resetProtocolState() {
    cmdReplyLen = 0;
    cmdReplyPending = false;
    reportsEnabled = false;
    playerLedMask = 0;
    nfcCounter = 0;
    nfcState = 0x09;
    nfcRead = false;
}

static void setReply(uint8_t command, uint8_t subcommand, const uint8_t *payload, uint16_t payloadLen) {
    if (payloadLen > CMD_REPLY_MAX - CMD_HEADER_LEN)
        payloadLen = CMD_REPLY_MAX - CMD_HEADER_LEN;
    memset(cmdReply, 0, CMD_HEADER_LEN);
    cmdReply[0] = command;
    cmdReply[1] = CMD_DIRECTION_REPLY;
    cmdReply[2] = CMD_TRANSPORT_USB;
    cmdReply[3] = subcommand;
    cmdReply[5] = CMD_ACK;
    if (payload != nullptr && payloadLen > 0)
        memcpy(&cmdReply[CMD_HEADER_LEN], payload, payloadLen);
    cmdReplyLen = CMD_HEADER_LEN + payloadLen;
}

static void overlayFlash(uint32_t address, uint8_t *out, uint16_t length, uint32_t blockAddress, const uint8_t *block, uint16_t blockLen) {
    for (uint16_t i = 0; i < length; i++) {
        uint32_t a = address + i;
        if (a >= blockAddress && a < blockAddress + blockLen)
            out[i] = block[a - blockAddress];
    }
}

static void replyFlashRead(const uint8_t *req) {
    uint8_t length = req[8];
    if (length > 0x40)
        length = 0x40;
    uint32_t address = req[12] | (req[13] << 8) | (req[14] << 16) | ((uint32_t)req[15] << 24);

    uint8_t payload[8 + 0x40];
    memset(payload, 0x00, 8);
    payload[0] = length;
    memcpy(&payload[4], &req[12], 4);
    uint8_t *data = &payload[8];
    memset(data, 0xFF, length);
    overlayFlash(address, data, length, 0x13000, factoryData, sizeof(factoryData));
    overlayFlash(address, data, length, 0x13040, switch2pro_flash_13040, sizeof(switch2pro_flash_13040));
    overlayFlash(address, data, length, 0x13080, switch2pro_flash_13080, sizeof(switch2pro_flash_13080));
    overlayFlash(address, data, length, 0x130C0, switch2pro_flash_130c0, sizeof(switch2pro_flash_130c0));
    overlayFlash(address, data, length, 0x13100, switch2pro_flash_13100, sizeof(switch2pro_flash_13100));
    setReply(CMD_FLASH_MEMORY, req[3], payload, 8 + length);
}

static void replyPairing(const uint8_t *req) {
    switch (req[3]) {
        case 0x01: {
            uint8_t payload[9] = { 0x01, 0x04, 0x01 };
            memcpy(&payload[3], btAddressReversed, 6);
            setReply(CMD_PAIRING, req[3], payload, sizeof(payload));
            break;
        }
        case 0x02: {
            uint8_t challenge[16];
            for (int i = 0; i < 16; i++)
                challenge[i] = req[24 - i];
            uint8_t payload[17] = { 0x01 };
            mbedtls_aes_context aes;
            mbedtls_aes_init(&aes);
            mbedtls_aes_setkey_enc(&aes, btLongTermKey, 128);
            mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, challenge, &payload[1]);
            mbedtls_aes_free(&aes);
            setReply(CMD_PAIRING, req[3], payload, sizeof(payload));
            break;
        }
        case 0x03: {
            uint8_t payload[1] = { 0x01 };
            setReply(CMD_PAIRING, req[3], payload, sizeof(payload));
            break;
        }
        case 0x04: {
            for (int i = 0; i < 16; i++)
                btLongTermKey[i] = req[24 - i] ^ deviceKey[i];
            uint8_t payload[17] = { 0x01 };
            for (int i = 0; i < 16; i++)
                payload[1 + i] = deviceKey[15 - i];
            setReply(CMD_PAIRING, req[3], payload, sizeof(payload));
            break;
        }
        default:
            setReply(CMD_PAIRING, req[3], nullptr, 0);
            break;
    }
}

static void copyTagUid(uint8_t *out, const uint8_t *tag) {
    memcpy(out, tag, 3);
    memcpy(out + 3, tag + 4, 4);
}

static void replyNfc(const uint8_t *req) {
    const uint8_t sub = req[3];
    const uint8_t *tag = AmiiboAddon::tagPresent() ? AmiiboAddon::tagData() : nullptr;
    switch (sub) {
        case 0x0C: {
            const uint8_t payload[] = { 0x61, 0x12, 0x50, 0x10 };
            setReply(CMD_NFC, sub, payload, sizeof(payload));
            break;
        }
        case 0x05: {
            uint8_t payload[61] = { 0x09, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x00, 0x07 };
            if (tag != nullptr) {
                payload[0] = nfcState;
                copyTagUid(&payload[9], tag);
            }
            setReply(CMD_NFC, sub, payload, sizeof(payload));
            break;
        }
        case 0x15: {
            if (tag == nullptr) {
                setReply(CMD_NFC, sub, nullptr, 0);
                break;
            }
            static const uint8_t header[] = { 0x01, 0x58, 0x02, 0x04, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x07 };
            static const uint8_t footer[] = { 0x03, 0x00, 0x3B, 0x3C, 0x77, 0x78, 0x86, 0x00, 0x00 };
            static uint8_t payload[sizeof(header) + 7 + 4 + AMIIBO_SIGNATURE_SIZE + sizeof(footer) + AMIIBO_DATA_SIZE + 19];
            memset(payload, 0, sizeof(payload));
            uint8_t *p = payload;
            memcpy(p, header, sizeof(header)); p += sizeof(header);
            copyTagUid(p, tag); p += 7;
            p += 4;
            const uint8_t *signature = AmiiboAddon::tagSignature();
            if (signature != nullptr)
                memcpy(p, signature, AMIIBO_SIGNATURE_SIZE);
            p += AMIIBO_SIGNATURE_SIZE;
            memcpy(p, footer, sizeof(footer)); p += sizeof(footer);
            memcpy(p, tag, AMIIBO_DATA_SIZE);
            setReply(CMD_NFC, sub, payload, sizeof(payload));
            nfcRead = true;
            break;
        }
        case 0x03:
            if (tag != nullptr && req[5] == 0x05 && req[11] == 0x2C && req[12] == 0x01)
                nfcCounter = (nfcCounter + 1) & 0x07;
            if (nfcRead) {
                nfcRead = false;
                nfcState = 0x09;
            }
            setReply(CMD_NFC, sub, nullptr, 0);
            break;
        case 0x06:
            if (tag != nullptr)
                nfcCounter = (nfcCounter + 1) & 0x07;
            nfcState = 0x04;
            setReply(CMD_NFC, sub, nullptr, 0);
            break;
        default:
            setReply(CMD_NFC, sub, nullptr, 0);
            break;
    }
}

static void handleCommand(const uint8_t *req) {
    const uint8_t command = req[0];
    const uint8_t sub = req[3];

    switch (command) {
        case CMD_NFC:
            replyNfc(req);
            break;
        case CMD_FLASH_MEMORY:
            if (sub == 0x01 || sub == 0x04) {
                replyFlashRead(req);
            } else if (sub == 0x02) {
                setReply(command, sub, &req[8], 8);
            } else if (sub == 0x03) {
                const uint8_t payload[4] = { };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x05) {
                uint8_t payload[8] = { };
                memcpy(&payload[4], &req[12], 4);
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_INIT:
            if (sub == 0x07) {
                for (int i = 0; i < 6; i++)
                    hostAddress[i] = req[13 - i];
                for (int i = 0; i < 16; i++)
                    btLongTermKey[i] = req[29 - i];
                setReply(command, sub, nullptr, 0);
            } else if (sub == 0x0D) {
                reportsEnabled = true;
                for (int i = 0; i < 6; i++)
                    hostAddress[i] = req[15 - i];
                const uint8_t payload[] = { 0x01, 0x00, 0x00, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x0F) {
                const uint8_t payload[] = { 0x05, 0x00, 0x00, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_UNKNOWN07:
            if (sub == 0x01) {
                const uint8_t payload[] = { 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_CHARGE_GRIP:
            if (sub == 0x01)
                setReply(command, sub, chargingGripData, 36);
            else if (sub == 0x03)
                setReply(command, sub, chargingGripData, 68);
            else
                setReply(command, sub, nullptr, 0);
            break;
        case CMD_PLAYER_LED:
            if (sub == 0x07)
                playerLedMask = req[8];
            setReply(command, sub, nullptr, 0);
            break;
        case CMD_VIBRATION:
            if (sub == 0x02) {
                const uint8_t payload[] = { req[8], 0x00, 0x00, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_BATTERY:
            if (sub == 0x03) {
                const uint8_t payload[] = { 0xA5, 0x0E, 0x00, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x04) {
                const uint8_t payload[] = { 0x34, 0x00, 0x83, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x06) {
                const uint8_t payload[] = { 0x11, 0x00, 0x00, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_FEATURE:
            if (sub == 0x01) {
                uint8_t payload[12] = { };
                if (req[8] & 0x01) payload[4] = 0x07;
                if (req[8] & 0x02) payload[5] = 0x07;
                if (req[8] & 0x04) payload[6] = 0x01;
                if (req[8] & 0x80) payload[7] = 0x01;
                if (req[8] & 0x10) payload[8] = 0x01;
                if (req[8] & 0x20) payload[9] = 0x03;
                setReply(command, sub, payload, sizeof(payload));
            } else {
                if (sub == 0x02 && (req[8] & 0x01) && (req[8] & 0x02))
                    reportsEnabled = true;
                else if (sub == 0x03)
                    reportsEnabled = false;
                const uint8_t payload[4] = { };
                if (sub >= 0x02 && sub <= 0x05)
                    setReply(command, sub, payload, sizeof(payload));
                else
                    setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_FW_INFO:
            if (sub == 0x01) {
                const uint8_t payload[] = { 0x02, 0x01, 0x04, firmwareType, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_UNKNOWN11:
            if (sub == 0x01) {
                const uint8_t payload[] = { 0x01, 0x00, 0x00, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x03) {
                const uint8_t payload[] = {
                    0x01, 0x20, 0x03, 0x00, 0x00, 0x0a, 0xe8, 0x1c, 0x3b, 0x79, 0x7d, 0x8b, 0x3a, 0x0a, 0xe8,
                    0x9c, 0x42, 0x58, 0xa0, 0x0b, 0x42, 0x0a, 0xe8, 0x9c, 0x41, 0x58, 0xa0, 0x0b, 0x41 };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_PAIRING:
            replyPairing(req);
            break;
        case CMD_UNKNOWN16:
            if (sub == 0x01) {
                uint8_t payload[0x18] = { };
                payload[12] = 0x7C;
                payload[13] = 0x06;
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_UNKNOWN18:
            if (sub == 0x01) {
                const uint8_t payload[] = { 0x00, 0x00, 0x40, 0xF0, 0x00, 0x00, 0x60, 0x00 };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x03) {
                const uint8_t payload[] = { req[8] };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        default:
            setReply(command, sub, nullptr, 0);
            break;
    }
}

static void sendReply(uint8_t rhport) {
    if (!cmdReplyPending || usbd_edpt_busy(rhport, SWITCH2_PRO_EP_BULK_IN))
        return;
    cmdReplyPending = false;
    usbd_edpt_xfer(rhport, SWITCH2_PRO_EP_BULK_IN, cmdReply, cmdReplyLen, false);
}

static void switch2pro_init(void) {
    hidd_init();
    resetProtocolState();
}

static bool switch2pro_deinit(void) {
    return hidd_deinit();
}

static void switch2pro_reset(uint8_t rhport) {
    hidd_reset(rhport);
    resetProtocolState();
}

static uint16_t switch2pro_open(uint8_t rhport, tusb_desc_interface_t const *itf_desc, uint16_t max_len) {
    const uint8_t itf = itf_desc->bInterfaceNumber;
    if (itf == SWITCH2_PRO_ITF_HID && itf_desc->bInterfaceClass == TUSB_CLASS_HID)
        return hidd_open(rhport, itf_desc, max_len);

    if (itf != SWITCH2_PRO_ITF_VENDOR || itf_desc->bInterfaceClass != TUSB_CLASS_VENDOR_SPECIFIC)
        return 0;

    uint8_t const *p = (uint8_t const *)itf_desc;
    uint8_t const *end = p + max_len;
    uint16_t len = 0;
    while (p < end) {
        const uint8_t type = tu_desc_type(p);
        if (len > 0 && type == TUSB_DESC_INTERFACE_ASSOCIATION)
            break;
        if (type == TUSB_DESC_ENDPOINT) {
            tusb_desc_endpoint_t const *ep = (tusb_desc_endpoint_t const *)p;
            TU_ASSERT(usbd_edpt_open(rhport, ep), 0);
            if (ep->bEndpointAddress == SWITCH2_PRO_EP_BULK_OUT)
                usbd_edpt_xfer(rhport, SWITCH2_PRO_EP_BULK_OUT, cmdRequest, sizeof(cmdRequest), false);
        }
        len += tu_desc_len(p);
        p = tu_desc_next(p);
    }
    return len;
}

static bool switch2pro_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    if (request->bmRequestType_bit.recipient != TUSB_REQ_RCPT_INTERFACE)
        return false;
    const uint8_t itf = tu_u16_low(request->wIndex);
    if (itf == SWITCH2_PRO_ITF_HID)
        return hidd_control_xfer_cb(rhport, stage, request);
    return false;
}

static bool switch2pro_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
    switch (ep_addr) {
        case SWITCH2_PRO_EP_BULK_OUT:
            if (result == XFER_RESULT_SUCCESS && xferred_bytes >= 4 &&
                    cmdRequest[1] == CMD_DIRECTION_REQUEST && cmdRequest[2] == CMD_TRANSPORT_USB) {
                if (xferred_bytes < sizeof(cmdRequest))
                    memset(&cmdRequest[xferred_bytes], 0, sizeof(cmdRequest) - xferred_bytes);
                handleCommand(cmdRequest);
                cmdReplyPending = true;
                sendReply(rhport);
            }
            usbd_edpt_xfer(rhport, SWITCH2_PRO_EP_BULK_OUT, cmdRequest, sizeof(cmdRequest), false);
            return true;
        case SWITCH2_PRO_EP_BULK_IN:
            sendReply(rhport);
            return true;
        default:
            return hidd_xfer_cb(rhport, ep_addr, result, xferred_bytes);
    }
}

static void setFactoryColor(uint8_t offset, uint32_t rgb) {
    factoryData[offset] = (rgb >> 16) & 0xFF;
    factoryData[offset + 1] = (rgb >> 8) & 0xFF;
    factoryData[offset + 2] = rgb & 0xFF;
}

static void setProductId(uint8_t *data, uint16_t pid) {
    data[0] = pid & 0xFF;
    data[1] = pid >> 8;
}

static void loadIdentity() {
    memcpy(deviceDescriptor, switch2pro_device_descriptor, sizeof(deviceDescriptor));
    memcpy(factoryData, switch2pro_factory_data, sizeof(factoryData));
    memcpy(chargingGripData, switch2pro_charging_grip_data, sizeof(chargingGripData));

    memcpy(btAddressReversed, switch2pro_bt_address_reversed, sizeof(btAddressReversed));
    for (int i = 0; i < SWITCH2_PRO_BT_RANDOM_BYTES; i++)
        btAddressReversed[i] = get_rand_32() & 0xFF;
    memcpy(deviceInfo, switch2pro_device_info, sizeof(deviceInfo));
    memcpy(&deviceInfo[SWITCH2_PRO_DEVICE_INFO_BT_OFFSET], btAddressReversed, sizeof(btAddressReversed));
    for (size_t i = 0; i < sizeof(deviceKey); i++)
        deviceKey[i] = get_rand_32() & 0xFF;
    for (int i = 0; i < SWITCH2_PRO_FACTORY_SERIAL_COUNT; i++)
        factoryData[SWITCH2_PRO_FACTORY_SERIAL_DIGITS + i] = '0' + (get_rand_32() % 10);
    productString = switch2pro_strings[SWITCH2_PRO_STRING_PRODUCT];
    firmwareType = SWITCH2_FW_TYPE_PRO;

    const GamepadOptions & options = Storage::getInstance().getGamepadOptions();
    if (options.switch2ProIdentity == SWITCH2_PRO_IDENTITY_GAMECUBE) {
        setProductId(&deviceDescriptor[SWITCH2_PRO_DEVICE_PID_OFFSET], SWITCH2_GAMECUBE_PRODUCT_ID);
        setProductId(&factoryData[SWITCH2_PRO_FACTORY_PID_OFFSET], SWITCH2_GAMECUBE_PRODUCT_ID);
        setProductId(&chargingGripData[SWITCH2_PRO_GRIP_PID_OFFSET], SWITCH2_GAMECUBE_PRODUCT_ID);
        productString = switch2gamecube_product_string;
        firmwareType = SWITCH2_FW_TYPE_GAMECUBE;
        return;
    }

    uint32_t body, buttons, highlight, grip;
    switch (options.switch2ProColorPreset) {
        case SWITCH2_PRO_COLOR_GP2040:
            body = 0x1B1B1D;
            buttons = 0xFFFFFF;
            highlight = 0x00FF00;
            grip = 0xEC008C;
            break;
        case SWITCH2_PRO_COLOR_CUSTOM:
            body = options.switch2ProBodyColor;
            buttons = options.switch2ProButtonsColor;
            highlight = options.switch2ProHighlightColor;
            grip = options.switch2ProGripColor;
            break;
        default:
            return;
    }
    setFactoryColor(SWITCH2_PRO_FACTORY_BODY_COLOR, body);
    setFactoryColor(SWITCH2_PRO_FACTORY_BUTTONS_COLOR, buttons);
    setFactoryColor(SWITCH2_PRO_FACTORY_HIGHLIGHT_COLOR, highlight);
    setFactoryColor(SWITCH2_PRO_FACTORY_GRIP_COLOR, grip);
}

void Switch2ProDriver::initialize() {
    memset(inputReport, 0, sizeof(inputReport));
    reportCounter = 0;
    loadIdentity();

    class_driver = {
    #if CFG_TUSB_DEBUG >= 2
        .name = "SWITCH2PRO",
    #endif
        .init = switch2pro_init,
        .deinit = switch2pro_deinit,
        .reset = switch2pro_reset,
        .open = switch2pro_open,
        .control_xfer_cb = switch2pro_control_xfer_cb,
        .xfer_cb = switch2pro_xfer_cb,
        .sof = NULL
    };
}

static void packStick(uint8_t *out, uint16_t x16, uint16_t y16) {
    const uint16_t x = x16 >> 4;
    const uint16_t y = SWITCH2_PRO_STICK_MAX - (y16 >> 4);
    out[0] = x & 0xFF;
    out[1] = ((x >> 8) & 0x0F) | ((y & 0x0F) << 4);
    out[2] = (y >> 4) & 0xFF;
}

bool Switch2ProDriver::process(Gamepad * gamepad) {
    static bool tagWasPresent = false;
    static uint32_t lastTapGeneration = 0;
    const bool tagIsPresent = AmiiboAddon::tagPresent();
    const uint32_t tapGeneration = AmiiboAddon::tapGeneration();
    if (tagIsPresent && (!tagWasPresent || tapGeneration != lastTapGeneration)) {
        nfcCounter = (nfcCounter + 1) & 0x07;
        nfcState = 0x09;
    }
    tagWasPresent = tagIsPresent;
    lastTapGeneration = tapGeneration;
    Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
    processedGamepad->auxState.playerID.active = reportsEnabled;
    processedGamepad->auxState.playerID.ledValue = playerLedMask;
    processedGamepad->auxState.playerID.value = playerLedMask;

    if (tud_suspended())
        tud_remote_wakeup();

    if (!reportsEnabled)
        return false;

    uint8_t b0 = 0, b1 = 0, b2 = 0;
    if (gamepad->pressedB1()) b0 |= SWITCH2_PRO_MASK_B;
    if (gamepad->pressedB2()) b0 |= SWITCH2_PRO_MASK_A;
    if (gamepad->pressedB3()) b0 |= SWITCH2_PRO_MASK_Y;
    if (gamepad->pressedB4()) b0 |= SWITCH2_PRO_MASK_X;
    if (gamepad->pressedR1()) b0 |= SWITCH2_PRO_MASK_R;
    if (gamepad->pressedR2()) b0 |= SWITCH2_PRO_MASK_ZR;
    if (gamepad->pressedS2()) b0 |= SWITCH2_PRO_MASK_PLUS;
    if (gamepad->pressedR3()) b0 |= SWITCH2_PRO_MASK_RSTICK;

    if (gamepad->pressedDown())  b1 |= SWITCH2_PRO_MASK_DOWN;
    if (gamepad->pressedRight()) b1 |= SWITCH2_PRO_MASK_RIGHT;
    if (gamepad->pressedLeft())  b1 |= SWITCH2_PRO_MASK_LEFT;
    if (gamepad->pressedUp())    b1 |= SWITCH2_PRO_MASK_UP;
    if (gamepad->pressedL1())    b1 |= SWITCH2_PRO_MASK_L;
    if (gamepad->pressedL2())    b1 |= SWITCH2_PRO_MASK_ZL;
    if (gamepad->pressedS1())    b1 |= SWITCH2_PRO_MASK_MINUS;
    if (gamepad->pressedL3())    b1 |= SWITCH2_PRO_MASK_LSTICK;

    if (gamepad->pressedA1()) b2 |= SWITCH2_PRO_MASK_HOME;
    if (gamepad->pressedA2()) b2 |= SWITCH2_PRO_MASK_CAPTURE;
    if (gamepad->pressedA3()) b2 |= SWITCH2_PRO_MASK_C;
    if (gamepad->pressedE1()) b2 |= SWITCH2_PRO_MASK_GL;
    if (gamepad->pressedE2()) b2 |= SWITCH2_PRO_MASK_GR;
    inputReport[1] = 0x23;
    inputReport[2] = b0;
    inputReport[3] = b1;
    inputReport[4] = b2;
    packStick(&inputReport[5], gamepad->state.lx, gamepad->state.ly);
    packStick(&inputReport[8], gamepad->state.rx, gamepad->state.ry);
    inputReport[11] = 0x38;
    inputReport[12] = nfcCounter;
    const uint32_t now = to_ms_since_boot(get_absolute_time());
    const bool changed = memcmp(&inputReport[2], &lastSentState[0], sizeof(lastSentState)) != 0;
    if (!changed && (now - lastSentMs) < SWITCH2_PRO_KEEPALIVE_MS)
        return false;
    if (!tud_hid_n_ready(0))
        return false;

    inputReport[0] = reportCounter;
    if (!tud_hid_n_report(0, SWITCH2_PRO_INPUT_REPORT_ID, inputReport, sizeof(inputReport)))
        return false;
    memcpy(lastSentState, &inputReport[2], sizeof(lastSentState));
    lastSentMs = now;
    reportCounter++;
    return true;
}

uint16_t Switch2ProDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    return 0;
}

void Switch2ProDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
}

bool Switch2ProDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    if (stage != CONTROL_STAGE_SETUP)
        return true;

    if (request->bmRequestType_bit.direction == TUSB_DIR_IN) {
        static uint8_t factory[64];
        switch (request->bRequest) {
            case VENDOR_REQ_DEVICE_INFO:
                return tud_control_xfer(rhport, request, deviceInfo, sizeof(deviceInfo));
            case VENDOR_REQ_FACTORY_DATA:
                memset(factory, 0xFF, sizeof(factory));
                memcpy(factory, factoryData, sizeof(factoryData));
                return tud_control_xfer(rhport, request, factory, sizeof(factory));
            default:
                return false;
        }
    }

    if (request->bRequest == VENDOR_REQ_SET_MAX_LEN) {
        static uint8_t sink[64];
        if (request->wLength == 0)
            return tud_control_status(rhport, request);
        return tud_control_xfer(rhport, request, sink, request->wLength < sizeof(sink) ? request->wLength : sizeof(sink));
    }
    return false;
}

const uint16_t * Switch2ProDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    uint8_t count = 0;
    if (index == 0) {
        for (uint16_t lang : switch2pro_language_ids)
            stringDescriptor[1 + count++] = lang;
    } else if (index == 0xEE) {
        const char *signature = "MSFT100";
        for (; signature[count] != '\0'; count++)
            stringDescriptor[1 + count] = signature[count];
        stringDescriptor[1 + count++] = 0x01;
    } else if (index < TU_ARRAY_SIZE(switch2pro_strings)) {
        const char *value = (index == SWITCH2_PRO_STRING_PRODUCT) ? productString : switch2pro_strings[index];
        for (; value[count] != '\0' && count < TU_ARRAY_SIZE(stringDescriptor) - 1; count++)
            stringDescriptor[1 + count] = value[count];
    } else {
        return nullptr;
    }
    stringDescriptor[0] = (TUSB_DESC_STRING << 8) | (2 * count + 2);
    return stringDescriptor;
}

const uint8_t * Switch2ProDriver::get_descriptor_device_cb() {
    return deviceDescriptor;
}

const uint8_t * Switch2ProDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return switch2pro_report_descriptor;
}

const uint8_t * Switch2ProDriver::get_descriptor_configuration_cb(uint8_t index) {
    return switch2pro_configuration_descriptor;
}

const uint8_t * Switch2ProDriver::get_descriptor_device_qualifier_cb() {
    return switch2pro_device_qualifier;
}

const uint8_t * Switch2ProDriver::get_descriptor_other_speed_configuration_cb(uint8_t index) {
    memcpy(otherSpeedConfiguration, switch2pro_configuration_descriptor, sizeof(otherSpeedConfiguration));
    otherSpeedConfiguration[1] = TUSB_DESC_OTHER_SPEED_CONFIG;
    for (size_t offset = 0; offset < sizeof(otherSpeedConfiguration);) {
        const uint8_t length = otherSpeedConfiguration[offset];
        if (length == 0)
            break;
        if (otherSpeedConfiguration[offset + 1] == TUSB_DESC_ENDPOINT) {
            const uint8_t xfer = otherSpeedConfiguration[offset + 3] & 0x03;
            if (xfer == TUSB_XFER_INTERRUPT)
                otherSpeedConfiguration[offset + 6] = 6;
        }
        offset += length;
    }
    return otherSpeedConfiguration;
}

uint16_t Switch2ProDriver::GetJoystickMidValue() {
    return GAMEPAD_JOYSTICK_MID;
}

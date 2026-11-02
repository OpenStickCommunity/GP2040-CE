/*
 * SPDX-License-Identifier: MIT
 * Nintendo Switch 2 Pro Controller (057E:2069) wired USB emulation.
 */

#include "drivers/switch2pro/Switch2ProDriver.h"
#include "storagemanager.h"
#include "mbedtls/aes.h"
#include "pico/rand.h"

#include <string.h>
#include <cmath>

#define CMD_DIRECTION_REQUEST 0x91
#define CMD_DIRECTION_REPLY   0x01
#define CMD_TRANSPORT_USB     0x00
#define CMD_ACK               0xF8
#define CMD_HEADER_LEN        8
#define CMD_PACKET_LEN        64
#define CMD_BUFFER_LEN        160
#define CMD_REPLY_MAX         160

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
    CMD_FW_UPDATE    = 0x0D,
    CMD_FW_INFO      = 0x10,
    CMD_UNKNOWN11    = 0x11,
    CMD_UNKNOWN13    = 0x13,
    CMD_PAIRING      = 0x15,
    CMD_UNKNOWN16    = 0x16,
    CMD_UNKNOWN18    = 0x18,
} Switch2ProCommand;

static uint8_t cmdPacket[CMD_PACKET_LEN];
static uint8_t cmdRequest[CMD_BUFFER_LEN];
static uint16_t cmdRequestLen = 0;
static uint16_t cmdExpectedLen = 0;
static uint8_t cmdReply[CMD_REPLY_MAX];
static uint16_t cmdReplyLen;
static bool cmdReplyPending;
static bool reportsEnabled = true;
static uint8_t playerLedMask;
static uint8_t btLongTermKey[16];
static uint8_t hostAddress[6];
static uint8_t selectedReportId = 0;


static uint8_t deviceDescriptor[sizeof(switch2pro_device_descriptor)];
static uint8_t factoryData[sizeof(switch2pro_factory_data)];
static uint8_t deviceInfo[sizeof(switch2pro_device_info)];
static uint8_t btAddressReversed[sizeof(switch2pro_bt_address_reversed)];
static uint8_t deviceKey[sizeof(switch2pro_device_key)];
static uint8_t chargingGripData[sizeof(switch2pro_charging_grip_data)];
static uint8_t userCalibData[0x100];
static bool userCalibDirty = false;
static uint32_t lastCalibActivityMs = 0;
static const char *productString;
static uint8_t firmwareType;

static uint8_t otherSpeedConfiguration[sizeof(switch2pro_configuration_descriptor)];
static uint8_t configuration[sizeof(switch2pro_configuration_descriptor)];
static uint8_t joyCon2Side = 0;
static uint16_t stringDescriptor[64];

static void markUserCalibDirty() {
    userCalibDirty = true;
    lastCalibActivityMs = to_ms_since_boot(get_absolute_time());
}

static void resetProtocolState() {
    cmdRequestLen = 0;
    cmdExpectedLen = 0;
    cmdReplyLen = 0;
    cmdReplyPending = false;
    reportsEnabled = true;
    playerLedMask = 0;
    selectedReportId = 0;
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

static bool writeOverlayFlash(uint32_t address, const uint8_t *in, uint16_t length, uint32_t blockAddress, uint8_t *block, uint16_t blockLen) {
    bool modified = false;
    for (uint16_t i = 0; i < length; i++) {
        uint32_t a = address + i;
        if (a >= blockAddress && a < blockAddress + blockLen) {
            if (block[a - blockAddress] != in[i]) {
                block[a - blockAddress] = in[i];
                modified = true;
            }
        }
    }
    return modified;
}

static void replyFlashRead(const uint8_t *req) {
    uint8_t length = (req[3] == 0x01) ? 0x40 : req[8];
    if (length > 0x50)
        length = 0x50;
    uint32_t address = req[12] | (req[13] << 8) | (req[14] << 16) | ((uint32_t)req[15] << 24);

    uint8_t payload[8 + 0x50];
    memset(payload, 0x00, 8);
    payload[0] = length;
    memcpy(&payload[4], &req[12], 4);
    uint8_t *data = &payload[8];
    memset(data, 0xFF, length);
    overlayFlash(address, data, length, 0x13000, factoryData, sizeof(factoryData));
    overlayFlash(address, data, length, 0x1FC000, userCalibData, sizeof(userCalibData));
    if (joyCon2Side == 1) {
        overlayFlash(address, data, length, 0x13040, switch2_joycon2_left_flash_13040, sizeof(switch2_joycon2_left_flash_13040));
        overlayFlash(address, data, length, 0x13060, switch2_joycon2_left_flash_13060, sizeof(switch2_joycon2_left_flash_13060));
        overlayFlash(address, data, length, 0x13080, switch2_joycon2_left_flash_13080, sizeof(switch2_joycon2_left_flash_13080));
        overlayFlash(address, data, length, 0x13100, switch2_joycon2_left_flash_13100, sizeof(switch2_joycon2_left_flash_13100));
        overlayFlash(address, data, length, 0x13140, switch2_joycon2_left_flash_13140, sizeof(switch2_joycon2_left_flash_13140));
    } else if (joyCon2Side == 2) {
        overlayFlash(address, data, length, 0x13040, switch2_joycon2_right_flash_13040, sizeof(switch2_joycon2_right_flash_13040));
        overlayFlash(address, data, length, 0x13060, switch2_joycon2_right_flash_13060, sizeof(switch2_joycon2_right_flash_13060));
        overlayFlash(address, data, length, 0x13080, switch2_joycon2_right_flash_13080, sizeof(switch2_joycon2_right_flash_13080));
        overlayFlash(address, data, length, 0x13100, switch2_joycon2_right_flash_13100, sizeof(switch2_joycon2_right_flash_13100));
        overlayFlash(address, data, length, 0x13140, switch2_joycon2_right_flash_13140, sizeof(switch2_joycon2_right_flash_13140));
    } else {
        overlayFlash(address, data, length, 0x13040, switch2pro_flash_13040, sizeof(switch2pro_flash_13040));
        overlayFlash(address, data, length, 0x13080, switch2pro_flash_13080, sizeof(switch2pro_flash_13080));
        overlayFlash(address, data, length, 0x130C0, switch2pro_flash_130c0, sizeof(switch2pro_flash_130c0));
        overlayFlash(address, data, length, 0x13100, switch2pro_flash_13100, sizeof(switch2pro_flash_13100));
        if (firmwareType == SWITCH2_FW_TYPE_GAMECUBE) {
            static const uint8_t switch2gamecube_flash_13140[] = { 0x00, 0x00 };
            overlayFlash(address, data, length, 0x13140, switch2gamecube_flash_13140, sizeof(switch2gamecube_flash_13140));
        }
    }
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

static void handleCommand(const uint8_t *req) {
    const uint8_t command = req[0];
    const uint8_t sub = req[3];

    switch (command) {
        case CMD_NFC:
            if (sub == 0x0C) {
                const uint8_t payload[] = { 0x61, 0x12, 0x50, 0x10 };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x05) {
                uint8_t payload[61] = { 0x09, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x00, 0x07 };
                setReply(command, sub, payload, sizeof(payload));
            } else {
                setReply(command, sub, nullptr, 0);
            }
            break;
        case CMD_FLASH_MEMORY:
            if (sub == 0x01 || sub == 0x04) {
                if (userCalibDirty) {
                    lastCalibActivityMs = to_ms_since_boot(get_absolute_time());
                }
                replyFlashRead(req);
            } else if (sub == 0x02) {
                uint32_t address = req[12] | (req[13] << 8) | (req[14] << 16) | ((uint32_t)req[15] << 24);
                if (writeOverlayFlash(address, &req[16], 0x40, 0x1FC000, userCalibData, sizeof(userCalibData))) {
                    markUserCalibDirty();
                }
                setReply(command, sub, &req[8], 8);
            } else if (sub == 0x03) {
                uint32_t addr1 = req[12] | (req[13] << 8) | (req[14] << 16) | ((uint32_t)req[15] << 24);
                uint32_t addr2 = req[8]  | (req[9]  << 8) | (req[10] << 16) | ((uint32_t)req[11] << 24);
                if (((addr1 & 0xFFFFF000u) == 0x1FC000u) || ((addr2 & 0xFFFFF000u) == 0x1FC000u)) {
                    for (size_t i = 0; i < sizeof(userCalibData); i++) {
                        if (userCalibData[i] != 0xFF) {
                            userCalibData[i] = 0xFF;
                            markUserCalibDirty();
                        }
                    }
                }
                const uint8_t payload[4] = { };
                setReply(command, sub, payload, sizeof(payload));
            } else if (sub == 0x05) {
                uint8_t wlen = req[8];
                if (wlen > CMD_BUFFER_LEN - 16)
                    wlen = CMD_BUFFER_LEN - 16;
                uint32_t address = req[12] | (req[13] << 8) | (req[14] << 16) | ((uint32_t)req[15] << 24);
                if (writeOverlayFlash(address, &req[16], wlen, 0x1FC000, userCalibData, sizeof(userCalibData))) {
                    markUserCalibDirty();
                }
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
            } else if (sub == 0x0A) {
                selectedReportId = req[8];
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
        case CMD_UNKNOWN13:
            if (sub == 0x01) {
                const uint8_t payload[] = { 0x01, 0x00, 0x00, 0x00 };
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
                usbd_edpt_xfer(rhport, SWITCH2_PRO_EP_BULK_OUT, cmdPacket, sizeof(cmdPacket), false);
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
            // Ignore firmware updates so the console fails cleanly instead of hanging
            if (result == XFER_RESULT_SUCCESS && xferred_bytes > 0) {
                if (cmdRequestLen > 0) {
                    uint16_t copyLen = (uint16_t)xferred_bytes;
                    if (cmdRequestLen + copyLen > CMD_BUFFER_LEN)
                        copyLen = CMD_BUFFER_LEN - cmdRequestLen;
                    memcpy(&cmdRequest[cmdRequestLen], cmdPacket, copyLen);
                    cmdRequestLen += copyLen;
                    if (cmdRequestLen >= cmdExpectedLen || xferred_bytes < CMD_PACKET_LEN) {
                        if (cmdRequestLen < CMD_BUFFER_LEN)
                            memset(&cmdRequest[cmdRequestLen], 0, CMD_BUFFER_LEN - cmdRequestLen);
                        cmdRequestLen = 0;
                        cmdExpectedLen = 0;
                        handleCommand(cmdRequest);
                        cmdReplyPending = true;
                        sendReply(rhport);
                    }
                } else if (xferred_bytes >= 4 &&
                           cmdPacket[1] == CMD_DIRECTION_REQUEST &&
                           cmdPacket[2] == CMD_TRANSPORT_USB &&
                           cmdPacket[0] != CMD_FW_UPDATE) {
                    uint16_t expected = (xferred_bytes >= CMD_HEADER_LEN)
                        ? (uint16_t)(CMD_HEADER_LEN + cmdPacket[5])
                        : (uint16_t)xferred_bytes;
                    if (expected > CMD_BUFFER_LEN)
                        expected = CMD_BUFFER_LEN;
                    uint16_t copyLen = (xferred_bytes <= CMD_BUFFER_LEN) ? (uint16_t)xferred_bytes : (uint16_t)CMD_BUFFER_LEN;
                    memcpy(cmdRequest, cmdPacket, copyLen);
                    if (copyLen < expected && xferred_bytes == CMD_PACKET_LEN) {
                        cmdRequestLen = copyLen;
                        cmdExpectedLen = expected;
                    } else {
                        if (copyLen < CMD_BUFFER_LEN)
                            memset(&cmdRequest[copyLen], 0, CMD_BUFFER_LEN - copyLen);
                        handleCommand(cmdRequest);
                        cmdReplyPending = true;
                        sendReply(rhport);
                    }
                }
            } else {
                cmdRequestLen = 0;
                cmdExpectedLen = 0;
            }
            usbd_edpt_xfer(rhport, SWITCH2_PRO_EP_BULK_OUT, cmdPacket, sizeof(cmdPacket), false);
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
    if (options.has_switch2ProUserCalib && options.switch2ProUserCalib.size == sizeof(userCalibData)) {
        memcpy(userCalibData, options.switch2ProUserCalib.bytes, sizeof(userCalibData));
    } else {
        memset(userCalibData, 0xFF, sizeof(userCalibData));
    }
    userCalibDirty = false;

    if (options.switch2ProIdentity == SWITCH2_PRO_IDENTITY_GAMECUBE) {
        setProductId(&deviceDescriptor[SWITCH2_PRO_DEVICE_PID_OFFSET], SWITCH2_GAMECUBE_PRODUCT_ID);
        setProductId(&factoryData[SWITCH2_PRO_FACTORY_PID_OFFSET], SWITCH2_GAMECUBE_PRODUCT_ID);
        setProductId(&chargingGripData[SWITCH2_PRO_GRIP_PID_OFFSET], SWITCH2_GAMECUBE_PRODUCT_ID);
        productString = switch2gamecube_product_string;
        firmwareType = SWITCH2_FW_TYPE_GAMECUBE;
        return;
    }

    joyCon2Side = 0;
    if (options.switch2ProIdentity == SWITCH2_PRO_IDENTITY_JOYCON2_LEFT || options.switch2ProIdentity == SWITCH2_PRO_IDENTITY_JOYCON2_RIGHT) {
        const bool left = options.switch2ProIdentity == SWITCH2_PRO_IDENTITY_JOYCON2_LEFT;
        const uint16_t pid = left ? SWITCH2_JOYCON2_LEFT_PRODUCT_ID : SWITCH2_JOYCON2_RIGHT_PRODUCT_ID;
        joyCon2Side = left ? 1 : 2;
        setProductId(&deviceDescriptor[SWITCH2_PRO_DEVICE_PID_OFFSET], pid);
        deviceDescriptor[12] = 0x00;
        deviceDescriptor[13] = 0x01;
        setProductId(&factoryData[SWITCH2_PRO_FACTORY_PID_OFFSET], pid);
        setProductId(&chargingGripData[SWITCH2_PRO_GRIP_PID_OFFSET], pid);
        productString = left ? switch2_joycon2_left_product_string : switch2_joycon2_right_product_string;
        firmwareType = left ? SWITCH2_FW_TYPE_JOYCON2_LEFT : SWITCH2_FW_TYPE_JOYCON2_RIGHT;
        factoryData[SWITCH2_PRO_FACTORY_VARIANT_OFFSET] = 0x01;
        factoryData[SWITCH2_PRO_FACTORY_VARIANT_OFFSET + 1] = 0x08;
        factoryData[SWITCH2_PRO_FACTORY_VARIANT_OFFSET + 2] = 0x02;
        uint32_t colors[4] = { 0x323232, 0xAAAAAA, left ? 0x9BE1E6u : 0xFF8C5Fu, 0x323232 };
        if (options.switch2ProJoyCon2ColorPreset == SWITCH2_PRO_JOYCON2_COLOR_PURPLE_GREEN) {
            colors[2] = left ? 0xB19FFB : 0x90EE90;
        } else if (options.switch2ProJoyCon2ColorPreset == SWITCH2_PRO_JOYCON2_COLOR_BLUE_YELLOW) {
            colors[2] = left ? 0x2B4CFF : 0xFFF066;
        } else if (options.switch2ProJoyCon2ColorPreset == SWITCH2_PRO_JOYCON2_COLOR_LEGENDARY) {
            colors[0] = left ? 0x34A049 : 0x92DD9E;
            colors[1] = 0x264623;
            colors[2] = 0xFCFF7D;
            colors[3] = 0x364933;
        } else if (options.switch2ProJoyCon2ColorPreset == SWITCH2_PRO_JOYCON2_COLOR_GP2040) {
            colors[2] = left ? 0xEC008C : 0x00FF00;
        } else if (options.switch2ProJoyCon2ColorPreset == SWITCH2_PRO_JOYCON2_COLOR_CUSTOM) {
            const uint32_t custom[4] = {
                left ? options.switch2ProJoyCon2LeftBodyColor : options.switch2ProJoyCon2RightBodyColor,
                left ? options.switch2ProJoyCon2LeftButtonsColor : options.switch2ProJoyCon2RightButtonsColor,
                left ? options.switch2ProJoyCon2LeftAccentColor : options.switch2ProJoyCon2RightAccentColor,
                left ? options.switch2ProJoyCon2LeftStickColor : options.switch2ProJoyCon2RightStickColor,
            };
            memcpy(colors, custom, sizeof(colors));
        }
        setFactoryColor(SWITCH2_PRO_FACTORY_BODY_COLOR, colors[0]);
        setFactoryColor(SWITCH2_PRO_FACTORY_BUTTONS_COLOR, colors[1]);
        setFactoryColor(SWITCH2_PRO_FACTORY_HIGHLIGHT_COLOR, colors[2]);
        setFactoryColor(SWITCH2_PRO_FACTORY_GRIP_COLOR, colors[3]);
        return;
    }

    uint32_t body, buttons, highlight, grip;
    uint32_t leftShoulder = 0, rightShoulder = 0, stick = 0, leftRing = 0, rightRing = 0;
    bool extended = false;
    switch (options.switch2ProColorPreset) {
        case SWITCH2_PRO_COLOR_GP2040:
            body = 0x1B1B1D;
            buttons = 0xFFFFFF;
            highlight = 0x00FF00;
            grip = 0xEC008C;
            break;
        case SWITCH2_PRO_COLOR_LEGENDARY:
            body = 0x2B3631;
            buttons = 0xA0A0A0;
            highlight = 0xF2EB5E;
            grip = 0x34A049;
            leftShoulder = 0xF2EB5E;
            rightShoulder = 0xF2EB5E;
            stick = 0x2B3631;
            leftRing = 0x34A049;
            rightRing = 0xF2EB5E;
            extended = true;
            break;
        case SWITCH2_PRO_COLOR_CUSTOM:
            body = options.switch2ProBodyColor;
            buttons = options.switch2ProButtonsColor;
            highlight = options.switch2ProHighlightColor;
            grip = options.switch2ProGripColor;
            leftShoulder = options.switch2ProLeftShoulderColor;
            rightShoulder = options.switch2ProRightShoulderColor;
            stick = options.switch2ProStickColor;
            leftRing = options.switch2ProLeftRingColor;
            rightRing = options.switch2ProRightRingColor;
            extended = true;
            break;
        default:
            return;
    }
    setFactoryColor(SWITCH2_PRO_FACTORY_BODY_COLOR, body);
    setFactoryColor(SWITCH2_PRO_FACTORY_BUTTONS_COLOR, buttons);
    setFactoryColor(SWITCH2_PRO_FACTORY_HIGHLIGHT_COLOR, highlight);
    setFactoryColor(SWITCH2_PRO_FACTORY_GRIP_COLOR, grip);
    if (extended) {
        // A last variant byte of 2 makes the console use the separate shoulder, stick and ring colours.
        factoryData[SWITCH2_PRO_FACTORY_VARIANT_OFFSET + 2] = 0x02;
        setFactoryColor(SWITCH2_PRO_FACTORY_LEFT_SHOULDER_COLOR, leftShoulder);
        setFactoryColor(SWITCH2_PRO_FACTORY_RIGHT_SHOULDER_COLOR, rightShoulder);
        setFactoryColor(SWITCH2_PRO_FACTORY_STICK_COLOR, stick);
        setFactoryColor(SWITCH2_PRO_FACTORY_LEFT_RING_COLOR, leftRing);
        setFactoryColor(SWITCH2_PRO_FACTORY_RIGHT_RING_COLOR, rightRing);
    }
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
    constexpr int32_t CENTER = 2048;
    constexpr int32_t RANGE  = 1600;

    const int32_t dx = static_cast<int32_t>(x16) - 32768;
    const int32_t dy = 32768 - static_cast<int32_t>(y16);

    uint16_t x = CENTER;
    uint16_t y = CENTER;

    if (dx != 0 || dy != 0) {
        const float fx = static_cast<float>(dx) / ((dx >= 0) ? 32767.0f : 32768.0f);
        const float fy = static_cast<float>(dy) / ((dy >= 0) ? 32768.0f : 32767.0f);
        const float r = sqrtf(fx * fx + fy * fy);

        float scale = 1.0f;
        if (r > 1.0f) {
            scale = 1.0f / r;
        }

        int32_t sx = CENTER + static_cast<int32_t>(lroundf(fx * scale * RANGE));
        int32_t sy = CENTER + static_cast<int32_t>(lroundf(fy * scale * RANGE));

        if (sx < CENTER - RANGE) sx = CENTER - RANGE;
        else if (sx > CENTER + RANGE) sx = CENTER + RANGE;
        if (sy < CENTER - RANGE) sy = CENTER - RANGE;
        else if (sy > CENTER + RANGE) sy = CENTER + RANGE;

        x = static_cast<uint16_t>(sx);
        y = static_cast<uint16_t>(sy);
    }

    out[0] = x & 0xFF;
    out[1] = ((x >> 8) & 0x0F) | ((y & 0x0F) << 4);
    out[2] = (y >> 4) & 0xFF;
}

bool Switch2ProDriver::process(Gamepad * gamepad) {
    Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();
    processedGamepad->auxState.playerID.active = reportsEnabled;
    processedGamepad->auxState.playerID.ledValue = playerLedMask;
    processedGamepad->auxState.playerID.value = playerLedMask;
    const uint32_t now = to_ms_since_boot(get_absolute_time());

    if (userCalibDirty && (now - lastCalibActivityMs) >= 500) {
        GamepadOptions & options = Storage::getInstance().getGamepadOptions();
        options.has_switch2ProUserCalib = true;
        options.switch2ProUserCalib.size = sizeof(userCalibData);
        memcpy(options.switch2ProUserCalib.bytes, userCalibData, sizeof(userCalibData));
        Storage::getInstance().save();
        userCalibDirty = false;
    }

    static bool previousWakeInput = false;
    static bool lastWakeupValid = false;
    static uint32_t lastWakeupMs = 0;

    const bool currentWakeInput =
        gamepad->state.buttons != 0 ||
        gamepad->state.dpad != 0 ||
        (gamepad->hasAnalogTriggers &&
         (gamepad->state.lt >= 128 || gamepad->state.rt >= 128));
    const bool wakeEdge = currentWakeInput && !previousWakeInput;
    previousWakeInput = currentWakeInput;

    if (tud_suspended()) {
        if (wakeEdge &&
            (!lastWakeupValid || (uint32_t)(now - lastWakeupMs) >= 2000) &&
            tud_remote_wakeup()) {
            lastWakeupMs = now;
            lastWakeupValid = true;
        }
        return false;
    }

    if (!reportsEnabled)
        return false;

    uint8_t b0 = 0, b1 = 0, b2 = 0;
    if (gamepad->pressedB1()) b0 |= SWITCH2_PRO_MASK_B;
    if (gamepad->pressedB2()) b0 |= SWITCH2_PRO_MASK_A;
    if (gamepad->pressedB3()) b0 |= SWITCH2_PRO_MASK_Y;
    if (gamepad->pressedB4()) b0 |= SWITCH2_PRO_MASK_X;
    const bool gamecube = firmwareType == SWITCH2_FW_TYPE_GAMECUBE;
    if (gamepad->pressedR1()) b0 |= gamecube ? SWITCH2_GAMECUBE_MASK_Z : SWITCH2_PRO_MASK_R;
    if (gamepad->pressedR2()) b0 |= gamecube ? SWITCH2_GAMECUBE_MASK_R : SWITCH2_PRO_MASK_ZR;
    if (gamepad->pressedS2()) b0 |= SWITCH2_PRO_MASK_PLUS;
    if (gamepad->pressedR3()) b0 |= SWITCH2_PRO_MASK_RSTICK;

    if (gamepad->pressedDown())  b1 |= SWITCH2_PRO_MASK_DOWN;
    if (gamepad->pressedRight()) b1 |= SWITCH2_PRO_MASK_RIGHT;
    if (gamepad->pressedLeft())  b1 |= SWITCH2_PRO_MASK_LEFT;
    if (gamepad->pressedUp())    b1 |= SWITCH2_PRO_MASK_UP;
    if (gamepad->pressedL1())    b1 |= gamecube ? SWITCH2_GAMECUBE_MASK_ZL : SWITCH2_PRO_MASK_L;
    if (gamepad->pressedL2())    b1 |= gamecube ? SWITCH2_GAMECUBE_MASK_L : SWITCH2_PRO_MASK_ZL;
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
    if (gamecube) {
        inputReport[12] = gamepad->pressedL2() ? 0xFF : (gamepad->hasAnalogTriggers ? gamepad->state.lt : 0x00);
        inputReport[13] = gamepad->pressedR2() ? 0xFF : (gamepad->hasAnalogTriggers ? gamepad->state.rt : 0x00);
    }
    if (joyCon2Side != 0) {
        const bool left = joyCon2Side == 1;
        uint16_t x = gamepad->state.lx;
        uint16_t y = gamepad->state.ly;
        if (gamepad->pressedLeft()) x = GAMEPAD_JOYSTICK_MIN;
        if (gamepad->pressedRight()) x = GAMEPAD_JOYSTICK_MAX;
        if (gamepad->pressedUp()) y = GAMEPAD_JOYSTICK_MIN;
        if (gamepad->pressedDown()) y = GAMEPAD_JOYSTICK_MAX;
        uint8_t j0 = 0, j1 = 0;
        if (left) {
            if (gamepad->pressedB1()) j0 |= 0x04;
            if (gamepad->pressedB2()) j0 |= 0x01;
            if (gamepad->pressedB3()) j0 |= 0x08;
            if (gamepad->pressedB4()) j0 |= 0x02;
            if (gamepad->pressedS1()) j0 |= 0x40;
            if (gamepad->pressedL3()) j0 |= 0x80;
            if (gamepad->pressedA2()) j1 |= 0x01;
        } else {
            if (gamepad->pressedB1()) j0 |= 0x02;
            if (gamepad->pressedB2()) j0 |= 0x08;
            if (gamepad->pressedB3()) j0 |= 0x01;
            if (gamepad->pressedB4()) j0 |= 0x04;
            if (gamepad->pressedS2()) j0 |= 0x40;
            if (gamepad->pressedL3() || gamepad->pressedR3()) j0 |= 0x80;
            if (gamepad->pressedA1()) j1 |= 0x01;
            if (gamepad->pressedA3()) j1 |= 0x10;
        }
        if (gamepad->pressedL2()) j0 |= 0x10;
        if (gamepad->pressedR2()) j0 |= 0x20;
        if (gamepad->pressedL1()) j1 |= 0x80;
        if (gamepad->pressedR1()) j1 |= 0x40;
        memset(&inputReport[1], 0, sizeof(inputReport) - 1);
        inputReport[1] = 0x25;
        inputReport[2] = j0;
        inputReport[3] = j1;
        inputReport[4] = 0x07;
        if (left)
            packStick(&inputReport[5], GAMEPAD_JOYSTICK_MAX - y, x);
        else
            packStick(&inputReport[5], y, GAMEPAD_JOYSTICK_MAX - x);
    }
    const bool changed = memcmp(&inputReport[2], &lastSentState[0], sizeof(lastSentState)) != 0;
    if (!changed && (now - lastSentMs) < SWITCH2_PRO_KEEPALIVE_MS)
        return false;
    if (!tud_hid_n_ready(0))
        return false;

    inputReport[0] = reportCounter;
    const uint8_t defaultReportId = joyCon2Side == 1 ? SWITCH2_JOYCON2_LEFT_INPUT_REPORT_ID
        : joyCon2Side == 2 ? SWITCH2_JOYCON2_RIGHT_INPUT_REPORT_ID
        : firmwareType == SWITCH2_FW_TYPE_GAMECUBE ? SWITCH2_GAMECUBE_INPUT_REPORT_ID : SWITCH2_PRO_INPUT_REPORT_ID;
    const uint8_t reportId = (selectedReportId != 0) ? selectedReportId : defaultReportId;
    if (!tud_hid_n_report(0, reportId, inputReport, sizeof(inputReport)))
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
    if (joyCon2Side == 1)
        return switch2_joycon2_left_report_descriptor;
    if (joyCon2Side == 2)
        return switch2_joycon2_right_report_descriptor;
    if (firmwareType == SWITCH2_FW_TYPE_GAMECUBE)
        return switch2gamecube_report_descriptor;
    return switch2pro_report_descriptor;
}

const uint8_t * Switch2ProDriver::get_descriptor_configuration_cb(uint8_t index) {
    memcpy(configuration, switch2pro_configuration_descriptor, sizeof(configuration));
    if (joyCon2Side != 0)
        configuration[33] = sizeof(switch2_joycon2_left_report_descriptor);
    return configuration;
}

const uint8_t * Switch2ProDriver::get_descriptor_device_qualifier_cb() {
    return switch2pro_device_qualifier;
}

const uint8_t * Switch2ProDriver::get_descriptor_other_speed_configuration_cb(uint8_t index) {
    memcpy(otherSpeedConfiguration, get_descriptor_configuration_cb(index), sizeof(otherSpeedConfiguration));
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

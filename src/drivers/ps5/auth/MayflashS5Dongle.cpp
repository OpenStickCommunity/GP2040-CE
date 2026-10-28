/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps5/auth/MayflashS5Dongle.h"
#include "gamepad.h"   // getMicro()

#include <string.h>

#include "tusb.h"
#include "host/hcd.h"
#include "class/hid/hid.h"
#include "mbedtls/des.h"

// Personality the adapter enumerates with in PS5 mode.
static constexpr uint16_t S5_VENDOR_ID      = 0x054C;
static constexpr uint16_t S5_PRODUCT_ID_PS5 = 0x0CE6;

// Mayflash S5 vendor reports
static constexpr uint8_t S5_REPORT_GET_UNLOCK   = 0x01;   // Input:  unlock nonce
static constexpr uint8_t S5_REPORT_UNLOCK_REPLY = 0x02;   // Output: encrypted nonce

// DES key pair for the S5 unlock (Mayflash MAGS5 protocol doc)
static constexpr uint8_t S5_DES_SECRET_KEY[16] = {
    0x5C, 0x28, 0xE3, 0x05, 0x97, 0xC5, 0xAD, 0x04,
    0x9E, 0x5D, 0x19, 0xC3, 0x25, 0x40, 0x5B, 0x9D
};

// Battery/status bytes (report bytes 53-55): charged, USB data + power
static constexpr uint8_t S5_BATTERY_REPORT[3] = { 0x28, 0x18, 0x00 };

// Untouched touchpad points + timestamp
static constexpr uint8_t S5_TOUCHPAD_IDLE[9] = { 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00 };

// Current DualSense firmware report (0x20 payload). The console offers a
// controller update instead of proceeding when it sees the adapter's older
// build. From PR #1727; needs updating as Sony ships newer firmware.
static constexpr uint8_t DS_CURRENT_FIRMWARE_0x20[63] = {
    0x4A, 0x75, 0x6C, 0x20, 0x20, 0x34, 0x20, 0x32,  // "Jul  4 2025"
    0x30, 0x32, 0x35,
    0x31, 0x30, 0x3A, 0x31, 0x30, 0x3A, 0x33, 0x32,  // "10:10:32"
    0x03, 0x00,                                       // fw_type 3
    0x04, 0x00,                                       // sw_series 4
    0x14, 0x05, 0x00, 0x00,                           // hw_info 0x0514
    0x2A, 0x00, 0x10, 0x01,                           // fw_version
    0xC1, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // deviceinfo
    0x00, 0x00, 0x00, 0x00,
    0x30, 0x06,                                       // update_version 0x0630
    0x00,
    0x00, 0x38, 0x00, 0x01,
    0x00, 0x0A, 0x00, 0x02,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00
};

static constexpr uint64_t S5_TRANSFER_TIMEOUT_US = 500 * 1000;
static constexpr uint8_t  S5_MAX_RETRIES         = 3;
static constexpr uint64_t S5_RETRY_SPACING_US    = 400 * 1000;
static constexpr uint64_t S5_RESET_DELAY_US      = 882 * 1000;   // unlock -> bus reset
static constexpr uint64_t S5_RESET_HOLD_US       = 20 * 1000;
static constexpr uint64_t S5_REMOUNT_TIMEOUT_US  = 3000 * 1000;

// Smallest adapter input report spliceSignedReport() can read
static constexpr uint16_t S5_SIGNED_REPORT_MIN_LEN = 0x35;

static bool s5Encrypt(const uint8_t in[16], uint8_t out[16]) {
    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    bool ok = mbedtls_des_setkey_enc(&ctx, &S5_DES_SECRET_KEY[0]) == 0 &&
              mbedtls_des_crypt_ecb(&ctx, &in[0], &out[0]) == 0 &&
              mbedtls_des_setkey_enc(&ctx, &S5_DES_SECRET_KEY[8]) == 0 &&
              mbedtls_des_crypt_ecb(&ctx, &in[8], &out[8]) == 0;
    mbedtls_des_free(&ctx);
    return ok;
}

bool MayflashS5Dongle::matches(uint16_t vid, uint16_t pid) {
    // PS5 mode only. In PS4 mode (button held on the dongle) the adapter
    // enumerates as a DualShock 4 (054C:09CC), which PS5AuthDetector treats
    // as a PS4 dongle.
    return vid == S5_VENDOR_ID && pid == S5_PRODUCT_ID_PS5;
}

MayflashS5Dongle::MayflashS5Dongle(uint8_t dev_addr, uint8_t instance) : PS5USBAuth(dev_addr, instance) {
    memset(setupBuffer, 0, sizeof(setupBuffer));
    memset(proxyBuffer, 0, sizeof(proxyBuffer));
    memset(setBuffer, 0, sizeof(setBuffer));
    memset(&pendingReport, 0, sizeof(pendingReport));
    memset(keyBlock, 0, sizeof(keyBlock));
    memset(signedReport, 0, sizeof(signedReport));
}

bool MayflashS5Dongle::expectsReenumeration() const {
    return state == State::ResetWait || state == State::ResetAssert || state == State::AwaitRemount;
}

void MayflashS5Dongle::onReattached() {
    awaiting = false;
    probeRetries = 0;
    stateDueUs = 0;
    state = State::GetCalibration;
}

// ---- Host transfers (one control transfer in flight) ----

bool MayflashS5Dongle::issueGet(Xfer kind, uint8_t report_id, uint8_t report_type, uint8_t * buffer, uint16_t len) {
    if ( !hostGetReport(report_id, report_type, buffer, len) ) {
        return false;   // pipe busy, retry next pass
    }
    awaiting = true;
    awaitingKind = kind;
    awaitingId = report_id;
    awaitingSinceUs = getMicro();
    return true;
}

bool MayflashS5Dongle::issueSet(Xfer kind, uint8_t report_id, uint8_t report_type, uint8_t * buffer, uint16_t len) {
    if ( !hostSetReport(report_id, report_type, buffer, len) ) {
        return false;
    }
    awaiting = true;
    awaitingKind = kind;
    awaitingId = report_id;
    awaitingSinceUs = getMicro();
    return true;
}

// A setup transfer failed (stall, len 0) or timed out.
void MayflashS5Dongle::transferLost() {
    awaiting = false;
    switch ( awaitingKind ) {
        case Xfer::Proxy:
            proxy = Proxy::Stalled;
            return;
        case Xfer::ForwardSet:
            return;
        case Xfer::Setup:
        default:
            break;
    }

    if ( ++probeRetries < S5_MAX_RETRIES ) {
        if ( state == State::UnlockReply ) {
            state = State::UnlockRequest;   // restart the handshake
        }
        stateDueUs = getMicro() + S5_RETRY_SPACING_US;
        return;
    }
    probeRetries = 0;
    switch ( state ) {
        case State::UnlockRequest:
        case State::UnlockReply:
            // A real DualSense stalls or ignores the vendor request.
            probeFailed = true;
            break;
        case State::GetCalibration: state = State::GetFirmware; break;
        case State::GetFirmware:    state = State::GetPairing;  break;
        case State::GetPairing:
            state = State::Ready;
            break;
        default:
            break;
    }
}

void MayflashS5Dongle::process() {
    if ( probeFailed ) {
        return;
    }
    uint64_t now = getMicro();

    // Bus reset sequence (the adapter may be detached during it)
    switch ( state ) {
        case State::ResetWait:
            if ( now >= stateDueUs ) {
                hcd_event_device_remove(BOARD_TUH_RHPORT, false);
                tuh_rhport_reset_bus(BOARD_TUH_RHPORT, true);
                state = State::ResetAssert;
                stateDueUs = now + S5_RESET_HOLD_US;
            }
            return;
        case State::ResetAssert:
            if ( now >= stateDueUs ) {
                tuh_rhport_reset_bus(BOARD_TUH_RHPORT, false);
                hcd_event_device_attach(BOARD_TUH_RHPORT, false);
                state = State::AwaitRemount;
                stateDueUs = now + S5_REMOUNT_TIMEOUT_US;
            }
            return;
        case State::AwaitRemount:
            if ( now >= stateDueUs ) {
                probeFailed = true;
            }
            return;
        default:
            break;
    }
    if ( !isAttached() ) {
        return;
    }

    // Report signing uses the interrupt OUT pipe; hold it while a console
    // read is being proxied (as PR #1727 does).
    if ( state == State::Ready && reportPending && proxy == Proxy::Idle ) {
        if ( hostSendReady() ) {
            buildKeyBlock();
            if ( hostSendReport(keyBlock, sizeof(keyBlock)) ) {
                reportPending = false;
            }
        }
    }

    if ( awaiting ) {
        if ( (now - awaitingSinceUs) >= S5_TRANSFER_TIMEOUT_US ) {
            transferLost();
        }
        return;
    }
    if ( stateDueUs != 0 && now < stateDueUs && state != State::Ready ) {
        return;     // retry spacing
    }

    switch ( state ) {
        case State::UnlockRequest:
            memset(setupBuffer, 0, sizeof(setupBuffer));
            issueGet(Xfer::Setup, S5_REPORT_GET_UNLOCK, HID_REPORT_TYPE_INPUT, setupBuffer, 64);
            break;
        case State::UnlockReply:
            issueSet(Xfer::Setup, S5_REPORT_UNLOCK_REPLY, HID_REPORT_TYPE_OUTPUT, setupBuffer, 48);
            break;
        case State::GetCalibration:
            issueGet(Xfer::Setup, PS5_FEATURE_CALIBRATION, HID_REPORT_TYPE_FEATURE, setupBuffer, 41);
            break;
        case State::GetFirmware:
            issueGet(Xfer::Setup, PS5_FEATURE_FIRMWARE, HID_REPORT_TYPE_FEATURE, setupBuffer, 64);
            break;
        case State::GetPairing:
            issueGet(Xfer::Setup, PS5_FEATURE_PAIRING_INFO, HID_REPORT_TYPE_FEATURE, setupBuffer, 16);
            break;
        case State::Ready:
            // A held console read goes first, then queued console writes.
            if ( proxy == Proxy::Requested ) {
                memset(proxyBuffer, 0, sizeof(proxyBuffer));
                if ( issueGet(Xfer::Proxy, proxyId, HID_REPORT_TYPE_FEATURE, proxyBuffer, proxyLen) ) {
                    proxy = Proxy::InFlight;
                }
            } else if ( setCount > 0 ) {
                PendingSet & ps = setQueue[setHead];
                memcpy(setBuffer, ps.data, ps.len);
                if ( issueSet(Xfer::ForwardSet, ps.id, HID_REPORT_TYPE_FEATURE, setBuffer, ps.len) ) {
                    setHead = (setHead + 1) % SET_QUEUE_LEN;
                    setCount--;
                }
            }
            break;
        default:
            break;
    }
}

void MayflashS5Dongle::onSetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( !awaiting || report_id != awaitingId ) {
        return;   // late or unexpected
    }
    if ( len == 0 ) {
        transferLost();
        return;
    }
    awaiting = false;

    if ( awaitingKind == Xfer::Setup && state == State::UnlockReply ) {
        probeRetries = 0;
        state = State::ResetWait;
        stateDueUs = getMicro() + S5_RESET_DELAY_US;
    }
}

void MayflashS5Dongle::onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( !awaiting || report_id != awaitingId ) {
        return;
    }
    if ( len == 0 ) {
        transferLost();     // stall: passed on for proxied reads
        return;
    }
    awaiting = false;

    if ( awaitingKind == Xfer::Proxy ) {
        proxyReplyLen = len < sizeof(proxyBuffer) ? len : sizeof(proxyBuffer);
        proxy = Proxy::Done;
        return;
    }

    probeRetries = 0;
    stateDueUs = 0;
    switch ( state ) {
        case State::UnlockRequest: {
            // Only the adapter answers with its nonce: bytes 1-6 zero, 0x08 at 7,
            // 0x10 (16-byte length) at 12, nonce at 13-28.
            bool isS5 = (len == 64 && setupBuffer[7] == 0x08 && setupBuffer[12] == 0x10);
            for (uint8_t i = 1; isS5 && i <= 6; i++) {
                isS5 = (setupBuffer[i] == 0x00);
            }
            if ( !isS5 ) {
                probeFailed = true;
                return;
            }
            uint8_t encrypted[16];
            if ( !s5Encrypt(&setupBuffer[13], encrypted) ) {
                probeFailed = true;
                return;
            }
            memset(setupBuffer, 0, sizeof(setupBuffer));
            setupBuffer[0] = 0x02;
            setupBuffer[1] = 0x08;      // authentication data
            setupBuffer[3] = 0x10;      // 16 bytes
            memcpy(&setupBuffer[4], encrypted, sizeof(encrypted));
            state = State::UnlockReply;
            break;
        }
        case State::GetCalibration:
            state = State::GetFirmware;
            break;
        case State::GetFirmware:
            state = State::GetPairing;
            break;
        case State::GetPairing:
            state = State::Ready;
            break;
        default:
            break;
    }
}

// ---- Console feature reports ----

bool MayflashS5Dongle::proxiesFeature(uint8_t report_id) const {
    // 0x81 (test command result) is stalled locally, as the adapter does.
    return ready() && report_id != PS5_FEATURE_GET_TEST;
}

bool MayflashS5Dongle::startProxyGet(uint8_t report_id, uint16_t len) {
    if ( !ready() || proxy != Proxy::Idle || len == 0 ) {
        return false;
    }
    proxyId = report_id;
    proxyLen = len;
    proxy = Proxy::Requested;
    return true;
}

PS5ProxyResult MayflashS5Dongle::pollProxyGet(uint8_t * buffer, uint16_t & len) {
    switch ( proxy ) {
        case Proxy::Done:
            memcpy(buffer, proxyBuffer, proxyReplyLen);
            if ( proxyId == PS5_FEATURE_FIRMWARE && proxyReplyLen >= 64 ) {
                memcpy(&buffer[1], DS_CURRENT_FIRMWARE_0x20, sizeof(DS_CURRENT_FIRMWARE_0x20));
            }
            len = proxyReplyLen;
            proxy = Proxy::Idle;
            return PS5ProxyResult::Ready;
        case Proxy::Stalled:
            proxy = Proxy::Idle;
            return PS5ProxyResult::Stalled;
        default:
            return PS5ProxyResult::Pending;
    }
}

bool MayflashS5Dongle::setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len) {
    // Every console write goes to the adapter as-is (report ID prepended).
    if ( len + 1 > PS5_ENDPOINT_SIZE ) {
        return true;
    }
    if ( setCount == SET_QUEUE_LEN ) {
        return true;
    }
    PendingSet & ps = setQueue[(setHead + setCount) % SET_QUEUE_LEN];
    ps.id = report_id;
    ps.len = (uint8_t)(len + 1);
    ps.data[0] = report_id;
    memcpy(&ps.data[1], buffer, len);
    setCount++;
    return true;
}

// ---- Report signing ----

bool MayflashS5Dongle::submitReport(const PS5Report & report) {
    if ( reportPending ) {
        return false;
    }
    pendingReport = report;
    reportPending = true;
    return true;
}

bool MayflashS5Dongle::takeSignedReport(uint8_t out[PS5_ENDPOINT_SIZE]) {
    if ( !reportSigned ) {
        return false;
    }
    memcpy(out, signedReport, PS5_ENDPOINT_SIZE);
    reportSigned = false;
    return true;
}

void MayflashS5Dongle::buildKeyBlock() {
    memset(keyBlock, 0, sizeof(keyBlock));
    keyBlock[0] = 0x02;                                          // host -> adapter
    keyBlock[1] = 0x04;                                          // key data
    keyBlock[3] = 0x18;                                          // 24 bytes follow
    memcpy(&keyBlock[4], &pendingReport, 12);                    // report ID .. buttons
    memcpy(&keyBlock[16], S5_BATTERY_REPORT, sizeof(S5_BATTERY_REPORT));
    memcpy(&keyBlock[19], S5_TOUCHPAD_IDLE, sizeof(S5_TOUCHPAD_IDLE));
}

// DualSense-layout report to the console (Mayflash MAGS5 doc / PR #1727)
void MayflashS5Dongle::spliceSignedReport(const uint8_t * report) {
    uint8_t * out = signedReport;
    memset(out, 0, PS5_ENDPOINT_SIZE);
    memcpy(&out[0x00], &report[0x0D], 16);                      // 12-byte key data + 4-byte counter
    memcpy(&out[0x1C], &report[0x25], 5);                       // sensor timestamp + temperature
    memcpy(&out[0x21], S5_TOUCHPAD_IDLE, sizeof(TouchpadData)); // touchpad (as sent in the key block)
    memcpy(&out[0x2A], &report[0x2A], 2);                       // trigger status
    memcpy(&out[0x31], &report[0x31], 4);                       // device timestamp
    memcpy(&out[0x35], S5_BATTERY_REPORT, sizeof(S5_BATTERY_REPORT));
    memcpy(&out[0x38], &report[0x1D], 8);                       // AES-CMAC
}

void MayflashS5Dongle::onReport(const uint8_t * report, uint16_t len) {
    if ( !ready() || len < S5_SIGNED_REPORT_MIN_LEN ) {
        return;
    }
    spliceSignedReport(report);     // newest signature wins
    reportSigned = true;
}

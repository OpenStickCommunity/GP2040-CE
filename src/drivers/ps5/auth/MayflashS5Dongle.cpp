/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps5/auth/MayflashS5Dongle.h"
#include "gamepad.h"   // getMicro()

#include <string.h>

#include "class/hid/hid.h"
#include "mbedtls/des.h"

// Personalities the adapter enumerates with in PS5 mode.
static constexpr uint16_t S5_VENDOR_ID      = 0x054C;
static constexpr uint16_t S5_PRODUCT_ID_PS5 = 0x0CE6;

// Mayflash S5 vendor reports
static constexpr uint8_t S5_REPORT_GET_UNLOCK   = 0x01;   // Input:  unlock nonce
static constexpr uint8_t S5_REPORT_UNLOCK_REPLY = 0x02;   // Output: encrypted nonce

// DES key pair for the S5 unlock (from the vendor updater)
static constexpr uint8_t S5_DES_SECRET_KEY[16] = {
    0x5C, 0x28, 0xE3, 0x05, 0x97, 0xC5, 0xAD, 0x04,
    0x9E, 0x5D, 0x19, 0xC3, 0x25, 0x40, 0x5B, 0x9D
};

// Charging, 100%; USB data and power plugged
static constexpr uint8_t S5_BATTERY_REPORT[3] = { 0x28, 0x18, 0x00 };

// Untouched touchpad points + timestamp
static constexpr uint8_t S5_TOUCHPAD_IDLE[9] = { 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00 };

static constexpr uint64_t S5_TRANSFER_TIMEOUT_US = 500 * 1000;
static constexpr uint8_t  S5_MAX_PROBE_RETRIES   = 3;
static constexpr uint64_t S5_F2_DELAY_US         = 500 * 1000;   // Before the first signing-state poll
static constexpr uint64_t S5_F2_POLL_INTERVAL_US = 10 * 1000;    // Between "not ready" polls

// Smallest dongle input report spliceSignedReport() can read
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
    // TODO: add the adapter's PS4-mode personality so it can be reported as
    // "wrong mode" rather than ignored.
    return vid == S5_VENDOR_ID && pid == S5_PRODUCT_ID_PS5;
}

MayflashS5Dongle::MayflashS5Dongle(uint8_t dev_addr, uint8_t instance) : PS5USBAuth(dev_addr, instance) {
    memset(transferBuffer, 0, sizeof(transferBuffer));
    memset(calibration, 0, sizeof(calibration));
    memset(firmware, 0, sizeof(firmware));
    memset(pairing, 0, sizeof(pairing));
    memset(consoleF0, 0, sizeof(consoleF0));
    memset(dongleF1, 0, sizeof(dongleF1));
    memset(&pendingReport, 0, sizeof(pendingReport));
    memset(keyBlock, 0, sizeof(keyBlock));
    memset(signedReport, 0, sizeof(signedReport));
    PS5_LOG("[S5] created at addr=%u inst=%u, starting unlock\n", dev_addr, instance);
}

// ---- Host transfers ----

bool MayflashS5Dongle::issueGet(uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( !hostGetReport(report_id, report_type, transferBuffer, len) ) {
        return false;   // pipe busy, retry next pass
    }
    awaiting = true;
    awaitingId = report_id;
    awaitingSinceUs = getMicro();
    return true;
}

bool MayflashS5Dongle::issueSet(uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( !hostSetReport(report_id, report_type, transferBuffer, len) ) {
        return false;
    }
    awaiting = true;
    awaitingId = report_id;
    awaitingSinceUs = getMicro();
    return true;
}

void MayflashS5Dongle::transferLost() {
    awaiting = false;
    PS5_LOG("[S5] transfer 0x%02x timed out (state %u, retry %u)\n", awaitingId, (unsigned)state, probeRetries + 1);
    if ( state >= State::Idle ) {
        // Abandon this auth round; the console starts another one.
        state = State::Idle;
        return;
    }
    if ( ++probeRetries < S5_MAX_PROBE_RETRIES ) {
        if ( !unlocked ) {
            state = State::UnlockRequest;   // restart the handshake
        }
        return;                             // otherwise retry the same step
    }
    probeRetries = 0;
    if ( !unlocked ) {
        // The unlock never completed: a real DualSense typically stalls the
        // vendor request, which never produces a completion.
        PS5_LOG("[S5] unlock never completed: not a MagicBoots S5 in PS5 mode\n");
        probeFailed = true;
    } else {
        // An identity report the adapter won't give us: keep the DualSense
        // default for it and move on.
        PS5_LOG("[S5] skipping setup step %u, using DualSense default\n", (unsigned)state);
        advanceSetup();
    }
}

void MayflashS5Dongle::advanceSetup() {
    switch ( state ) {
        case State::GetCalibration:   state = State::GetFirmware;      break;
        case State::GetFirmware:      state = State::GetPairing;       break;
        case State::GetPairing:       state = State::DisableBluetooth; break;
        case State::DisableBluetooth:
            state = State::Idle;
            setupDone = true;
            PS5_LOG("[S5] setup complete, ready to sign\n");
            break;
        default: break;
    }
}

void MayflashS5Dongle::process() {
    if ( probeFailed ) {
        return;
    }

    // Report signing uses the interrupt OUT pipe, independent of control transfers.
    if ( setupDone && reportPending ) {
        if ( !hostSendReady() ) {
            sendBusyPasses++;   // previous key block still in flight: normal backpressure
        } else {
            buildKeyBlock();
            if ( hostSendReport(keyBlock, sizeof(keyBlock)) ) {
                reportPending = false;
                if ( keyBlocksSent++ % 1000 == 0 ) {
                    PS5_LOG("[S5] key blocks sent: %lu (IN reports received: %lu, OUT-busy passes: %lu)\n",
                            (unsigned long)keyBlocksSent, (unsigned long)reportsIn, (unsigned long)sendBusyPasses);
                }
            } else if ( !sendFailLogged ) {
                sendFailLogged = true;
                PS5_LOG("[S5] tuh_hid_send_report failed for key block\n");
            }
        }
    }

    if ( awaiting ) {
        if ( (getMicro() - awaitingSinceUs) >= S5_TRANSFER_TIMEOUT_US ) {
            transferLost();
        }
        return;
    }

    switch ( state ) {
        // -- Unlock and identity fetch --
        case State::UnlockRequest:
            memset(transferBuffer, 0, sizeof(transferBuffer));
            issueGet(S5_REPORT_GET_UNLOCK, HID_REPORT_TYPE_INPUT, 64);
            break;
        case State::UnlockReply:
            issueSet(S5_REPORT_UNLOCK_REPLY, HID_REPORT_TYPE_OUTPUT, 48);
            break;
        case State::GetCalibration:
            issueGet(PS5_FEATURE_CALIBRATION, HID_REPORT_TYPE_FEATURE, sizeof(calibration) + 1);
            break;
        case State::GetFirmware:
            issueGet(PS5_FEATURE_FIRMWARE, HID_REPORT_TYPE_FEATURE, sizeof(firmware) + 1);
            break;
        case State::GetPairing:
            issueGet(PS5_FEATURE_PAIRING_INFO, HID_REPORT_TYPE_FEATURE, sizeof(pairing) + 1);
            break;
        case State::DisableBluetooth:
            // As sent by #1630 after the pairing fetch. NOTE: the mode lands
            // in byte 0 here but byte 1 in the console-forwarded path below;
            // confirm which the adapter expects.
            memset(transferBuffer, 0, 25);
            transferBuffer[0] = 0x02;
            issueSet(PS5_FEATURE_SET_BLUETOOTH, HID_REPORT_TYPE_FEATURE, 25);
            break;

        // -- Auth exchange --
        case State::SendF0: {
            uint8_t * frame = transferBuffer;
            frame[0] = PS5_FEATURE_SET_AUTH_PAYLOAD;
            frame[1] = authType;
            frame[2] = frameId;
            frame[3] = f0Sent;
            memcpy(&frame[4], &consoleF0[f0Sent * PS5_AUTH_PAGE_LEN], PS5_AUTH_PAGE_LEN);
            ps5WriteAuthCrc(PS5_FEATURE_SET_AUTH_PAYLOAD, &frame[1], 63);
            if ( issueSet(PS5_FEATURE_SET_AUTH_PAYLOAD, HID_REPORT_TYPE_FEATURE, 64) ) {
                f0Sent++;
            }
            break;
        }
        case State::RecvF2Delay:
        case State::RecvF2:
            if ( getMicro() >= dueUs ) {
                memset(transferBuffer, 0, 16);
                issueGet(PS5_FEATURE_GET_SIGNING_STATE, HID_REPORT_TYPE_FEATURE, 16);
            }
            break;
        case State::RecvF1:
            memset(transferBuffer, 0, sizeof(transferBuffer));
            issueGet(PS5_FEATURE_GET_SIGNATURE, HID_REPORT_TYPE_FEATURE, 64);
            break;
        case State::Idle:
            // Forward console commands only between auth transfers.
            if ( bluetoothPending ) {
                memset(transferBuffer, 0, 17);
                transferBuffer[0] = PS5_FEATURE_SET_BLUETOOTH;
                transferBuffer[1] = bluetoothMode;
                if ( issueSet(PS5_FEATURE_SET_BLUETOOTH, HID_REPORT_TYPE_FEATURE, 17) ) {
                    bluetoothPending = false;
                }
            } else if ( testPending ) {
                memset(transferBuffer, 0, sizeof(transferBuffer));
                transferBuffer[0] = PS5_FEATURE_SET_TEST;
                transferBuffer[1] = testCommand[0];
                transferBuffer[2] = testCommand[1];
                if ( issueSet(PS5_FEATURE_SET_TEST, HID_REPORT_TYPE_FEATURE, 64) ) {
                    testPending = false;
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
    awaiting = false;
    probeRetries = 0;

    switch ( state ) {
        case State::UnlockReply:
            unlocked = true;
            state = State::GetCalibration;
            PS5_LOG("[S5] unlocked, fetching calibration/firmware/pairing\n");
            return;
        case State::DisableBluetooth:
            advanceSetup();
            return;
        default:
            break;
    }

    // Forwarded console commands complete without changing auth state.
    if ( report_id != PS5_FEATURE_SET_AUTH_PAYLOAD ) {
        return;
    }

    switch ( state ) {
        case State::SendF0:
            PS5_LOG("[S5] F0 page %u/%u delivered to dongle\n", f0Sent, f0Received);
            if ( f0Sent < f0Received ) {
                break;   // more pages already buffered: keep sending
            }
            if ( authType == 0x01 ) {
                if ( f0Sent >= PS5_AUTH_PAGES ) {
                    state = State::RecvF2Delay;
                    dueUs = getMicro() + S5_F2_DELAY_US;
                } else {
                    state = State::Idle;   // wait for the console's next page
                }
            } else if ( authType == 0x02 ) {
                state = State::RecvF2;
                dueUs = getMicro();
            } else if ( authType == 0x03 ) {
                state = State::RecvF2Delay;
                dueUs = getMicro() + S5_F2_DELAY_US;
            } else {
                state = State::Idle;
            }
            break;
        default:
            break;
    }
}

void MayflashS5Dongle::onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( !awaiting || report_id != awaitingId ) {
        return;
    }
    awaiting = false;
    probeRetries = 0;

    // transferBuffer[0] is the report ID
    uint16_t payloadLen = (len > 0) ? len - 1 : 0;

    switch ( state ) {
        case State::UnlockRequest: {
            // 0x08 / 0x10 marks the S5's encrypted-nonce response
            if ( len != 64 || transferBuffer[7] != 0x08 || transferBuffer[12] != 0x10 ) {
                PS5_LOG("[S5] unlock response mismatch (len=%u [7]=%02x [12]=%02x): not a MagicBoots S5\n",
                        len, transferBuffer[7], transferBuffer[12]);
                probeFailed = true;   // not a MagicBoots (or not in PS5 mode)
                return;
            }
            PS5_LOG("[S5] unlock nonce received, sending encrypted reply\n");
            uint8_t nonce[16];
            uint8_t encrypted[16];
            memcpy(nonce, &transferBuffer[13], sizeof(nonce));
            if ( !s5Encrypt(nonce, encrypted) ) {
                probeFailed = true;
                return;
            }
            memset(transferBuffer, 0, sizeof(transferBuffer));
            transferBuffer[0] = 0x02;
            transferBuffer[1] = 0x08;
            transferBuffer[2] = 0x00;
            transferBuffer[3] = 0x10;
            memcpy(&transferBuffer[4], encrypted, sizeof(encrypted));
            state = State::UnlockReply;
            break;
        }
        case State::GetCalibration:
            PS5_LOG("[S5] calibration 0x05 len=%u\n", len);
            memcpy(calibration, &transferBuffer[1], payloadLen < sizeof(calibration) ? payloadLen : sizeof(calibration));
            advanceSetup();
            break;
        case State::GetFirmware:
            PS5_LOG("[S5] firmware 0x20 len=%u\n", len);
            memcpy(firmware, &transferBuffer[1], payloadLen < sizeof(firmware) ? payloadLen : sizeof(firmware));
            advanceSetup();
            break;
        case State::GetPairing:
            PS5_LOG("[S5] pairing 0x09 len=%u\n", len);
            memcpy(pairing, &transferBuffer[1], payloadLen < sizeof(pairing) ? payloadLen : sizeof(pairing));
            advanceSetup();
            break;
        case State::RecvF2Delay:
        case State::RecvF2:
            // Log only status changes; "not ready" is polled every 10ms.
            if ( transferBuffer[3] != lastF2Status ) {
                lastF2Status = transferBuffer[3];
                PS5_LOG("[S5] dongle F2 frame=%02x status=%02x\n", transferBuffer[2], transferBuffer[3]);
            }
            switch ( transferBuffer[3] ) {
                case PS5_AUTH_STATUS_READY:
                case PS5_AUTH_STATUS_REFRESH_READY:
                    state = State::RecvF1;
                    break;
                case PS5_AUTH_STATUS_DONE:
                    state = State::Idle;
                    break;
                case PS5_AUTH_STATUS_NOT_READY:
                case PS5_AUTH_STATUS_REFRESH_NOT_READY:
                default:
                    state = State::RecvF2;   // still signing: poll again shortly
                    dueUs = getMicro() + S5_F2_POLL_INTERVAL_US;
                    break;
            }
            break;
        case State::RecvF1: {
            uint8_t page = transferBuffer[3];
            if ( transferBuffer[2] != frameId || page >= PS5_AUTH_PAGES ) {
                PS5_LOG("[S5] dongle F1 rejected: frame=%02x (want %02x) page=%u\n", transferBuffer[2], frameId, page);
                state = State::Idle;   // stale or malformed page
                break;
            }
            PS5_LOG("[S5] dongle F1 frame=%02x page=%u\n", frameId, page);
            memcpy(&dongleF1[page * PS5_AUTH_PAGE_LEN], &transferBuffer[4], PS5_AUTH_PAGE_LEN);
            if ( authType == 0x01 && page < PS5_AUTH_PAGES - 1 ) {
                break;   // fetch the next page
            }
            f1Ready = true;
            state = State::Idle;
            PS5_LOG("[S5] response ready for console\n");
            break;
        }
        default:
            break;
    }
}

// ---- Console feature reports ----

bool MayflashS5Dongle::getFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen, uint16_t & outLen) {
    switch ( report_id ) {
        case PS5_FEATURE_CALIBRATION:
        case PS5_FEATURE_FIRMWARE:
        case PS5_FEATURE_PAIRING_INFO: {
            // Mirror the adapter's identity once fetched; DualSense defaults before that.
            if ( !setupDone ) {
                return false;
            }
            const uint8_t * src = (report_id == PS5_FEATURE_CALIBRATION) ? calibration :
                                  (report_id == PS5_FEATURE_FIRMWARE)    ? firmware : pairing;
            uint16_t size = (report_id == PS5_FEATURE_CALIBRATION) ? sizeof(calibration) :
                            (report_id == PS5_FEATURE_FIRMWARE)    ? sizeof(firmware) : sizeof(pairing);
            if ( reqlen < size ) {
                return false;
            }
            memcpy(buffer, src, size);
            outLen = size;
            return true;
        }
        case PS5_FEATURE_GET_SIGNATURE:
            if ( reqlen < 63 ) {
                return false;
            }
            memset(buffer, 0, 63);
            buffer[0] = (authType == 0x03) ? 0x01 : authType;
            buffer[1] = frameId;
            buffer[2] = f1ServeIndex;
            memcpy(&buffer[3], &dongleF1[f1ServeIndex * PS5_AUTH_PAGE_LEN], PS5_AUTH_PAGE_LEN);
            ps5WriteAuthCrc(PS5_FEATURE_GET_SIGNATURE, buffer, 63);
            f1ServeIndex = (f1ServeIndex + 1) % PS5_AUTH_PAGES;
            outLen = 63;
            return true;
        case PS5_FEATURE_GET_SIGNING_STATE: {
            if ( reqlen < 15 ) {
                return false;
            }
            uint8_t status;
            if ( f1Done ) {
                PS5_LOG("[S5] console F2 -> done (0x40), round complete\n");
                status = PS5_AUTH_STATUS_DONE;
                // Round complete: ready for the next challenge
                f0Received = 0;
                f0Sent = 0;
                f1ServeIndex = 0;
                f1Ready = false;
                f1Done = false;
            } else if ( f1Ready ) {
                status = (authType == 0x03) ? PS5_AUTH_STATUS_REFRESH_READY : PS5_AUTH_STATUS_READY;
                PS5_LOG("[S5] console F2 -> ready (%02x)\n", status);
                f1ServeIndex = 0;
            } else {
                status = (authType == 0x03) ? PS5_AUTH_STATUS_REFRESH_NOT_READY : PS5_AUTH_STATUS_NOT_READY;
            }
            memset(buffer, 0, 15);
            buffer[1] = frameId;
            buffer[2] = status;
            ps5WriteAuthCrc(PS5_FEATURE_GET_SIGNING_STATE, buffer, 15);
            outLen = 15;
            return true;
        }
        default:
            return false;
    }
}

void MayflashS5Dongle::beginConsoleRound(const PS5AuthPage & page) {
    f0Received = 1;
    f0Sent = 0;
    frameId = page.frame_id;
    authType = page.auth_type;
    state = State::SendF0;
}

bool MayflashS5Dongle::setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len) {
    switch ( report_id ) {
        case PS5_FEATURE_SET_BLUETOOTH:
            if ( len == 16 ) {
                bluetoothMode = buffer[0];
                bluetoothPending = true;
            }
            return true;
        case PS5_FEATURE_SET_TEST:
            if ( len == 63 ) {
                testCommand[0] = buffer[0];
                testCommand[1] = buffer[1];
                testPending = true;
            }
            return true;
        case PS5_FEATURE_SET_AUTH_PAYLOAD: {
            if ( len != sizeof(PS5AuthPage) ) {
                return true;
            }
            // The adapter mishandles a challenge while PS is held.
            if ( reportPending && pendingReport.button_home ) {
                PS5_LOG("[S5] console F0 skipped: PS button held\n");
                return true;
            }
            const PS5AuthPage & page = *(const PS5AuthPage *)buffer;
            if ( page.page_index >= PS5_AUTH_PAGES ) {
                PS5_LOG("[S5] console F0 bad page index %u\n", page.page_index);
                return true;
            }
            PS5_LOG("[S5] console F0 type=%02x frame=%02x page=%u\n", page.auth_type, page.frame_id, page.page_index);
            memcpy(&consoleF0[page.page_index * PS5_AUTH_PAGE_LEN], page.data, PS5_AUTH_PAGE_LEN);

            if ( page.auth_type == 0x01 ) {
                if ( page.page_index == 0 ) {
                    f1Ready = false;
                    f1Done = false;
                    beginConsoleRound(page);
                } else if ( f0Received < PS5_AUTH_PAGES ) {
                    f0Received++;
                    if ( state == State::Idle ) {
                        state = State::SendF0;   // sent once the pipe is free
                    }
                }
            } else if ( page.auth_type == 0x02 ) {
                // Final page: tell the console we're done while the dongle finishes.
                f1Done = true;
                beginConsoleRound(page);
            } else if ( page.auth_type == 0x03 ) {
                f1Ready = false;
                f1Done = false;
                beginConsoleRound(page);
            }
            return true;
        }
        default:
            return false;
    }
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

void MayflashS5Dongle::spliceSignedReport(const uint8_t * report) {
    uint8_t * out = signedReport;
    memset(out, 0, PS5_ENDPOINT_SIZE);
    memcpy(&out[0x00], &report[0x0D], 16);                      // 12-byte key data + 4-byte counter
    memcpy(&out[0x1C], &report[0x25], 5);                       // sensor timestamp + temperature
    memcpy(&out[0x21], S5_TOUCHPAD_IDLE, sizeof(TouchpadData)); // touchpad (not passed through yet)
    memcpy(&out[0x2A], &report[0x2A], 2);                       // trigger status
    memcpy(&out[0x31], &report[0x31], 4);                       // device timestamp
    memcpy(&out[0x35], S5_BATTERY_REPORT, sizeof(S5_BATTERY_REPORT));
    memcpy(&out[0x38], &report[0x1D], 8);                       // AES-CMAC
}

void MayflashS5Dongle::onReport(const uint8_t * report, uint16_t len) {
    // First few IN reports in full detail, then a running count.
    if ( reportsIn < 3 ) {
        PS5_LOG("[S5] IN report #%lu len=%u setup=%u: %02x %02x %02x %02x %02x %02x %02x %02x | [0x1D..] %02x %02x %02x %02x\n",
                (unsigned long)reportsIn, len, setupDone,
                report[0], report[1], report[2], report[3], report[4], report[5], report[6], report[7],
                len > 0x20 ? report[0x1D] : 0, len > 0x20 ? report[0x1E] : 0,
                len > 0x20 ? report[0x1F] : 0, len > 0x20 ? report[0x20] : 0);
    }
    reportsIn++;
    if ( !setupDone ) {
        return;
    }
    if ( len < S5_SIGNED_REPORT_MIN_LEN ) {
        if ( shortReports++ == 0 ) {
            PS5_LOG("[S5] IN report too short to splice (len=%u, need %u)\n", len, S5_SIGNED_REPORT_MIN_LEN);
        }
        return;
    }
    if ( !firstSignedLogged ) {
        firstSignedLogged = true;
        PS5_LOG("[S5] first signed report from dongle (len=%u)\n", len);
    }
    // Newest signature wins
    spliceSignedReport(report);
    reportSigned = true;
}

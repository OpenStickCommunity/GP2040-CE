/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps5/auth/P5GeneralDongle.h"
#include "gamepad.h"   // getMicro()

#include <string.h>

#include "class/hid/hid.h"

// A dongle GET/SET_REPORT that has not completed within this window is treated
// as lost. Failed completions (len == 0) are dropped by USBHostManager, and the
// console's next request is only accepted while idle, so a lost completion
// would otherwise wedge auth until replug.
static constexpr uint64_t P5GENERAL_WAIT_TIMEOUT_US = 500 * 1000;

// The dongle needs time after the last F0 page before its signing state is valid.
static constexpr uint64_t P5GENERAL_F2_DELAY_US = 500 * 1000;

P5GeneralDongle::P5GeneralDongle(uint8_t dev_addr, uint8_t instance) : PS5USBAuth(dev_addr, instance) {
    memset(authBuffer, 0, sizeof(authBuffer));
    memset(transferBuffer, 0, sizeof(transferBuffer));
    PS5_LOG("[P5G] created at addr=%u inst=%u (ready immediately)\n", dev_addr, instance);
}

void P5GeneralDongle::enterWait(State waitState) {
    state = waitState;
    waitSinceUs = getMicro();
}

void P5GeneralDongle::process() {
    // Sign the latest input report. Only clear the pending flag once the
    // transfer is queued, so a busy pipe is retried on the next pass.
    if ( reportPending && hostSendReady() && hostSendReport(pendingReport, sizeof(pendingReport)) ) {
        reportPending = false;
    }

    switch ( state ) {
        case State::SendF0:
            memcpy(transferBuffer, authBuffer, sizeof(transferBuffer));
            if ( hostSetReport(PS5_FEATURE_SET_AUTH_PAYLOAD, HID_REPORT_TYPE_FEATURE, transferBuffer, sizeof(transferBuffer)) ) {
                enterWait(State::SendF0Wait);
            }
            break;
        case State::RecvF1:
            if ( f1Remaining == 0 ) {
                state = State::Idle;
            } else if ( hostGetReport(PS5_FEATURE_GET_SIGNATURE, HID_REPORT_TYPE_FEATURE, transferBuffer, sizeof(transferBuffer)) ) {
                f1Remaining--;
                enterWait(State::RecvF1Wait);
            }
            break;
        case State::RecvF2Delay:
            if ( getMicro() < f2DueUs ) {
                break;
            }
            state = State::RecvF2;
            [[fallthrough]];
        case State::RecvF2:
            if ( hostGetReport(PS5_FEATURE_GET_SIGNING_STATE, HID_REPORT_TYPE_FEATURE, transferBuffer, 16) ) {
                enterWait(State::RecvF2Wait);
            }
            break;
        case State::SendF0Wait:
        case State::RecvF1Wait:
        case State::RecvF2Wait:
            // Completion lost: return to idle so the console's next request is
            // accepted. A late completion is ignored because its handler only
            // acts while still in the matching wait state.
            if ( (getMicro() - waitSinceUs) >= P5GENERAL_WAIT_TIMEOUT_US ) {
                PS5_LOG("[P5G] dongle transfer timed out (state %u), back to idle\n", (unsigned)state);
                state = State::Idle;
            }
            break;
        case State::Idle:
        default:
            break;
    }
}

bool P5GeneralDongle::getFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen, uint16_t & outLen) {
    switch ( report_id ) {
        case PS5_FEATURE_GET_SIGNATURE:
            outLen = 63;
            break;
        case PS5_FEATURE_GET_SIGNING_STATE:
            outLen = 15;
            break;
        default:
            return false;
    }
    if ( reqlen < outLen ) {
        outLen = reqlen;
    }

    // Serve the last page fetched from the dongle, then fetch the next one.
    memcpy(buffer, authBuffer + 1, outLen);
    if ( state == State::Idle ) {
        state = State::RecvF1;
    }
    return true;
}

bool P5GeneralDongle::setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len) {
    if ( report_id != PS5_FEATURE_SET_AUTH_PAYLOAD ) {
        return false;
    }
    // Only accept a new page while idle; the console retries.
    const PS5AuthPage * page = (const PS5AuthPage *)buffer;
    if ( len == sizeof(PS5AuthPage) && state == State::Idle ) {
        authBuffer[0] = report_id;
        memcpy(authBuffer + 1, buffer, len);
        state = State::SendF0;
        PS5_LOG("[P5G] console F0 type=%02x frame=%02x page=%u -> dongle\n", page->auth_type, page->frame_id, page->page_index);
    } else {
        PS5_LOG("[P5G] console F0 dropped (len=%u, busy=%u)\n", len, state != State::Idle);
    }
    return true;
}

bool P5GeneralDongle::submitReport(const PS5Report & report) {
    if ( reportPending ) {
        return false;
    }
    memcpy(pendingReport, &report, sizeof(pendingReport));
    reportPending = true;
    return true;
}

bool P5GeneralDongle::takeSignedReport(uint8_t out[PS5_ENDPOINT_SIZE]) {
    if ( !reportSigned ) {
        return false;
    }
    memcpy(out, signedReport, PS5_ENDPOINT_SIZE);
    reportSigned = false;
    return true;
}

void P5GeneralDongle::onReport(const uint8_t * report, uint16_t len) {
    // Keep the oldest unsent signature; the next one follows on the next pass.
    if ( reportSigned ) {
        return;
    }
    if ( !firstSignedLogged ) {
        firstSignedLogged = true;
        PS5_LOG("[P5G] first signed report from dongle (len=%u)\n", len);
    }
    memset(signedReport, 0, sizeof(signedReport));
    memcpy(signedReport, report, len < sizeof(signedReport) ? len : sizeof(signedReport));
    reportSigned = true;
}

void P5GeneralDongle::onSetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( report_id != PS5_FEATURE_SET_AUTH_PAYLOAD || state != State::SendF0Wait ) {
        return;
    }
    if ( len == 0 ) {
        PS5_LOG("[P5G] dongle rejected F0, back to idle\n");
        state = State::Idle;
        return;
    }

    const PS5AuthPage * page = (const PS5AuthPage *)(authBuffer + 1);
    switch ( page->auth_type ) {
        case 0x01: f1Remaining = 4; break;   // Full challenge
        case 0x03: f1Remaining = 1; break;   // Refresh
        case 0x02:                           // Final
        default:   f1Remaining = 0; break;
    }

    // After the last page of a round, poll the signing state once the dongle
    // has had time to sign. Otherwise wait for the console's next page.
    bool lastPage = (page->auth_type == 0x01 && page->page_index == 3) ||
                    page->auth_type == 0x02 || page->auth_type == 0x03;
    PS5_LOG("[P5G] F0 page=%u delivered%s\n", page->page_index, lastPage ? ", polling F2 in 500ms" : "");
    if ( lastPage ) {
        f2DueUs = getMicro() + P5GENERAL_F2_DELAY_US;
        state = State::RecvF2Delay;
    } else {
        state = State::Idle;
    }
}

void P5GeneralDongle::onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {
    bool expected = (report_id == PS5_FEATURE_GET_SIGNATURE     && state == State::RecvF1Wait) ||
                    (report_id == PS5_FEATURE_GET_SIGNING_STATE && state == State::RecvF2Wait);
    if ( !expected ) {
        return;
    }
    if ( len == 0 ) {
        PS5_LOG("[P5G] dongle stalled 0x%02x, back to idle\n", report_id);
        state = State::Idle;
        return;
    }
    memcpy(authBuffer, transferBuffer, len < sizeof(authBuffer) ? len : sizeof(authBuffer));
    if ( report_id == PS5_FEATURE_GET_SIGNING_STATE ) {
        PS5_LOG("[P5G] dongle F2 frame=%02x status=%02x\n", transferBuffer[2], transferBuffer[3]);
    } else {
        PS5_LOG("[P5G] dongle F1 frame=%02x page=%u (len=%u, %u left)\n", transferBuffer[2], transferBuffer[3], len, f1Remaining);
    }
    state = State::Idle;
}

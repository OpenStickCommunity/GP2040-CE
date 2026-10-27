/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps5/PS5ModeDriver.h"
#include "peripheralmanager.h"
#include "storagemanager.h"

#include <stdio.h>

// How long boot may hold the console connection waiting for a PS5 dongle,
// and the longer limit while a bound dongle is still setting up (the
// MagicBoots S5 unlocks, waits ~0.9 s and re-enumerates before it's ready).
#define PS5_MODE_DECIDE_US        (2000ull * 1000ull)
#define PS5_MODE_DECIDE_MAX_US    (5000ull * 1000ull)

// How long to stay disconnected before re-enumerating as the PS4 arcade stick,
// so the console registers the unplug.
#define PS5_MODE_SWITCH_HOLD_US   (500ull * 1000ull)

void PS5ModeDriver::initialize() {
    ps5.initialize();
    ps4.initialize();

    // TinyUSB keeps a pointer to our class_driver; its contents follow the
    // active path (both paths share hidd_init, which runs once at tud_init).
    class_driver = *ps5.get_class_driver();
}

void PS5ModeDriver::initializeAux() {
    usbHostAuth = PeripheralManager::getInstance().isUSBEnabled(0);
    ps5.initializeAux();
    ps4.initializeAux();
    listener.set(ps5.get_usb_auth_listener(), ps4.get_usb_auth_listener());
}

USBListener * PS5ModeDriver::get_usb_auth_listener() {
    return listener.empty() ? nullptr : &listener;
}

GPDriver * PS5ModeDriver::active() {
    return (path == Path::PS4 || path == Path::SwitchingToPS4) ? (GPDriver *)&ps4 : (GPDriver *)&ps5;
}

void PS5ModeDriver::connectAs(Path newPath) {
    path = newPath;
    if (path == Path::PS5) {
        ps5.selectIdentity();
    }
    class_driver = *active()->get_class_driver();
    PS5_LOG("[PS5Mode] connecting as %s\n", path == Path::PS5 ? "PS5 third-party (PS5Driver)" : "PS4 arcade stick (PS4Driver)");
    tud_connect();
}

bool PS5ModeDriver::process(Gamepad * gamepad) {
    switch (path) {
        case Path::Deciding: {
            if (deadlineUs == 0) {
                // First pass, right after tusb_init: keep the console from
                // enumerating us until we know which identity to present.
                tud_disconnect();
                decideStartUs = getMicro();
                deadlineUs = decideStartUs + PS5_MODE_DECIDE_US;
                PS5_LOG("[PS5Mode] holding connection up to %llums for a PS5 dongle\n",
                        (unsigned long long)(PS5_MODE_DECIDE_US / 1000));
            }
            ps5.process(gamepad);   // pumps dongle detection; nothing is sent while disconnected
            if (ps5.authInitializing() && deadlineUs < decideStartUs + PS5_MODE_DECIDE_MAX_US) {
                deadlineUs = decideStartUs + PS5_MODE_DECIDE_MAX_US;
                PS5_LOG("[PS5Mode] dongle still setting up, waiting up to %llums\n",
                        (unsigned long long)(PS5_MODE_DECIDE_MAX_US / 1000));
            }
            if (ps5.readyAuth() != nullptr) {
                connectAs(Path::PS5);
            } else if (!usbHostAuth || getMicro() >= deadlineUs) {
                PS5_LOG("[PS5Mode] no PS5 dongle ready, falling back\n");
                connectAs(Path::PS4);
            }
            return false;
        }
        case Path::PS5: {
            bool sent = ps5.process(gamepad);
            if (ps5.identityMismatch()) {
                // A dongle that signs for a different identity was bound
                // after connecting (hot swap): re-enumerate with its identity.
                PS5_LOG("[PS5Mode] bound dongle needs another identity, re-enumerating\n");
                tud_disconnect();
                deadlineUs = getMicro() + PS5_MODE_SWITCH_HOLD_US;
                path = Path::ReconnectPS5;
            } else if (ps5.consoleAuthFailed()) {
                PS5_LOG("[PS5Mode] console did not complete PS5 auth, re-enumerating as PS4 arcade stick\n");
                tud_disconnect();
                deadlineUs = getMicro() + PS5_MODE_SWITCH_HOLD_US;
                path = Path::SwitchingToPS4;
            }
            return sent;
        }
        case Path::ReconnectPS5:
            ps5.process(gamepad);   // keep the dongle serviced while disconnected
            if (getMicro() >= deadlineUs) {
                connectAs(Path::PS5);
            }
            return false;
        case Path::SwitchingToPS4:
            if (getMicro() >= deadlineUs) {
                connectAs(Path::PS4);
            }
            return false;
        case Path::PS4:
        default:
            return ps4.process(gamepad);
    }
}

void PS5ModeDriver::processAux() {
    // PS4 auth (dongle passthrough or key signing) runs on core 1, exactly as
    // in PS4 mode. It only does work once the console sends a PS4 nonce.
    ps4.processAux();
    ps5.processAux();
}

bool PS5ModeDriver::getAuthSent() {
    switch (path) {
        case Path::PS5: return ps5.getAuthSent();
        case Path::PS4: return ps4.getAuthSent();
        default:        return false;
    }
}

bool PS5ModeDriver::getDongleAuthRequired() {
    // Saving restarts the USB host, which would drop either kind of dongle.
    return usbHostAuth || ps4.getDongleAuthRequired();
}

const char * PS5ModeDriver::statusLabel() {
    const char * text = "PS5";
    if (path == Path::PS5 && ps5.getAuthSent() && ps5.readyAuth() != nullptr) {
        snprintf(label, sizeof(label), "PS5%s", ps5.readyAuth()->shortName());
        text = label;
    } else if (path == Path::PS4 && ps4.getAuthSent()) {
        bool keys = Storage::getInstance().getGamepadOptions().ps5AuthType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_KEYS;
        text = keys ? "PS4:AS" : "PS5:AS";
    }
    if (text != label) {
        snprintf(label, sizeof(label), "%s", text);
    }
    // Pad to 6 characters so the rest of the status bar doesn't shift.
    size_t n = strlen(label);
    while (n < 6) {
        label[n++] = ' ';
    }
    label[n] = '\0';
    return label;
}

// ---- Forwarding to the active path ----

uint16_t PS5ModeDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen) {
    return active()->get_report(report_id, report_type, buffer, reqlen);
}

void PS5ModeDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const * buffer, uint16_t bufsize) {
    active()->set_report(report_id, report_type, buffer, bufsize);
}

void PS5ModeDriver::set_report_n(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const * buffer, uint16_t bufsize) {
    active()->set_report_n(itf, report_id, report_type, buffer, bufsize);
}

bool PS5ModeDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) {
    return active()->vendor_control_xfer_cb(rhport, stage, request);
}

const uint16_t * PS5ModeDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    return active()->get_descriptor_string_cb(index, langid);
}

const uint8_t * PS5ModeDriver::get_descriptor_device_cb() {
    return active()->get_descriptor_device_cb();
}

const uint8_t * PS5ModeDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return active()->get_hid_descriptor_report_cb(itf);
}

const uint8_t * PS5ModeDriver::get_descriptor_configuration_cb(uint8_t index) {
    return active()->get_descriptor_configuration_cb(index);
}

const uint8_t * PS5ModeDriver::get_descriptor_device_qualifier_cb() {
    return active()->get_descriptor_device_qualifier_cb();
}

const uint8_t * PS5ModeDriver::get_descriptor_other_speed_configuration_cb(uint8_t index) {
    return active()->get_descriptor_other_speed_configuration_cb(index);
}

uint16_t PS5ModeDriver::GetJoystickMidValue() {
    return active()->GetJoystickMidValue();
}

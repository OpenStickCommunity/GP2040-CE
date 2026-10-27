/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/ps5/PS5AuthDetector.h"
#include "drivers/ps5/auth/P5GeneralDongle.h"
#include "drivers/ps5/auth/MayflashS5Dongle.h"

#include "host/usbh.h"
#include "class/hid/hid_host.h"

// Supported auth devices, highest priority first.
struct PS5AuthProvider {
    bool (*matches)(uint16_t vid, uint16_t pid);
    PS5USBAuth * (*create)(uint8_t dev_addr, uint8_t instance);
};

static const PS5AuthProvider providers[] = {
    { P5GeneralDongle::matches,  P5GeneralDongle::create  },
    { MayflashS5Dongle::matches, MayflashS5Dongle::create },
};

static_assert(PS5_AUTH_MAX_INTERFACES >= CFG_TUH_HID,
              "PS5_AUTH_MAX_INTERFACES must cover every host HID interface");

// ---- PS5USBAuth host helpers ----

bool PS5USBAuth::hostGetReport(uint8_t report_id, uint8_t report_type, void * buffer, uint16_t len) {
    return tuh_hid_get_report(devAddr, instance, report_id, report_type, buffer, len);
}

bool PS5USBAuth::hostSetReport(uint8_t report_id, uint8_t report_type, void * buffer, uint16_t len) {
    return tuh_hid_set_report(devAddr, instance, report_id, report_type, buffer, len);
}

bool PS5USBAuth::hostSendReport(const void * buffer, uint16_t len) {
    return tuh_hid_send_report(devAddr, instance, 0, buffer, len);
}

bool PS5USBAuth::hostSendReady() const {
    return tuh_hid_send_ready(devAddr, instance);
}

// ---- PS5AuthDetector ----

void PS5AuthDetector::setup() {
    numCandidates = 0;
    activeIndex = -1;
    active = nullptr;
}

void PS5AuthDetector::mount(uint8_t dev_addr, uint8_t instance, uint8_t const * desc_report, uint16_t desc_len) {
    uint16_t vid, pid;
    if ( !tuh_vid_pid_get(dev_addr, &vid, &pid) ) {
        PS5_LOG("[PS5Det] mount addr=%u inst=%u: no VID/PID, ignored\n", dev_addr, instance);
        return;
    }

    bool matched = false;
    for (uint8_t type = 0; type < TU_ARRAY_SIZE(providers); type++) {
        if ( !providers[type].matches(vid, pid) ) {
            continue;
        }
        matched = true;
        if ( parked && active != nullptr && type == activeType ) {
            // The parked provider's device is back after its own reset.
            if ( numCandidates < PS5_AUTH_MAX_INTERFACES ) {
                activeIndex = numCandidates;
                candidates[numCandidates++] = { dev_addr, instance, type, false };
            }
            parked = false;
            active->reattach(dev_addr, instance);
            PS5_LOG("[PS5Det] %s re-enumerated at addr=%u inst=%u\n", active->name(), dev_addr, instance);
            return;
        }
        if ( numCandidates < PS5_AUTH_MAX_INTERFACES ) {
            candidates[numCandidates++] = { dev_addr, instance, type, false };
            PS5_LOG("[PS5Det] mount addr=%u inst=%u %04x:%04x -> candidate (provider %u), %u candidate(s)\n",
                    dev_addr, instance, vid, pid, type, numCandidates);
        } else {
            PS5_LOG("[PS5Det] mount addr=%u inst=%u %04x:%04x -> candidate list full, ignored\n",
                    dev_addr, instance, vid, pid);
        }
        break;
    }
    if ( !matched ) {
        PS5_LOG("[PS5Det] mount addr=%u inst=%u %04x:%04x -> not an auth device\n", dev_addr, instance, vid, pid);
    }

    // A higher-priority device may have just arrived; only switch when idle
    // so an in-progress auth session is never torn down by a hot-plug.
    if ( active == nullptr ) {
        bindNext();
    }
}

void PS5AuthDetector::unmount(uint8_t dev_addr) {
    bool lostActive = (active != nullptr && !parked && active->address() == dev_addr);
    if ( lostActive && active->expectsReenumeration() ) {
        // Intentional reset: keep the provider and its state.
        PS5_LOG("[PS5Det] unmount addr=%u: %s re-enumerating, keeping it\n", dev_addr, active->name());
        active->detach();
        parked = true;
        activeIndex = -1;
        lostActive = false;
    } else if ( lostActive ) {
        PS5_LOG("[PS5Det] unmount addr=%u: bound %s removed\n", dev_addr, active->name());
        unbind();
    }

    // Drop every candidate interface on this device. activeIndex is invalid
    // after compaction, but it is -1 whenever lostActive was handled above,
    // and is recomputed below otherwise.
    uint8_t kept = 0;
    int8_t newActiveIndex = -1;
    for (uint8_t i = 0; i < numCandidates; i++) {
        if ( candidates[i].dev_addr == dev_addr ) {
            continue;
        }
        if ( (int8_t)i == activeIndex ) {
            newActiveIndex = kept;
        }
        candidates[kept++] = candidates[i];
    }
    if ( kept != numCandidates ) {
        PS5_LOG("[PS5Det] unmount addr=%u: %u candidate(s) left\n", dev_addr, kept);
    }
    numCandidates = kept;
    activeIndex = newActiveIndex;

    if ( active == nullptr ) {
        bindNext();
    }
}

void PS5AuthDetector::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const * report, uint16_t len) {
    if ( active != nullptr && active->owns(dev_addr, instance) ) {
        active->onReport(report, len);
    }
}

void PS5AuthDetector::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( active != nullptr && active->owns(dev_addr, instance) ) {
        active->onSetReportComplete(report_id, report_type, len);
    }
}

void PS5AuthDetector::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( active != nullptr && active->owns(dev_addr, instance) ) {
        active->onGetReportComplete(report_id, report_type, len);
    }
}

void PS5AuthDetector::process() {
    if ( active == nullptr ) {
        return;
    }

    active->process();

    if ( active->failed() ) {
        PS5_LOG("[PS5Det] %s at addr=%u failed probe, rejected until unplugged\n", active->name(), active->address());
        if ( activeIndex >= 0 ) {
            candidates[activeIndex].rejected = true;
        }
        unbind();
        bindNext();
    }
}

void PS5AuthDetector::unbind() {
    delete active;
    active = nullptr;
    activeIndex = -1;
    parked = false;
}

void PS5AuthDetector::bindNext() {
    // Lowest provider index (highest priority) wins; ties go to the
    // interface that was mounted first.
    int8_t best = -1;
    for (uint8_t i = 0; i < numCandidates; i++) {
        if ( candidates[i].rejected ) {
            continue;
        }
        if ( best < 0 || candidates[i].type < candidates[best].type ) {
            best = i;
        }
    }
    if ( best < 0 ) {
        PS5_LOG("[PS5Det] no usable auth device, running unsigned\n");
        return;
    }

    const Candidate & c = candidates[best];
    active = providers[c.type].create(c.dev_addr, c.instance);
    activeIndex = (active != nullptr) ? best : -1;
    activeType = c.type;
    if ( active != nullptr ) {
        PS5_LOG("[PS5Det] bound %s at addr=%u inst=%u\n", active->name(), c.dev_addr, c.instance);
    } else {
        PS5_LOG("[PS5Det] failed to allocate provider %u\n", c.type);
    }
}

/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_AUTH_H_
#define _PS5_AUTH_H_

#include <stdint.h>
#include <string.h>
#include "CRC32.h"
#include "drivers/ps5/PS5Descriptors.h"

// PS5 auth protocol constants shared by auth providers.
#define PS5_AUTH_PAGE_LEN  56    // Payload bytes per F0/F1 page
#define PS5_AUTH_PAGES     4     // Pages per full challenge/response

typedef enum {
    PS5_AUTH_STATUS_NOT_READY         = 0x11,
    PS5_AUTH_STATUS_READY             = 0x12,
    PS5_AUTH_STATUS_DONE              = 0x40,
    PS5_AUTH_STATUS_REFRESH_NOT_READY = 0x02,
    PS5_AUTH_STATUS_REFRESH_READY     = 0x52,
} PS5AuthStatus;

// F0 page as sent by the console (report ID stripped)
typedef struct __attribute__((packed)) {
    uint8_t auth_type;       // 0x01 full, 0x02 final, 0x03 refresh
    uint8_t frame_id;
    uint8_t page_index;
    uint8_t data[PS5_AUTH_PAGE_LEN];
    uint8_t crc32[4];
} PS5AuthPage;

static_assert(sizeof(PS5AuthPage) == 63, "PS5AuthPage must match the 63-byte F0 payload");

// Write the CRC32 trailer of an F0/F1/F2 payload (report ID stripped). The
// CRC covers the report ID plus every payload byte before the 4-byte trailer.
static inline void ps5WriteAuthCrc(uint8_t report_id, uint8_t * payload, uint16_t payloadLen) {
    uint8_t tmp[PS5_ENDPOINT_SIZE];
    uint16_t covered = payloadLen - sizeof(uint32_t);
    tmp[0] = report_id;
    memcpy(&tmp[1], payload, covered);
    uint32_t crc = CRC32::calculate(tmp, covered + 1);
    memcpy(&payload[covered], &crc, sizeof(crc));
}

// Outcome of a live-proxied console feature read.
enum class PS5ProxyResult : uint8_t { Pending, Ready, Stalled };

//
// An external authenticator for PS5 mode.
//
// PS5Driver presents a third-party PS5 controller identity and builds plain
// input reports. An auth provider answers the console's auth feature reports and
// signs input reports. All methods run on core 0 (TinyUSB device + host tasks).
//
class PS5Auth {
public:
    virtual ~PS5Auth() = default;
    virtual const char * name() const = 0;
    virtual const char * shortName() const = 0;   // 3 chars, for the status bar

    // Console-facing identity this provider's signatures are valid for.
    virtual const PS5Identity & identity() const { return ps5ThirdPartyIdentity; }

    // Initialized/unlocked and able to sign. Until then PS5Driver sends
    // unsigned reports and answers auth requests with "not ready".
    virtual bool ready() const = 0;

    // The device turned out not to be a usable authenticator (e.g. a real
    // DualSense matched by VID/PID but failed the unlock). The owner drops it.
    virtual bool failed() const { return false; }

    // Pump the provider's state machine.
    virtual void process() = 0;

    // Console feature reports. Return false if this provider does not handle
    // the report, in which case PS5Driver answers with its default.
    virtual bool getFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen, uint16_t & outLen) { return false; }
    virtual bool setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len) { return false; }

    // Live proxy for console feature reads. When proxiesFeature() is true,
    // PS5Driver holds the console's GET_REPORT (EP0 NAKs) and calls
    // startProxyGet(); it then polls until the provider has the device's
    // answer (raw, report ID in byte 0) or the device stalled.
    virtual bool proxiesFeature(uint8_t report_id) const { return false; }
    virtual bool startProxyGet(uint8_t report_id, uint16_t len) { return false; }
    virtual PS5ProxyResult pollProxyGet(uint8_t * buffer, uint16_t & len) { return PS5ProxyResult::Stalled; }

    // Per-report signing. submitReport() returns false while a previous
    // report is still being signed. takeSignedReport() returns true when a
    // signed 64-byte report is ready to send to the console.
    virtual bool submitReport(const PS5Report & report) = 0;
    virtual bool takeSignedReport(uint8_t out[PS5_ENDPOINT_SIZE]) = 0;
};

//
// Base for authenticators attached to the USB host port.
// PS5AuthDetector creates these and forwards host callbacks that belong to
// this device's address/instance only.
//
class PS5USBAuth : public PS5Auth {
public:
    uint8_t address() const { return devAddr; }
    bool owns(uint8_t dev_addr, uint8_t inst) const { return attached && dev_addr == devAddr && inst == instance; }

    // The provider is about to re-enumerate its device on purpose (e.g. a
    // bus reset). The detector keeps it across the unmount and calls
    // reattach() when the same kind of device mounts again.
    virtual bool expectsReenumeration() const { return false; }
    void detach() { attached = false; }
    void reattach(uint8_t dev_addr, uint8_t inst) {
        devAddr = dev_addr;
        instance = inst;
        attached = true;
        onReattached();
    }
    bool isAttached() const { return attached; }

    virtual void onReport(const uint8_t * report, uint16_t len) {}
    virtual void onSetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {}

protected:
    PS5USBAuth(uint8_t dev_addr, uint8_t inst) : devAddr(dev_addr), instance(inst) {}
    virtual void onReattached() {}

    // Thin wrappers over TinyUSB host HID. They return false when the
    // transfer could not be queued (control pipe busy, device gone); callers
    // retry on a later process() pass.
    bool hostGetReport(uint8_t report_id, uint8_t report_type, void * buffer, uint16_t len);
    bool hostSetReport(uint8_t report_id, uint8_t report_type, void * buffer, uint16_t len);
    bool hostSendReport(const void * buffer, uint16_t len);
    bool hostSendReady() const;

    uint8_t devAddr;
    uint8_t instance;
    bool attached = true;
};

#endif // _PS5_AUTH_H_

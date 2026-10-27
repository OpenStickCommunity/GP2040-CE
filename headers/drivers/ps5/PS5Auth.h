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

#ifndef PS5_AUTH_DEBUG
#define PS5_AUTH_DEBUG 0
#endif

#include <stdio.h>
#if PS5_AUTH_DEBUG
#define PS5_LOG(...) printf(__VA_ARGS__)
#else
#define PS5_LOG(...) do { if (0) printf(__VA_ARGS__); } while (0)
#endif

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

static inline void ps5WriteAuthCrc(uint8_t report_id, uint8_t * payload, uint16_t payloadLen) {
    uint8_t tmp[PS5_ENDPOINT_SIZE];
    uint16_t covered = payloadLen - sizeof(uint32_t);
    tmp[0] = report_id;
    memcpy(&tmp[1], payload, covered);
    uint32_t crc = CRC32::calculate(tmp, covered + 1);
    memcpy(&payload[covered], &crc, sizeof(crc));
}

class PS5Auth {
public:
    virtual ~PS5Auth() = default;
    virtual const char * name() const = 0;

    virtual bool ready() const = 0;

    virtual bool failed() const { return false; }

    virtual void process() = 0;

    virtual bool getFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen, uint16_t & outLen) { return false; }
    virtual bool setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len) { return false; }

    virtual bool submitReport(const PS5Report & report) = 0;
    virtual bool takeSignedReport(uint8_t out[PS5_ENDPOINT_SIZE]) = 0;
};

class PS5USBAuth : public PS5Auth {
public:
    uint8_t address() const { return devAddr; }
    bool owns(uint8_t dev_addr, uint8_t inst) const { return dev_addr == devAddr && inst == instance; }

    virtual void onReport(const uint8_t * report, uint16_t len) {}
    virtual void onSetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len) {}

protected:
    PS5USBAuth(uint8_t dev_addr, uint8_t inst) : devAddr(dev_addr), instance(inst) {}

    bool hostGetReport(uint8_t report_id, uint8_t report_type, void * buffer, uint16_t len);
    bool hostSetReport(uint8_t report_id, uint8_t report_type, void * buffer, uint16_t len);
    bool hostSendReport(const void * buffer, uint16_t len);
    bool hostSendReady() const;

    const uint8_t devAddr;
    const uint8_t instance;
};

#endif // _PS5_AUTH_H_

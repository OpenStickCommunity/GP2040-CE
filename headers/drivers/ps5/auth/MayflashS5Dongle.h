/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_MAYFLASHS5_DONGLE_H_
#define _PS5_MAYFLASHS5_DONGLE_H_

#include "drivers/ps5/PS5Auth.h"

//
// Mayflash MagicBoots S5 in PS5 mode (Mayflash MAGS5/TGS protocol, and the
// working sequence from GP2040-CE PR #1727).
//
// The adapter enumerates as a DualSense, so VID/PID only makes it a
// candidate: it must answer the DES unlock handshake before it is used. A
// real DualSense fails the handshake and is dropped (failed() == true).
//
// Setup: unlock, wait ~880 ms, reset the host bus so the adapter
// re-enumerates, then read 0x05, 0x20 and 0x09. Once ready:
//  - the console sees the DualSense identity;
//  - every console feature write (including F0) is forwarded raw;
//  - every console feature read (including F1/F2) is proxied live to the
//    adapter and answered with its raw reply (0x81 stalls, 0x20 is replaced
//    with a current DualSense firmware build);
//  - input reports are signed by sending a compact key block and splicing the
//    returned CMAC into a DualSense report.
//
class MayflashS5Dongle : public PS5USBAuth {
public:
    static bool matches(uint16_t vid, uint16_t pid);
    static PS5USBAuth * create(uint8_t dev_addr, uint8_t instance) { return new MayflashS5Dongle(dev_addr, instance); }

    virtual const char * name() const { return "Mayflash S5"; }
    virtual const char * shortName() const { return "MS5"; }
    virtual const PS5Identity & identity() const { return ps5DualSenseIdentity; }
    virtual bool ready() const { return state == State::Ready; }
    virtual bool failed() const { return probeFailed; }
    virtual bool expectsReenumeration() const;
    virtual void process();

    virtual bool setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len);
    virtual bool proxiesFeature(uint8_t report_id) const;
    virtual bool startProxyGet(uint8_t report_id, uint16_t len);
    virtual PS5ProxyResult pollProxyGet(uint8_t * buffer, uint16_t & len);

    virtual bool submitReport(const PS5Report & report);
    virtual bool takeSignedReport(uint8_t out[PS5_ENDPOINT_SIZE]);

    virtual void onReport(const uint8_t * report, uint16_t len);
    virtual void onSetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len);
    virtual void onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len);

protected:
    virtual void onReattached();

private:
    MayflashS5Dongle(uint8_t dev_addr, uint8_t instance);

    enum class State : uint8_t {
        UnlockRequest,      // GET input 0x01: adapter's nonce
        UnlockReply,        // SET output 0x02: DES-encrypted nonce
        ResetWait,          // adapter expects ~880 ms before the bus reset
        ResetAssert,        // bus reset held
        AwaitRemount,       // waiting for the adapter to re-enumerate
        GetCalibration,     // 0x05
        GetFirmware,        // 0x20
        GetPairing,         // 0x09
        Ready,
    };

    // What the control transfer in flight is for
    enum class Xfer : uint8_t { Setup, Proxy, ForwardSet };

    enum class Proxy : uint8_t { Idle, Requested, InFlight, Done, Stalled };

    // A console feature write waiting to be forwarded (report ID in data[0])
    struct PendingSet {
        uint8_t id;
        uint8_t len;
        uint8_t data[PS5_ENDPOINT_SIZE];
    };
    static constexpr uint8_t SET_QUEUE_LEN = 8;

    bool issueGet(Xfer kind, uint8_t report_id, uint8_t report_type, uint8_t * buffer, uint16_t len);
    bool issueSet(Xfer kind, uint8_t report_id, uint8_t report_type, uint8_t * buffer, uint16_t len);
    void transferLost();
    void buildKeyBlock();
    void spliceSignedReport(const uint8_t * report);

    State state = State::UnlockRequest;
    bool probeFailed = false;
    uint8_t probeRetries = 0;
    uint64_t stateDueUs = 0;            // ResetWait / ResetAssert / AwaitRemount deadline

    bool awaiting = false;
    Xfer awaitingKind = Xfer::Setup;
    uint8_t awaitingId = 0;
    uint64_t awaitingSinceUs = 0;

    uint8_t setupBuffer[PS5_ENDPOINT_SIZE];

    // Live proxy of one console feature read
    Proxy proxy = Proxy::Idle;
    uint8_t proxyId = 0;
    uint16_t proxyLen = 0;
    uint16_t proxyReplyLen = 0;
    uint8_t proxyBuffer[PS5_ENDPOINT_SIZE];

    // Console feature writes, forwarded in order
    PendingSet setQueue[SET_QUEUE_LEN];
    uint8_t setHead = 0;
    uint8_t setCount = 0;
    uint8_t setBuffer[PS5_ENDPOINT_SIZE];

    // Report signing
    PS5Report pendingReport;
    bool reportPending = false;
    uint8_t keyBlock[48];
    uint8_t signedReport[PS5_ENDPOINT_SIZE];
    bool reportSigned = false;
    bool firstSignedLogged = false;
    uint32_t keyBlocksSent = 0;     // logging
    uint32_t reportsIn = 0;         // logging: every IN report from the adapter
    uint32_t shortReports = 0;      // logging
    uint32_t sendBusyPasses = 0;    // logging
    bool sendFailLogged = false;
};

#endif // _PS5_MAYFLASHS5_DONGLE_H_

/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_MAYFLASHS5_DONGLE_H_
#define _PS5_MAYFLASHS5_DONGLE_H_

#include "drivers/ps5/PS5Auth.h"

//
// Mayflash MagicBoots S5 in PS5 mode.
//
// The adapter enumerates as a DualSense, so VID/PID only makes it a
// candidate: it must answer the DES unlock handshake before it is used.
// A real DualSense fails the handshake and is dropped (failed() == true).
//
// After unlocking, its calibration, firmware and pairing reports are mirrored
// to the console. Challenge pages are buffered and re-framed for the dongle,
// and input reports are signed by sending a compact key block and splicing
// the returned CMAC into a DualSense report.
//
class MayflashS5Dongle : public PS5USBAuth {
public:
    static bool matches(uint16_t vid, uint16_t pid);
    static PS5USBAuth * create(uint8_t dev_addr, uint8_t instance) { return new MayflashS5Dongle(dev_addr, instance); }

    virtual const char * name() const { return "Mayflash S5"; }
    virtual const char * shortName() const { return "MS5"; }
    virtual bool ready() const { return setupDone; }   // unlocked and identity fetched
    virtual bool failed() const { return probeFailed; }
    virtual void process();

    virtual bool getFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen, uint16_t & outLen);
    virtual bool setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len);

    virtual bool submitReport(const PS5Report & report);
    virtual bool takeSignedReport(uint8_t out[PS5_ENDPOINT_SIZE]);

    virtual void onReport(const uint8_t * report, uint16_t len);
    virtual void onSetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len);
    virtual void onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len);

private:
    MayflashS5Dongle(uint8_t dev_addr, uint8_t instance);

    enum class State : uint8_t {
        // Unlock and identity fetch, in order
        UnlockRequest,
        UnlockReply,
        GetCalibration,
        GetFirmware,
        GetPairing,
        DisableBluetooth,
        // Auth exchange
        Idle,
        SendF0,
        RecvF2Delay,
        RecvF2,
        RecvF1,
    };

    // One control transfer in flight at a time
    bool issueGet(uint8_t report_id, uint8_t report_type, uint16_t len);
    bool issueSet(uint8_t report_id, uint8_t report_type, uint16_t len);
    void transferLost();
    void advanceSetup();
    void beginConsoleRound(const PS5AuthPage & page);
    void buildKeyBlock();
    void spliceSignedReport(const uint8_t * report);

    State state = State::UnlockRequest;
    bool unlocked = false;
    bool probeFailed = false;
    bool setupDone = false;
    uint8_t probeRetries = 0;

    bool awaiting = false;
    uint8_t awaitingId = 0;
    uint64_t awaitingSinceUs = 0;
    uint64_t dueUs = 0;                         // RecvF2Delay / F2 poll spacing

    uint8_t transferBuffer[PS5_ENDPOINT_SIZE];  // Host control transfer buffer

    // Mirrored identity reports (report ID stripped)
    uint8_t calibration[40];
    uint8_t firmware[63];
    uint8_t pairing[15];

    // Console -> dongle challenge
    uint8_t consoleF0[PS5_AUTH_PAGE_LEN * PS5_AUTH_PAGES];
    uint8_t f0Received = 0;       // Pages received from the console this round
    uint8_t f0Sent = 0;           // Pages forwarded to the dongle this round
    uint8_t authType = 0;         // 0x01 full, 0x02 final, 0x03 refresh
    uint8_t frameId = 0;

    // Dongle -> console response
    uint8_t dongleF1[PS5_AUTH_PAGE_LEN * PS5_AUTH_PAGES];
    uint8_t f1ServeIndex = 0;
    bool f1Ready = false;
    bool f1Done = false;

    // Console commands forwarded to the dongle when the pipe is free
    bool bluetoothPending = false;
    uint8_t bluetoothMode = 0;
    bool testPending = false;
    uint8_t testCommand[2] = {0, 0};

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
    uint8_t lastF2Status = 0;
};

#endif // _PS5_MAYFLASHS5_DONGLE_H_

/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_P5GENERAL_DONGLE_H_
#define _PS5_P5GENERAL_DONGLE_H_

#include "drivers/ps5/PS5Auth.h"

//
// Besavior P5General auth dongle.
//
// The dongle speaks the console's F0/F1/F2 framing directly, so challenge
// pages are passed through one at a time. Input reports are signed by
// sending the full 64-byte report out and taking the returned report as-is.
//
class P5GeneralDongle : public PS5USBAuth {
public:
    static constexpr uint16_t VENDOR_ID  = 0x2B81;
    static constexpr uint16_t PRODUCT_ID = 0x0101;

    static bool matches(uint16_t vid, uint16_t pid) { return vid == VENDOR_ID && pid == PRODUCT_ID; }
    static PS5USBAuth * create(uint8_t dev_addr, uint8_t instance) { return new P5GeneralDongle(dev_addr, instance); }

    virtual const char * name() const { return "P5General"; }
    virtual const char * shortName() const { return "P5G"; }
    virtual bool ready() const { return true; }   // usable as soon as it enumerates
    virtual void process();

    virtual bool getFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen, uint16_t & outLen);
    virtual bool setFeature(uint8_t report_id, const uint8_t * buffer, uint16_t len);

    virtual bool submitReport(const PS5Report & report);
    virtual bool takeSignedReport(uint8_t out[PS5_ENDPOINT_SIZE]);

    virtual void onReport(const uint8_t * report, uint16_t len);
    virtual void onSetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len);
    virtual void onGetReportComplete(uint8_t report_id, uint8_t report_type, uint16_t len);

private:
    P5GeneralDongle(uint8_t dev_addr, uint8_t instance);

    enum class State : uint8_t {
        Idle,
        SendF0,
        SendF0Wait,
        RecvF1,
        RecvF1Wait,
        RecvF2Delay,
        RecvF2,
        RecvF2Wait,
    };

    void enterWait(State waitState);

    State state = State::Idle;
    uint8_t f1Remaining = 0;     // F1 pages still to fetch for this round
    uint64_t waitSinceUs = 0;    // When the current *Wait state was entered
    uint64_t f2DueUs = 0;        // When the delayed F2 poll may run

    uint8_t authBuffer[PS5_ENDPOINT_SIZE];      // [0] report ID, then payload
    uint8_t transferBuffer[PS5_ENDPOINT_SIZE];  // Host control transfer buffer

    uint8_t pendingReport[PS5_ENDPOINT_SIZE];
    uint8_t signedReport[PS5_ENDPOINT_SIZE];
    bool reportPending = false;
    bool reportSigned = false;
    bool firstSignedLogged = false;
};

#endif // _PS5_P5GENERAL_DONGLE_H_

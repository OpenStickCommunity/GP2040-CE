/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#pragma once

#include <stdint.h>

struct USBReportPriority {
    uint64_t digital = 0;
    uint32_t triggers = 0;

    bool operator==(USBReportPriority const &other) const {
        return digital == other.digital && triggers == other.triggers;
    }

    bool operator!=(USBReportPriority const &other) const {
        return !(*this == other);
    }
};

class USBReportScheduler {
public:
    static USBReportScheduler &getInstance();

    // One active full-speed interrupt-IN endpoint, scheduled at USB frame boundaries.
    void configure(uint8_t endpoint);
    void start();
    static void onSof(uint8_t rhport, uint32_t frameCount);
    void reset();
    // Relearn a changed input workload without discarding queued digital edges.
    void resetInputTiming();
    void beginInputProcessing();
    void endInputProcessing();

    bool sendDirectReport(void const *report, uint16_t len,
                          USBReportPriority const &priority,
                          bool allowAnalogScheduling = true);
    // Only reports submitted through this scheduler, not other endpoint traffic.
    bool hasPendingReport() const { return pendingOwned; }
    void onReportComplete();
    void onReportFailed();

private:
    USBReportScheduler() = default;

    void resetPollTiming();
    void noteHostPoll(uint32_t frame);
    bool shouldDefer(USBReportPriority const &priority) const;
    bool replacePendingInBuffer(void const *report, uint16_t len);
    bool tryReplace(void const *report, uint16_t len,
                    USBReportPriority const &priority);
    void noteQueued(USBReportPriority const &priority, uint32_t sofEpoch,
                    bool replaceable);

    uint8_t endpoint = 0;
    uint32_t inputStartUs = 0;
    uint32_t maxInputIntervalUs = 0;
    uint32_t maxInputProcessingUs = 0;
    uint32_t replacementGuardUs = 0;
    volatile uint32_t lastSofUs = 0;
    volatile uint32_t sofEpoch = 0;
    uint32_t lastSofFrame = 0;
    uint32_t lastNakCheckSofEpoch = 0;
    uint32_t lastPollSofEpoch = 0;
    uint32_t pollIntervalFrames = 0;
    bool lastPollValid = false;
    bool pollTimingReady = false;
    uint32_t completedSofEpoch = 0;
    uint32_t pendingSofEpoch = 0;
    USBReportPriority deliveredPriority;
    USBReportPriority pendingPriority;
    bool pendingOwned = false;
    bool pendingPriorityUnchanged = false;
    bool inputTimingStarted = false;
    bool inputTimingReady = false;
    bool replacementTimingReady = false;
    volatile bool sofTimingReady = false;
    volatile bool reportPendingAtSof = false;
    bool completedSofEpochValid = false;
    bool pendingSofEpochValid = false;
};

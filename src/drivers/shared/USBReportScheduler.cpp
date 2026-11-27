/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/shared/USBReportScheduler.h"

#include "tusb.h"
#include "device/usbd_pvt.h"
#include "device/dcd.h"

#include "pico/platform.h"
#include "pico/time.h"

#include "hardware/irq.h"
#include "hardware/structs/usb.h"
#include "hardware/structs/usb_dpram.h"
#include "hardware/sync.h"

#include <cstring>

// The RP USB controller uses full-speed frames. Endpoint bInterval controls
// host polling, not SOF cadence; never use it as a frame deadline.
static constexpr uint32_t USB_FULL_SPEED_FRAME_US = 1000;

// time_us_32() reports whole microseconds. Add one unit when publishing an
// observed duration so truncation cannot turn it into an unsafe lower bound.
static constexpr uint32_t TIME_US_32_RESOLUTION_US = 1;

static inline uint32_t boundedElapsedUs(uint32_t elapsedUs) {
    return elapsedUs == UINT32_MAX ? elapsedUs : elapsedUs + TIME_US_32_RESOLUTION_US;
}

// Consumed by the next class SOF callback on this core, and discarded at reset.
static volatile uint32_t preSofClearNak;

static __force_inline bool unarmedEndpoint(uint8_t ep, uint32_t mask) {
    uint32_t const occupied = USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL | USB_BUF_CTRL_STALL;
    uint32_t const ctrl = usb_dpram->ep_ctrl[ep - 1].in;
    uint32_t const enabled = EP_CTRL_ENABLE_BITS | EP_CTRL_INTERRUPT_ON_NAK;
    return (ctrl & (enabled | EP_CTRL_INTERRUPT_ON_STALL)) == enabled && !(usb_hw->abort & mask) &&
        !(usb_dpram->ep_buf_ctrl[ep].in & (occupied | (occupied << 16)));
}

USBReportScheduler &__not_in_flash_func(USBReportScheduler::getInstance)() {
    static USBReportScheduler instance;
    return instance;
}

void USBReportScheduler::configure(uint8_t endpoint, bool stageReports) {
    reset();
    this->endpoint = endpoint;
    stageReportsAllowed = stageReports;
}

void USBReportScheduler::resetHid(uint8_t rhport) {
    hidd_reset(rhport);
    getInstance().reset();
}

uint16_t USBReportScheduler::openHid(uint8_t rhport, tusb_desc_interface_t const *itf, uint16_t maxLen) {
    uint16_t const len = hidd_open(rhport, itf, maxLen);
    if (len == 0) return 0;

    // hidd_open validates the interface, HID descriptor, and interrupt endpoints.
    uint8_t const *desc = tu_desc_next(tu_desc_next(itf));
    for (uint8_t i = 0; i < itf->bNumEndpoints; ++i, desc = tu_desc_next(desc)) {
        auto const *ep = reinterpret_cast<tusb_desc_endpoint_t const *>(desc);
        if (tu_edpt_dir(ep->bEndpointAddress) == TUSB_DIR_IN) {
            USBReportScheduler &scheduler = getInstance();
            scheduler.configure(ep->bEndpointAddress);
            scheduler.start();
            break;
        }
    }
    return len;
}

bool USBReportScheduler::onHidTransfer(uint8_t rhport, uint8_t epAddr, xfer_result_t result, uint32_t bytes) {
    USBReportScheduler &scheduler = getInstance();
    if (epAddr == scheduler.endpoint) {
        bool const scheduled = scheduler.hasPendingReport();
        if (result == XFER_RESULT_SUCCESS) {
            scheduler.onReportComplete();
        } else {
            scheduler.onReportFailed();
        }
        if (scheduled) return true;
    }
    return hidd_xfer_cb(rhport, epAddr, result, bytes);
}

void USBReportScheduler::start() {
    uint8_t const epNum = tu_edpt_number(endpoint);
    if ((tu_edpt_dir(endpoint) == TUSB_DIR_IN) && epNum != 0 && epNum < USB_NUM_ENDPOINTS) {
        static bool handlerInstalled = false;
        queueNakSupported = rp2040_chip_version() >= 2;
        if (queueNakSupported && !handlerInstalled) {
            irq_add_shared_handler(USBCTRL_IRQ, onQueueNak, PICO_SHARED_IRQ_HANDLER_HIGHEST_ORDER_PRIORITY);
            handlerInstalled = true;
        }
        // Latch polls even while the NAK interrupt is disabled.
        usb_hw->ep_nak_stall_status = 1u << (2 * epNum);
        usb_dpram->ep_ctrl[epNum - 1].in |= EP_CTRL_INTERRUPT_ON_NAK;
        // The class SOF callback runs in the ISR; no queued application SOF event.
        dcd_sof_enable(0, true);
    }
}

void USBReportScheduler::reset() {
    if (stagedClaimed) usbd_edpt_release(0, endpoint);
    stagedLength = 0;
    stagedClaimed = false;
    stagedFlushRequired = false;
    uint32_t const irqState = save_and_disable_interrupts();
    preSofClearNak = 0;
    lastSofUs = 0;
    sofEpoch = 0;
    sofTimingReady = false;
    inputTimingResetPending = false;
    reportPendingAtSof = false;
    queueNakSupported = false;
    lastSofFrame = 0;
    resetPollTiming();
    restore_interrupts(irqState);

    resetInputTiming();
    replacementGuardUs = 0;
    servicedSofEpoch = 0;
    pendingSofEpoch = 0;
    deliveredPriority = {};
    pendingPriority = {};
    pendingOwned = false;
    reportRequired = true;
    pendingPriorityUnchanged = false;
    replacementTimingReady = false;
    servicedSofEpochValid = false;
    pendingSofEpochValid = false;
}

void USBReportScheduler::resetInputTiming() {
    if (stagedLength != 0) stagedFlushRequired = true;
    inputStartUs = 0;
    maxInputIntervalUs = 0;
    maxInputProcessingUs = 0;
    recentInputIntervalUs = 0;
    recentInputProcessingUs = 0;
    inputWindowSofEpoch = 0;
    inputTimingStarted = false;
    inputTimingReady = false;
}

void USBReportScheduler::resetPollTiming() {
    lastPollValid = false;
    pollTimingReady = false;
    pollIntervalFrames = 0;
}

void __not_in_flash_func(USBReportScheduler::noteHostPoll)(uint32_t frame) {
    if (lastPollValid) {
        uint32_t const interval = frame - lastPollSofEpoch;
        if (interval == 0) {
            return;
        }
        // Require two agreeing gaps; a changed cadence immediately falls back.
        pollTimingReady = interval == pollIntervalFrames;
        pollIntervalFrames = interval;
    }
    lastPollSofEpoch = frame;
    lastPollValid = true;
}

void __not_in_flash_func(USBReportScheduler::consumeNak)(uint32_t nakMask, bool observedUnarmed) {
    if (stagedLength != 0) stagedFlushRequired = true;
    usb_hw->ep_nak_stall_status = nakMask; // W1C: leave other endpoints alone.
    if (!sofTimingReady || (usb_hw->ints & USB_INTS_DEV_SOF_BITS)) {
        resetPollTiming();
        return;
    }

    noteHostPoll(sofEpoch);
    // A submission that crossed an unarmed SOF can see the new poll after publication.
    bool const serviced = observedUnarmed || (pendingOwned
        ? pendingSofEpochValid && pendingSofEpoch == sofEpoch && !reportPendingAtSof &&
            servicedSofEpochValid && sofEpoch - servicedSofEpoch == 1
        : !usbd_edpt_busy(0, endpoint));
    if (serviced) {
        servicedSofEpoch = sofEpoch;
        servicedSofEpochValid = true;
    }
}

void __not_in_flash_func(USBReportScheduler::onQueueNak)() {
    USBReportScheduler &scheduler = getInstance();
    uint32_t const flags = usb_hw->ints;
    if (flags & USB_INTS_DEV_SOF_BITS) {
        uint32_t const nakAtIrq = usb_hw->ep_nak_stall_status;
        preSofClearNak = 0;
        // A shared handler can execute on either side of TinyUSB's SOF_RD read.
        // Only the hardware's still-pending SOF establishes the lower time bound.
        if (!(flags & USB_INTS_BUS_RESET_BITS) && scheduler.queueNakSupported &&
            !scheduler.pendingOwned && !scheduler.stagedLength) {
            uint8_t const ep = tu_edpt_number(scheduler.endpoint);
            uint32_t const mask = 1u << (2 * ep);
            if (!(nakAtIrq & mask) && unarmedEndpoint(ep, mask)) preSofClearNak = mask;
        }
    }
    if (!scheduler.queueNakActive || !(flags & USB_INTS_EP_STALL_NAK_BITS)) return;

    uint8_t const epNum = tu_edpt_number(scheduler.endpoint);
    uint32_t const nakMask = 1u << (2 * epNum);
    uint32_t const owned = USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL;
    uint32_t const endpointControl = usb_dpram->ep_ctrl[epNum - 1].in;
    // AVAIL and FULL remain set for an IN packet awaiting a retry.
    if (scheduler.queueNakEpoch == scheduler.sofEpoch &&
        !(usb_hw->ints & (USB_INTS_DEV_SOF_BITS | USB_INTS_BUS_RESET_BITS)) &&
        (endpointControl & EP_CTRL_ENABLE_BITS) && (usb_hw->ep_nak_stall_status & nakMask) &&
        !(usb_hw->abort & nakMask) &&
        !(usb_dpram->ep_buf_ctrl[epNum].in & (owned | (owned << 16)))) {
        scheduler.queueNakObserved = true;
    }
    // The status remains latched for the normal scheduler path.
    hw_clear_bits(&usb_hw->inte, USB_INTE_EP_STALL_NAK_BITS);
}

void __not_in_flash_func(USBReportScheduler::onSof)(uint8_t rhport, uint32_t frameCount) {
    (void)rhport;
    uint32_t const nakAtEntry = usb_hw->ep_nak_stall_status;
    uint32_t const earlyClear = preSofClearNak;
    preSofClearNak = 0;
    USBReportScheduler &scheduler = getInstance();
    uint32_t const now = time_us_32();
    uint8_t const epNum = tu_edpt_number(scheduler.endpoint);
    if ((tu_edpt_dir(scheduler.endpoint) == TUSB_DIR_IN) && epNum != 0 && epNum < USB_NUM_ENDPOINTS) {
        uint32_t const nakMask = 1u << (2 * epNum);
        if (scheduler.stagedLength != 0 && (usb_hw->ep_nak_stall_status & nakMask)) {
            scheduler.stagedFlushRequired = true;
        }
        // A NAK latched after entry belongs to this frame's observation window.
        if (nakAtEntry & nakMask) {
            bool const fresh = earlyClear == nakMask && scheduler.sofTimingReady &&
                ((frameCount - scheduler.lastSofFrame) & USB_SOF_RD_BITS) == 1 &&
                now - scheduler.lastSofUs < 2 * USB_FULL_SPEED_FRAME_US &&
                !scheduler.pendingOwned && !scheduler.stagedLength &&
                !(usb_hw->ints & (USB_INTS_DEV_SOF_BITS | USB_INTS_BUS_RESET_BITS)) &&
                unarmedEndpoint(epNum, nakMask);
            if (!fresh) usb_hw->ep_nak_stall_status = nakMask;
        }
    }
    // Also catch pauses hidden by the USB frame counter's 11-bit wrap.
    if (scheduler.sofTimingReady &&
        (((frameCount - scheduler.lastSofFrame) & USB_SOF_RD_BITS) != 1 ||
         now - scheduler.lastSofUs >= 2 * USB_FULL_SPEED_FRAME_US)) {
        scheduler.resetPollTiming();
        scheduler.inputTimingResetPending = true;
    }
    scheduler.lastSofFrame = frameCount;
    scheduler.reportPendingAtSof = epNum != 0 && epNum < USB_NUM_ENDPOINTS &&
        (usb_dpram->ep_buf_ctrl[epNum].in & (USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL)) ==
            (USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL);
    scheduler.lastSofUs = now;
    ++scheduler.sofEpoch;
    scheduler.sofTimingReady = true;
}

void __not_in_flash_func(USBReportScheduler::beginInputProcessing)() {
    if (endpoint == 0) {
        return;
    }

    uint32_t const irqState = save_and_disable_interrupts();
    uint32_t const now = time_us_32();
    // A missed USB frame invalidates input costs without discarding queued edges.
    if (inputTimingResetPending) {
        resetInputTiming();
        inputTimingResetPending = false;
    }
    uint32_t const nakMask = 1u << (2 * tu_edpt_number(endpoint));
    if (usb_hw->ep_nak_stall_status & nakMask) {
        consumeNak(nakMask);
    }
    uint32_t const windowEpoch = sofEpoch;
    uint32_t const windowFrames = pollTimingReady ? pollIntervalFrames : 1;
    restore_interrupts(irqState);
    if (inputTimingStarted) {
        // Keep peaks from this and the preceding observed poll window. New
        // costs apply immediately; old isolated work expires without a timeout.
        uint32_t const elapsedFrames = windowEpoch - inputWindowSofEpoch;
        if (elapsedFrames >= windowFrames) {
            // The last completed loop may itself have spanned several frames.
            maxInputIntervalUs = recentInputIntervalUs;
            maxInputProcessingUs = recentInputProcessingUs;
            recentInputIntervalUs = 0;
            recentInputProcessingUs = 0;
            inputWindowSofEpoch = windowEpoch;
        }
        uint32_t const intervalUs = boundedElapsedUs(now - inputStartUs);
        if (intervalUs > recentInputIntervalUs) {
            recentInputIntervalUs = intervalUs;
        }
        if (intervalUs > maxInputIntervalUs) {
            maxInputIntervalUs = intervalUs;
        }
    } else {
        inputWindowSofEpoch = windowEpoch;
    }
    inputStartUs = now;
    inputTimingStarted = true;
}

void __not_in_flash_func(USBReportScheduler::endInputProcessing)() {
    if (!inputTimingStarted) {
        return;
    }

    uint32_t const processing = boundedElapsedUs(time_us_32() - inputStartUs);
    if (processing > recentInputProcessingUs) {
        recentInputProcessingUs = processing;
    }
    if (processing > maxInputProcessingUs) {
        maxInputProcessingUs = processing;
    }
    inputTimingReady = maxInputIntervalUs != 0;
    if (stagedLength != 0 && (stagedFlushRequired || !pollTimingReady || !hasSchedulingWindow())) {
        flushStagedReport();
    }
}

bool __not_in_flash_func(USBReportScheduler::shouldDefer)(
        USBReportPriority const &priority) const {
    USBReportPriority const &baseline = hasQueuedReport() ? pendingPriority : deliveredPriority;
    return priority == baseline && hasSchedulingWindow();
}

bool __not_in_flash_func(USBReportScheduler::hasSchedulingWindow)() const {
    if (!inputTimingReady) {
        return false;
    }

    uint32_t const now = time_us_32();
    uint32_t const irqState = save_and_disable_interrupts();
    uint32_t const sofUs = lastSofUs;
    bool const timingReady = sofTimingReady && !(usb_hw->ints & USB_INTS_DEV_SOF_BITS);
    bool const pollReady = pollTimingReady;
    uint32_t const framesSincePoll = sofEpoch - lastPollSofEpoch;
    uint32_t const pollFrames = pollIntervalFrames;
    restore_interrupts(irqState);

    if (!timingReady) {
        return false;
    }

    uint32_t const phaseUs = now - sofUs;
    if (phaseUs >= USB_FULL_SPEED_FRAME_US) {
        return false;
    }

    uint64_t remainingUs = USB_FULL_SPEED_FRAME_US - phaseUs;
    if (!pendingOwned && pollReady) {
        if (framesSincePoll >= pollFrames) {
            return false; // The prediction expired; never wait another period.
        }
        remainingUs += uint64_t{pollFrames - framesSincePoll - 1} * USB_FULL_SPEED_FRAME_US;
    }

    uint32_t const currentProcessingUs = now - inputStartUs;
    uint32_t reportProcessingUs = maxInputProcessingUs;
    if (currentProcessingUs > reportProcessingUs) {
        reportProcessingUs = currentProcessingUs;
    }

    uint32_t replacementUs = replacementGuardUs;
    if (!replacementTimingReady) {
        replacementUs = reportProcessingUs;
    }

    if (replacementUs >= remainingUs ||
        currentProcessingUs >= maxInputIntervalUs) {
        return false;
    }

    // Defer only when recent runtime estimates leave room for another complete
    // acquisition-to-report opportunity before the replacement cutoff.
    // This accounts for the loop tail and tud_task(), rather than treating the
    // shorter input-processing duration as the whole loop interval.
    uint64_t const budgetUs = remainingUs - replacementUs;
    return reportProcessingUs < budgetUs &&
        maxInputIntervalUs - currentProcessingUs < budgetUs - reportProcessingUs;
}

bool __not_in_flash_func(USBReportScheduler::captureQueueTiming)(
        uint32_t &epoch, bool allowReplacement, bool watchForNak) {
    uint32_t irqState = save_and_disable_interrupts();
    if (allowReplacement) {
        if (usb_hw->ints & USB_INTS_DEV_SOF_BITS) {
            // Give a pending SOF one chance to run before the transfer starts.
            restore_interrupts(irqState);
            irqState = save_and_disable_interrupts();
        }
        uint32_t const nakMask = 1u << (2 * tu_edpt_number(endpoint));
        if (usb_hw->ep_nak_stall_status & nakMask) {
            consumeNak(nakMask);
        }
    }

    epoch = sofEpoch;
    bool const replaceable = allowReplacement && sofTimingReady &&
        servicedSofEpochValid && servicedSofEpoch == epoch &&
        !(usb_hw->ints & USB_INTS_DEV_SOF_BITS);
    if (watchForNak && queueNakSupported && !replaceable && sofTimingReady && !reportPendingAtSof &&
        !inputTimingResetPending && servicedSofEpochValid && epoch - servicedSofEpoch == 1 &&
        !(usb_hw->ints & (USB_INTS_DEV_SOF_BITS | USB_INTS_BUS_RESET_BITS))) {
        queueNakEpoch = epoch;
        queueNakObserved = false;
        queueNakActive = true;
        hw_set_bits(&usb_hw->inte, USB_INTE_EP_STALL_NAK_BITS);
    }
    restore_interrupts(irqState);
    return replaceable;
}

bool __not_in_flash_func(USBReportScheduler::sendDirectReport)(
        void const *report, uint16_t len, USBReportPriority const &priority,
        bool allowAnalogScheduling) {
    if (len > sizeof(stagedReport) || (len != 0 && !report)) return false;
    if (stagedLength != 0) {
        if (!allowAnalogScheduling || len != stagedLength || !canCoalesce(priority)) {
            // Deliver an earlier edge before accepting its reversal or a mixed transition.
            flushStagedReport();
            return false;
        }
        memcpy(stagedReport, report, len);
        pendingPriority = priority;
        pendingPriorityUnchanged = priority == deliveredPriority;
        return true;
    }

    if (stageReportsAllowed && allowAnalogScheduling && !reportRequired &&
        len != 0 && !pendingOwned &&
        pollTimingReady && hasSchedulingWindow() && tud_ready() &&
        endpoint != 0 && !usbd_edpt_busy(0, endpoint) && usbd_edpt_claim(0, endpoint)) {
        // Reserve the endpoint so protocol replies cannot overtake accepted input.
        memcpy(stagedReport, report, len);
        stagedClaimed = true;
        pendingPriority = priority;
        pendingPriorityUnchanged = priority == deliveredPriority;
        uint32_t const irqState = save_and_disable_interrupts();
        stagedFlushRequired = false;
        stagedLength = len;
        restore_interrupts(irqState);
        return true;
    }

    if (allowAnalogScheduling && !reportRequired && shouldDefer(priority)) {
        return false;
    }

    if (!pendingOwned && tud_ready() && endpoint != 0 && !usbd_edpt_busy(0, endpoint) &&
        usbd_edpt_claim(0, endpoint)) {
        if (len != 0) memcpy(stagedReport, report, len);
        uint32_t queueSofEpoch;
        bool const watchForNak = allowAnalogScheduling && !reportRequired && priority == deliveredPriority;
        bool replaceable = captureQueueTiming(queueSofEpoch, allowAnalogScheduling, watchForNak);
        if (watchForNak && !replaceable && queueSofEpoch != sofEpoch) {
            // A SOF can interrupt captureQueueTiming immediately before it returns.
            // The transfer is still unpublished. Bracket this frame's poll once.
            uint32_t const irqState = save_and_disable_interrupts();
            uint8_t const ep = tu_edpt_number(endpoint);
            uint32_t const mask = 1u << (2 * ep);
            if (sofEpoch - queueSofEpoch == 1 && sofTimingReady && !inputTimingResetPending &&
                !reportPendingAtSof && !queueNakActive && queueNakSupported &&
                servicedSofEpochValid && servicedSofEpoch == queueSofEpoch &&
                !pendingOwned && !reportRequired && tud_ready() &&
                !(usb_hw->ints & (USB_INTS_DEV_SOF_BITS | USB_INTS_BUS_RESET_BITS)) &&
                unarmedEndpoint(ep, mask)) {
                replaceable = captureQueueTiming(queueSofEpoch, true, true);
            }
            restore_interrupts(irqState);
        }

        bool const queued = usbd_edpt_xfer(0, endpoint, stagedReport, len, false);
        uint32_t const queuedIrqState = save_and_disable_interrupts();
        if (queueNakActive) {
            hw_clear_bits(&usb_hw->inte, USB_INTE_EP_STALL_NAK_BITS);
            queueNakActive = false;
            uint8_t const epNum = tu_edpt_number(endpoint);
            uint32_t const nakMask = 1u << (2 * epNum);
            if (queued && queueNakObserved && queueSofEpoch == sofEpoch && !inputTimingResetPending &&
                !(usb_hw->ints & (USB_INTS_DEV_SOF_BITS | USB_INTS_BUS_RESET_BITS)) &&
                (usb_hw->ep_nak_stall_status & nakMask) &&
                (usb_dpram->ep_ctrl[epNum - 1].in & EP_CTRL_ENABLE_BITS)) {
                consumeNak(nakMask, true);
                replaceable = servicedSofEpochValid && servicedSofEpoch == queueSofEpoch;
            }
        }
        if (queued) noteQueued(priority, queueSofEpoch, replaceable);
        restore_interrupts(queuedIrqState);
        if (queued) return true;
        usbd_edpt_release(0, endpoint);
    }

    return allowAnalogScheduling && tryReplace(report, len, priority);
}

bool __not_in_flash_func(USBReportScheduler::tryReplace)(
        void const *report, uint16_t len, USBReportPriority const &priority) {
    if (!pendingOwned || !tud_ready() || endpoint == 0 ||
        priority == pendingPriority) {
        return false;
    }

    if (!canCoalesce(priority)) return false;

    bool const replaced = replacePendingInBuffer(report, len);
    if (replaced) {
        // A later CLEAR_FEATURE can re-arm this packet from TinyUSB's original pointer.
        if (len != 0) memcpy(stagedReport, report, len);
        pendingPriority = priority;
        pendingPriorityUnchanged = false;
    }
    return replaced;
}

bool __not_in_flash_func(USBReportScheduler::canCoalesce)(
        USBReportPriority const &priority) const {
    if (pendingPriorityUnchanged || priority == pendingPriority) return true;

    bool const samePendingTriggers =
        pendingPriority.triggers == priority.triggers;
    bool const sameDeliveredTriggers = samePendingTriggers &&
        deliveredPriority.triggers == pendingPriority.triggers;
    bool const monotonicPress = samePendingTriggers &&
        (pendingPriority.digital & priority.digital) == pendingPriority.digital &&
        // Do not re-add a delivered bit that the pending report released.
        (priority.digital & deliveredPriority.digital & ~pendingPriority.digital) == 0;
    bool const monotonicRelease = sameDeliveredTriggers &&
        (pendingPriority.digital & deliveredPriority.digital) == pendingPriority.digital &&
        (priority.digital & pendingPriority.digital) == priority.digital;

    return monotonicPress || monotonicRelease;
}

bool __not_in_flash_func(USBReportScheduler::flushStagedReport)() {
    stagedFlushRequired = true;
    if (stagedLength == 0 || !tud_ready() || usbd_edpt_busy(0, endpoint)) return false;
    if (!stagedClaimed) stagedClaimed = usbd_edpt_claim(0, endpoint);
    if (!stagedClaimed) return false;

    uint32_t queueSofEpoch;
    bool const replaceable = captureQueueTiming(queueSofEpoch, true);

    if (!usbd_edpt_xfer(0, endpoint, stagedReport, stagedLength, false)) {
        // TinyUSB releases a failed transfer's claim; retain the accepted bytes.
        stagedClaimed = usbd_edpt_claim(0, endpoint);
        return false;
    }
    stagedLength = 0;
    stagedClaimed = false;
    stagedFlushRequired = false;
    uint32_t const queuedIrqState = save_and_disable_interrupts();
    noteQueued(pendingPriority, queueSofEpoch, replaceable);
    restore_interrupts(queuedIrqState);
    return true;
}

void __not_in_flash_func(USBReportScheduler::noteQueued)(
        USBReportPriority const &priority, uint32_t queuedSofEpoch,
        bool replaceable) {
    if (replaceable && sofEpoch - queuedSofEpoch == 1 && !inputTimingResetPending &&
        !reportPendingAtSof && !(usb_hw->ints & USB_INTS_DEV_SOF_BITS)) {
        uint32_t const armed = USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL;
        // A completed transfer awaiting its callback no longer owns the USB buffer.
        if ((usb_dpram->ep_buf_ctrl[tu_edpt_number(endpoint)].in & armed) == armed) {
            queuedSofEpoch = sofEpoch;
        }
    }
    pendingPriority = priority;
    pendingPriorityUnchanged = priority == deliveredPriority;
    pendingOwned = true;
    reportRequired = false;
    pendingSofEpoch = queuedSofEpoch;
    pendingSofEpochValid = replaceable && sofEpoch == queuedSofEpoch;
}

void USBReportScheduler::onReportComplete() {
    if (endpoint == 0) {
        return;
    }

    uint32_t const irqState = save_and_disable_interrupts();
    servicedSofEpoch = sofEpoch;
    // tud_task() may dispatch a completion after its USB frame has ended.
    // A report queued earlier must still have belonged to hardware at this
    // frame's SOF; otherwise its actual completion frame is ambiguous.
    servicedSofEpochValid = sofTimingReady && pendingOwned &&
        (pendingSofEpoch == sofEpoch || reportPendingAtSof) &&
        !(usb_hw->ints & USB_INTS_DEV_SOF_BITS);
    if (servicedSofEpochValid) {
        noteHostPoll(sofEpoch);
    } else {
        resetPollTiming();
    }
    restore_interrupts(irqState);

    if (pendingOwned) {
        deliveredPriority = pendingPriority;
        pendingOwned = false;
        pendingPriorityUnchanged = false;
        pendingSofEpochValid = false;
    }
}

void USBReportScheduler::onReportFailed() {
    if (endpoint == 0) {
        return;
    }

    servicedSofEpochValid = false;
    uint32_t const irqState = save_and_disable_interrupts();
    resetPollTiming();
    restore_interrupts(irqState);
    if (!pendingOwned) {
        return;
    }
    reportRequired = true;
    pendingPriority = deliveredPriority;
    pendingOwned = false;
    pendingPriorityUnchanged = false;
    pendingSofEpochValid = false;
}

bool __no_inline_not_in_flash_func(USBReportScheduler::replacePendingInBuffer)(
        void const *report, uint16_t len) {
    uint8_t const epNum = tu_edpt_number(endpoint);
    if (rp2040_chip_version() < 2 || tu_edpt_dir(endpoint) != TUSB_DIR_IN || epNum == 0 ||
        epNum >= USB_NUM_ENDPOINTS || len > USB_MAX_PACKET_SIZE) {
        return false;
    }

    uint32_t const irqState = save_and_disable_interrupts();
    uint32_t const attemptStartUs = time_us_32();
    uint32_t guardUs = replacementGuardUs;
    if (!replacementTimingReady) {
        guardUs = maxInputProcessingUs;
        uint32_t const currentProcessingUs = attemptStartUs - inputStartUs;
        if (currentProcessingUs > guardUs) {
            guardUs = currentProcessingUs;
        }
    }
    uint32_t const phaseUs = attemptStartUs - lastSofUs;
    bool const inWindow = inputTimingReady && sofTimingReady &&
        pendingSofEpochValid && servicedSofEpochValid &&
        pendingSofEpoch == sofEpoch && servicedSofEpoch == sofEpoch &&
        !(usb_hw->ints & USB_INTS_DEV_SOF_BITS) &&
        phaseUs < USB_FULL_SPEED_FRAME_US &&
        guardUs < USB_FULL_SPEED_FRAME_US - phaseUs;
    if (!inWindow) {
        restore_interrupts(irqState);
        return false;
    }

    uint32_t const abortMask = 1u << (2 * epNum);
    hw_set_bits(&usb_hw->abort, abortMask);
    while ((usb_hw->abort_done & abortMask) != abortMask) {}

    volatile uint32_t * const bufferControl = &usb_dpram->ep_buf_ctrl[epNum].in;
    uint32_t const savedControl = *bufferControl;
    bool const sofArrived = usb_hw->ints & USB_INTS_DEV_SOF_BITS;
    bool const pending = !sofArrived &&
        (savedControl & (USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL)) ==
            (USB_BUF_CTRL_AVAIL | USB_BUF_CTRL_FULL) &&
        (savedControl & USB_BUF_CTRL_LEN_MASK) == len;
    if (pending) {
        uint32_t const endpointControl = usb_dpram->ep_ctrl[epNum - 1].in;
        uint8_t * const dpramBuffer = reinterpret_cast<uint8_t *>(usb_dpram) +
            (endpointControl & 0xffffu);
        *bufferControl = 0;
        if (len != 0) memcpy(dpramBuffer, report, len);
        *bufferControl = savedControl & ~USB_BUF_CTRL_AVAIL;
        busy_wait_at_least_cycles(12);
        *bufferControl = savedControl;
    }

    hw_clear_bits(&usb_hw->abort_done, abortMask);
    hw_clear_bits(&usb_hw->abort, abortMask);
    bool const crossedSof = usb_hw->ints & USB_INTS_DEV_SOF_BITS;
    uint32_t const replacementDurationUs = boundedElapsedUs(
        time_us_32() - attemptStartUs);
    if (replacementDurationUs > replacementGuardUs) {
        replacementGuardUs = replacementDurationUs;
    }
    if (crossedSof) {
        // The SOF callback timestamp can lag the wire boundary. If hardware
        // observes a crossing, retain the actual remaining phase as the new
        // runtime guard so the same boundary cannot be crossed again.
        uint32_t const crossedBoundaryGuardUs = boundedElapsedUs(
            USB_FULL_SPEED_FRAME_US - phaseUs);
        if (crossedBoundaryGuardUs > replacementGuardUs) {
            replacementGuardUs = crossedBoundaryGuardUs;
        }
    }
    if (pending) {
        replacementTimingReady = true;
    }
    restore_interrupts(irqState);
    return pending;
}

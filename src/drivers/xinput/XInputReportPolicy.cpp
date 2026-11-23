/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/xinput/XInputDriver.h"
#include "drivers/shared/USBReportScheduler.h"
#include "drivers/shared/InputReportPolicy.h"

void XInputDriver::updateAnalogScheduling(uint32_t appliedProfileNumber) {
    USBReportScheduler::getInstance().resetInputTiming();
    analogSchedulingProfile = appliedProfileNumber;
    analogSchedulingAllowed = false;

    // These driver mappings are initialized once, not on profile changes.
    if ((deviceType == INPUT_MODE_DEVICE_TYPE_WHEEL &&
         (buttonSteerLeft->pinMask || buttonSteerRight->pinMask)) ||
        (deviceType == INPUT_MODE_DEVICE_TYPE_GUITAR &&
         (buttonWhammy->pinMask || buttonTilt->pinMask))) {
        return;
    }

    analogSchedulingAllowed = !hasDigitalAxisMappings();
}

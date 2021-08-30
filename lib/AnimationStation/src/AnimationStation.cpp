/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "AnimationStation.hpp"

AnimationStation::AnimationStation(int numPixels) {
    this->numPixels = numPixels;
}

void AnimationStation::SetStaticColor(bool defaultAnimation, uint32_t color, int firstPixel, int lastPixel = -1) {
    if (lastPixel < 0) 
        lastPixel = this->numPixels - 1;

    this->animations.push_back(new StaticColor(firstPixel, lastPixel, defaultAnimation, color));
}

void AnimationStation::Animate() {
    for (auto & element : this->animations) {
        element->Animate(this->frame);
    }
}


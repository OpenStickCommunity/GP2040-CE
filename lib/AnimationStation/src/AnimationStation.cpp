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
#include "Effects/Rainbow.hpp"
#include "Effects/StaticColor.hpp"
#include "Effects/Chase.hpp"

AnimationStation::AnimationStation(int numPixels) {
    this->numPixels = numPixels;
}

void AnimationStation::SetStaticColor(bool defaultAnimation, uint32_t color, int firstPixel, int lastPixel = -1) {
    if (lastPixel < 0) 
        lastPixel = this->numPixels - 1;

    this->animations.push_back(new StaticColor(firstPixel, lastPixel, defaultAnimation, color));
}

void AnimationStation::SetRainbow(bool defaultAnimation, int firstPixel, int lastPixel = -1, int cycleTime = 50) {
    if (lastPixel < 0) 
        lastPixel = this->numPixels - 1;

    this->animations.push_back(new Rainbow(firstPixel, lastPixel, cycleTime, defaultAnimation));
}

void AnimationStation::SetChase(bool defaultAnimation, int firstPixel, int lastPixel = -1, int cycleTime = 50) {
    if (lastPixel < 0) 
        lastPixel = this->numPixels - 1;

    this->animations.push_back(new Chase(firstPixel, lastPixel, cycleTime, defaultAnimation));
}

void AnimationStation::Animate() {
    for (auto & element : this->animations) {
        element->Animate(this->frame);
    }
}


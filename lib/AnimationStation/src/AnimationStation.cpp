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

float AnimationStation::brightness = 0;
absolute_time_t AnimationStation::nextBrightnessChange = 0;

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

void AnimationStation::SetBrightness(float brightness) {
    AnimationStation::brightness = brightness;
}

void AnimationStation::DecreaseBrightness() {
    if (!time_reached(AnimationStation::nextBrightnessChange)) {
        return;
    }

    float newBrightness = AnimationStation::brightness;
    newBrightness -= 0.01;

    if (newBrightness < 0.0) {
        newBrightness = 0.0;
    }

    AnimationStation::brightness = newBrightness;
    AnimationStation::nextBrightnessChange = make_timeout_time_ms(50);
}

void AnimationStation::IncreaseBrightness() {
    if (!time_reached(AnimationStation::nextBrightnessChange)) {
        return;
    }

    float newBrightness = AnimationStation::brightness;
    newBrightness += 0.01;

    if (newBrightness > 1.0) {
        newBrightness = 1.0;
    }

    AnimationStation::brightness = newBrightness;    
    AnimationStation::nextBrightnessChange = make_timeout_time_ms(50);

}

uint32_t AnimationStation::RGB(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r * AnimationStation::brightness) << 8) |
            ((uint32_t) (g * AnimationStation::brightness) << 16) |
            (uint32_t) (b * AnimationStation::brightness);
}

uint32_t AnimationStation::Wheel(uint8_t pos) {
  pos = 255 - pos;
  if (pos < 85) {
    return AnimationStation::RGB(255 - pos * 3, 0, pos * 3);
  } else if (pos < 170) {
    pos -= 85;
    return AnimationStation::RGB(0, pos * 3, 255 - pos * 3);
  } else {
    pos -= 170;
    return AnimationStation::RGB(pos * 3, 255 - pos * 3, 0);
  }
}



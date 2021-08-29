/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "AnimationStation.hpp"

AnimationStation::AnimationStation(int numPixels) {
    this->numPixels = numPixels;
}

void AnimationStation::StaticColor(uint32_t color) {
    for (int i = 0; i < this->numPixels; ++i) {
        this->frame[i] = color;
    }
}

// RED = (255, 0, 0)
// YELLOW = (255, 150, 0)
// GREEN = (0, 255, 0)
// CYAN = (0, 255, 255)
// BLUE = (0, 0, 255)
// PURPLE = (180, 0, 255)

// WHEEL_FRAMES = 64
// WHEEL_TOTAL_TIME = 1.4
// WHEEL_FRAME_TIME = (WHEEL_TOTAL_TIME) / WHEEL_FRAMES

// class Leds:
//   def __init__(self, pixels, pixel_group):
//     self.pixels = pixels
//     self.pixel_group = pixel_group
//     self.reset()
  
//   def reset(self):
//     self.current_frame = 0
//     self.last_frame_time = None
//     self.animation = None
//     self.color((0, 0, 0))
    
//   def start(self, animation):
//     if self.current_frame < 1:
//       self.current_frame = 0
//       self.last_frame_time = None
//       self.animation = animation
//     else:
//       self.current_frame = 0

//   def color(self, color):
//     for pixel in self.pixel_group:
//       self.pixels[pixel] = color

//   def animate(self):
//     if self.animation == "Wheel":
//       self.wheel()

//   def wheel(self):
//     if self.current_frame > WHEEL_FRAMES:
//       self.reset()
//       return
    
//     if self.last_frame_time == None or time.monotonic() - self.last_frame_time > WHEEL_FRAME_TIME:
//       self.color(self.wheel_position())
//       self.last_frame_time = time.monotonic() 
//       self.current_frame = self.current_frame + 1

//   def wheel_position(self):
//     conversion = 255 / WHEEL_FRAMES
//     pos = self.current_frame * conversion

//     if pos < 0 or pos > 255:
//         return (0, 0, 0)
//     if pos < 85:
//         return (255 - pos * 3, pos * 3, 0)
//     if pos < 170:
//         pos -= 85
//         return (0, 255 - pos * 3, pos * 3)
//     pos -= 170
//     return (pos * 3, 0, 255 - pos * 3)
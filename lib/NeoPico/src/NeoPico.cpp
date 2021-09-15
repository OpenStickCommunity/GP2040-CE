/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "NeoPico.hpp"

void NeoPico::PutPixel(uint32_t pixel_grb) {
  pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

NeoPico::NeoPico(int ledPin, int numPixels) : numPixels(numPixels) {
  PIO pio = pio0;
  int sm = 0;
  uint offset = pio_add_program(pio, &ws2812_program);
  ws2812_program_init(pio, sm, offset, ledPin, 800000, false);
  this->Clear();
  sleep_ms(10);
}

// void NeoPico::SetPixel(int pixel, uint32_t color) {
//   if (pixels.size() > 0) {
//     for (size_t i = 0; i != pixels[pixel].positions.size(); i++) {
//       this->frame[pixels[pixel].positions[i]] = color;
//     }
//   } else {
//     this->frame[pixel] = color;
//   }
// }

void NeoPico::Clear() {
  memset(frame, 0, sizeof(frame));
}

void NeoPico::SetFrame(uint32_t newFrame[100]) {
  memcpy(frame, newFrame, sizeof(frame));
}

void NeoPico::Show() {
  for (int i = 0; i < this->numPixels; ++i) {
     this->PutPixel(this->frame[i]);
  }
  sleep_ms(10);
}


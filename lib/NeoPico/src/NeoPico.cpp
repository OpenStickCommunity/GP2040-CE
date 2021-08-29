/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * Modified by Jonathan Barket - 2021
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "NeoPico.hpp"

void NeoPico::PutPixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

uint32_t NeoPico::RGB(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

NeoPico::NeoPico(int ledPin, int numPixels) {
    PIO pio = pio0;
    int sm = 0;
    this->numPixels = numPixels;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, ledPin, 800000, false);
    this->Clear();
    sleep_ms(10);
}

void NeoPico::SetPixel(int pixel, uint32_t color) {
  this->frame[pixel] = color;
}

void NeoPico::Clear() {
  for (int i = 0; i < this->numPixels; ++i) {
    this->frame[i] = 0;
  }
}

void NeoPico::SetFrame(uint8_t newFrame[100]) {
  for (int i = 0; i < 100; i++) {
    this->frame[i] = newFrame[i];
  }
}

void NeoPico::Show() {
  for (int i = 0; i < this->numPixels; ++i) {
     this->PutPixel(this->frame[i]);
  }
  sleep_ms(10);
}


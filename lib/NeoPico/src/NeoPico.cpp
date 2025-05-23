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
#include "NeoPico.h"

NeoPico::NeoPico(){
}

LEDFormat NeoPico::GetFormat() {
  return format;
}

void NeoPico::PutPixel(uint32_t pixelData) {
  switch (format) {
    case LED_FORMAT_GRB:
    case LED_FORMAT_RGB:
      pio_sm_put_blocking(pio, stateMachine, pixelData << 8u);
      break;
    case LED_FORMAT_GRBW:
    case LED_FORMAT_RGBW:
      pio_sm_put_blocking(pio, stateMachine, pixelData);
      break;
  }
}

void NeoPico::Setup(int ledPin, int inNumPixels, LEDFormat inFormat, PIO inPio){
  format = inFormat;
  pio = inPio;
  numPixels = inNumPixels;
  stateMachine = 0;
  uint offset = pio_add_program(pio, &ws2812_program);
  bool rgbw = (format == LED_FORMAT_GRBW) || (format == LED_FORMAT_RGBW);
  ws2812_program_init(pio, stateMachine, offset, ledPin, 800000, rgbw);
  this->Clear();
}

void NeoPico::Clear() {
  memset(frame, 0, sizeof(frame));
}

void NeoPico::SetFrame(uint32_t * newFrame) {
  memcpy(frame, newFrame, sizeof(frame));
}

void NeoPico::Show() {
  for (int i = 0; i < this->numPixels; ++i) {
     this->PutPixel(this->frame[i]);
  }
}

void NeoPico::Off() {
  Clear();
  for (int i = 0; i < this->numPixels; ++i) {
     this->PutPixel(this->frame[i]);
  }
}

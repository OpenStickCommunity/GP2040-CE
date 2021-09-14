#include "Chase.hpp"

int Chase::defaultCycleTime = 85;

Chase::Chase(std::vector<Pixel> pixels) : Animation(pixels) {
  this->cycleTime = Chase::defaultCycleTime;
}

void Chase::Animate(RGB (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (size_t i = 0; i != pixels.size(); i++) {
    if (this->IsChasePixel(pixels[i].index)) {
      RGB color = RGB::wheel(this->WheelFrame(i));
      for (size_t j = 0; j != pixels[i].positions.size(); j++) {
        frame[pixels[i].positions[j]] = color;
      }
    } else {
      for (size_t j = 0; j != pixels[i].positions.size(); j++) {
        frame[pixels[i].positions[j]] = ColorBlack;
      }
    }
  }

  currentPixel++;

  if (currentPixel > pixels.size() - 1) {
    currentPixel = 0;
  }

  if (reverse) {
    currentFrame--;

    if (currentFrame < 0) {
      currentFrame = 1;
      reverse = false;
      this->currentLoop++;
    }
  }
  else {
    currentFrame++;

    if (currentFrame > 255) {
      currentFrame = 254;
      reverse = true;
    }
  }

  this->nextRunTime = make_timeout_time_ms(this->cycleTime);
}

bool Chase::IsChasePixel(int i) {
  if (i == this->currentPixel || i == (this->currentPixel - 1) || i == (this->currentPixel - 2)) {
    return true;
  }

  return false;
}

int Chase::WheelFrame(int i) {
  int frame = this->currentFrame;

  if (i == (this->currentPixel - 1) % pixels.size()) {
    if (this->reverse) {
      frame = frame + 16;
    }
    else {
      frame = frame - 16;
    }
  }

  if (i == (this->currentPixel - 2) % pixels.size()) {
    if (this->reverse) {
      frame = frame + 32;
    }
    else {
      frame = frame - 32;
    }
  }

  if (frame < 0) {
    return 0;
  }

  return frame;
}

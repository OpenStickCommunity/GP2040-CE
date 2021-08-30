#include "Chase.hpp"
#include "Helpers.hpp"

void Chase::Animate(uint32_t (&frame)[100]) {
  if (!time_reached(this->nextRunTime)) {
    return;
  }

  for (int i = this->firstPixel; i < this->lastPixel + 1; ++i) {
    if (this->IsChasePixel(i)) {
      frame[i] = Helpers::Wheel(this->WheelFrame(i));
    } else {
      frame[i] = 0;
    }
  }

  currentPixel++;

  if (currentPixel > this->lastPixel) {
    currentPixel = this->firstPixel;
  }

  if (reverse) {
    currentFrame--;

    if (currentFrame < 0) {
      currentFrame = 1;
      reverse = false;
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
  if (i == this->currentPixel || i == (this->currentPixel - 1) % this->totalPixels || i == (this->currentPixel - 2) % this->totalPixels) {
    return true; 
  }

  return false;
}

int Chase::WheelFrame(int i) {
  int frame = this->currentFrame;

  if (i == (this->currentPixel - 1) % totalPixels) {
    if (this->reverse) {
      frame = frame + 16;
    }
    else {
      frame = frame - 16;
    }
  }
  
  if (i == (this->currentPixel - 2) % totalPixels) {
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
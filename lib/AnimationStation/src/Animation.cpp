#include "Animation.hpp"

LEDFormat Animation::format;

Animation::Animation(PixelMatrix &matrix) : matrix(&matrix) {
  this->totalLoops = 1;
}

bool Animation::isComplete() {
  if (this->baseAnimation || this->currentLoop < this->totalLoops) {
    return false;
  }

  return true;
}

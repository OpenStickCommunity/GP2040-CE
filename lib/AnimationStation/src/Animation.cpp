#include "Animation.hpp"

Animation::Animation(std::vector<Pixel> pixels) : pixels(pixels) {
  this->totalLoops = 1;
}

bool Animation::isComplete() {
  if (this->baseAnimation || this->currentLoop < this->totalLoops) {
    return false;
  }

  return true;
}

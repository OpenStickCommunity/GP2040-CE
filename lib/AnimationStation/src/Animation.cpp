#include <stdio.h>
#include <stdlib.h>

#include "Animation.hpp"

int Animation::defaultFirstPixel = 0;
int Animation::defaultLastPixel = 0;

bool Animation::isComplete() {
  if (this->baseAnimation || this->currentLoop < this->totalLoops) {
    return false;
  }

  return true;
}
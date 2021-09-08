#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <stdint.h>

typedef enum
{
	STATIC,
	RAINBOW,
	CHASE,
} AnimationMode;

class Animation {
public:
  Animation() {
    this->firstPixel = Animation::defaultFirstPixel;
    this->lastPixel = Animation::defaultLastPixel;
    this->totalLoops = 1;
  }
  virtual void Animate(uint32_t (&frame)[100]);
  AnimationMode mode;

  bool isComplete();

  static void SetDefaultPixels(int firstPixel, int lastPixel) {
    Animation::defaultFirstPixel = firstPixel;
    Animation::defaultLastPixel = lastPixel;
  }
protected:
	static int defaultFirstPixel;
  static int defaultLastPixel;

  int firstPixel;
  int lastPixel;
  int currentLoop = 0;
  int totalLoops;
  bool baseAnimation = true;
};

#endif

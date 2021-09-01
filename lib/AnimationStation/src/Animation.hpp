#ifndef _ANIMATION_H_
#define _ANIMATION_H_

 #include <stdint.h>

class Animation {
public:
  Animation() {

  }
  Animation(int firstPixel, int lastPixel, bool defaultAnimation) {
    this->firstPixel = firstPixel;
    this->lastPixel = lastPixel;
    this->defaultAnimation = defaultAnimation;
  }
  virtual void Animate(uint32_t (&frame)[100]);
protected:
	int firstPixel;
  int lastPixel;
  bool defaultAnimation = false;
};

#endif
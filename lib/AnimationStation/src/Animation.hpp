#ifndef _ANIMATION_H_
#define _ANIMATION_H_

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

class StaticColor : public Animation {
public:
  StaticColor() : Animation() {

  }
  StaticColor(int firstPixel, int lastPixel, bool defaultAnimation, uint32_t color) : Animation(firstPixel, lastPixel, defaultAnimation) {
    this->color = color;
  }
  void Animate(uint32_t (&frame)[100]);
protected:
  uint32_t color;
};

class Rainbow : public Animation {
public:
  Rainbow() : Animation() {
  }
  Rainbow(int firstPixel, int lastPixel, bool defaultAnimation) : Animation(firstPixel, lastPixel, defaultAnimation) {
  }
  void Animate(uint32_t (&frame)[100]);
protected:
  int currentFrame = 0;
  bool reverse = false;
  uint32_t nextRunTime = 0;
};

#endif
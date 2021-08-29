#ifndef _ANIMATION_STATION_H_
#define _ANIMATION_STATION_H_

typedef enum
{
	OFF,
	STATIC,
	WHEEL
} AnimationEffect;

struct ActiveAnimation
{
	const AnimationEffect effect;
	const int startIndex;
	const int activeFrame;
};

class AnimationStation
{
public:
  AnimationStation(int numPixels);
  void StaticColor();
  void Clear();
private:
  int numPixels = 0;
  uint8_t frame[100];
};

#endif
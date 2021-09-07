#include "ConfigStorage.hpp"

AnimationMode ConfigStorage::getBaseAnimation() {
	AnimationMode mode = RAINBOW;
	EEPROM.get(STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX, mode);
	return mode;
}

void ConfigStorage::setBaseAnimation(AnimationMode mode)
{
	EEPROM.set(STORAGE_LEDS_BASE_ANIMATION_MODE_INDEX, mode);
}

uint8_t ConfigStorage::getBrightness() {
	uint8_t brightness = LEDS_BRIGHTNESS;
  EEPROM.get(STORAGE_LEDS_BRIGHTNESS_INDEX, brightness);
	return brightness;
}

void ConfigStorage::setBrightness(uint8_t brightness)
{
	EEPROM.set(STORAGE_LEDS_BRIGHTNESS_INDEX, brightness);
}

void ConfigStorage::setup() {
	AnimationStation::SetBrightness(this->getBrightness() / 100.0);
	Animation::SetDefaultPixels(LEDS_BASE_ANIMATION_FIRST_PIXEL, LEDS_BASE_ANIMATION_LAST_PIXEL);
	StaticColor::SetDefaultColor(LEDS_STATIC_COLOR_COLOR);
}

void ConfigStorage::commit()
{
	EEPROM.commit();
}

/* We don't want to couple our event calls directly to what AS is doing. That means we need to
  let it handle its business, and then afterwards save any changes we find. */
void ConfigStorage::save(AnimationStation as) {
  // bool dirty = false;

  // if (as.brightness * 100 != this->getBrightness()) {
    // this->setBrightness(as.brightness * 100);
    // dirty = true;
  // }

  // if (as.animations.size() > 0 && as.animations.at(0)->mode != this->getBaseAnimation()) {
    // this->setBaseAnimation(as.animations.at(0)->mode);
    // dirty = true;
  // }

  // if (dirty) {
    // this->commit();
  // }
}


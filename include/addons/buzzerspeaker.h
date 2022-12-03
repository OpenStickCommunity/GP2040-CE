

#ifndef BUZZER_H_
#define BUZZER_H_

#include <string>
#include "hardware/pwm.h"
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"

#ifndef HAS_BUZZER_SPEAKER
#define HAS_BUZZER_SPEAKER -1
#endif

#ifndef BUZZER_PIN
#define BUZZER_PIN -1
#endif

#ifndef BUZZER_VOLUME
#define BUZZER_VOLUME 100
#endif

#ifndef BUZZER_SPEED
#define BUZZER_SPEED 100
#endif

// Buzzer Speaker Module
#define BuzzerSpeakerName "BuzzerSpeaker"

// Buzzer Speaker
class BuzzerSpeakerAddon : public GPAddon
{
public:
	virtual bool available();  // GPAddon
	virtual void setup();
	virtual void process();
	virtual std::string name() { return BuzzerSpeakerName; }
	uint8_t buzzerPinSlice;
	uint8_t buzzerPinChannel;
};

#endif
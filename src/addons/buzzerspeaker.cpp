#include "addons/buzzerspeaker.h"
#include "enums.h"
#include "helper.h"
#include "storagemanager.h"
#include "pico/stdlib.h"
#include "bitmaps.h"

const uint LED_PIN = 25;

bool BuzzerSpeakerAddon::available() {
	// BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	// return boardOptions.hasBuzzerSpeaker && 
	// 	boardOptions.buzzerPin != (uint8_t)-1;
	return true;
}

void BuzzerSpeakerAddon::setup() {
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	
	// begin - test
	boardOptions.hasBuzzerSpeaker = true;
	boardOptions.buzzerPin = 14;
	Storage::getInstance().setBoardOptions(boardOptions);
	// end - test

	gpio_set_function(boardOptions.buzzerPin, GPIO_FUNC_PWM);
	buzzerPinSlice = pwm_gpio_to_slice_num (boardOptions.buzzerPin); 
	buzzerPinChannel = pwm_gpio_to_channel (boardOptions.buzzerPin);
}

void BuzzerSpeakerAddon::process() {
	bool configMode = Storage::getInstance().GetConfigMode();

	if (configMode == false) {
		Tone song[] = {
			NOTE_E5,
			NOTE_G5,
			NOTE_A5,
			NOTE_PAUSE,
			NOTE_E5,
			NOTE_G5,
			NOTE_B5,
			NOTE_A5,
			NOTE_PAUSE,
			NOTE_E5,
			NOTE_G5,
			NOTE_A5,
			NOTE_PAUSE,
			NOTE_G5,
			NOTE_E5
		};

		for (Tone tone : song)
			playTone(tone,300);
	}
}

void BuzzerSpeakerAddon::playTone(Tone tone, uint16_t durationMs) {
	if (tone != NOTE_PAUSE) {
		pwmSetFreqDuty(buzzerPinSlice, buzzerPinChannel, tone, 75);
		pwm_set_enabled (buzzerPinSlice, true);
	}
	sleep_ms(durationMs);
	pwm_set_enabled (buzzerPinSlice, false);
}

uint32_t BuzzerSpeakerAddon::pwmSetFreqDuty(uint slice, uint channel, uint32_t frequency, int duty) {
	uint32_t clock = 125000000;
	uint32_t divider16 = clock / frequency / 4096 + 
							(clock % (frequency * 4096) != 0);
	if (divider16 / 16 == 0)
	divider16 = 16;
	uint32_t wrap = clock * 16 / divider16 / frequency - 1;
	pwm_set_clkdiv_int_frac(slice, divider16/16,
										divider16 & 0xF);
	pwm_set_wrap(slice, wrap);
	pwm_set_chan_level(slice, channel, wrap * duty / 100);
	return wrap;
}


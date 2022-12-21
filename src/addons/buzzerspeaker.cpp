#include "addons/buzzerspeaker.h"
#include "songs.h"
#include "enums.h"
#include "helper.h"
#include "storagemanager.h"
#include "pico/stdlib.h"
#include "bitmaps.h"
#include "math.h"

bool BuzzerSpeakerAddon::available() {
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();
	return boardOptions.hasBuzzerSpeaker && 
		boardOptions.buzzerPin != (uint8_t)-1;
}

void BuzzerSpeakerAddon::setup() {
	BoardOptions boardOptions = Storage::getInstance().getBoardOptions();

	gpio_set_function(boardOptions.buzzerPin, GPIO_FUNC_PWM);
	buzzerPinSlice = pwm_gpio_to_slice_num (boardOptions.buzzerPin); 
	buzzerPinChannel = pwm_gpio_to_channel (boardOptions.buzzerPin);
	buzzerVolume = boardOptions.buzzerVolume;

	play(&intro);
}

void BuzzerSpeakerAddon::process() {
	processBuzzer();
}

void BuzzerSpeakerAddon::processBuzzer() {

	if (currentSong == NULL) {
		return;
	}

	uint32_t currentTimeSong = getMillis() - startedSongMils;
	uint32_t totalTimeSong = currentSong->song.size() * currentSong->toneDuration;
	uint16_t currentTonePosition = floor((currentTimeSong * currentSong->song.size()) / totalTimeSong);
	Tone currentTone = currentSong->song[currentTonePosition];

	if (currentTonePosition > currentSong->song.size()) {
		stop();
		return;
	}

	if (currentTone == PAUSE) {
		pwm_set_enabled (buzzerPinSlice, false);
		return;
	}

	pwmSetFreqDuty(buzzerPinSlice, buzzerPinChannel, currentTone, 0.03 * ((float) buzzerVolume));
	pwm_set_enabled (buzzerPinSlice, true);
}

void BuzzerSpeakerAddon::play(Song *song) {
	startedSongMils = getMillis();
	currentSong = song;
}

void BuzzerSpeakerAddon::stop() {
	pwm_set_enabled (buzzerPinSlice, false);
	currentSong = NULL;
}

uint32_t BuzzerSpeakerAddon::pwmSetFreqDuty(uint slice, uint channel, uint32_t frequency, float duty) {
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


#include "hardware/pwm.h"
#include "addons/buzzerspeaker.h"
#include "songs.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "usbdriver.h"
#include "math.h"
#include "helper.h"
#include "config.pb.h"

bool BuzzerSpeakerAddon::available() {
    const BuzzerOptions& options = Storage::getInstance().getAddonOptions().buzzerOptions;
	return options.enabled && isValidPin(options.pin);
}

void BuzzerSpeakerAddon::setup() {
	const BuzzerOptions& options = Storage::getInstance().getAddonOptions().buzzerOptions;
	buzzerPin = options.pin;
	gpio_set_function(buzzerPin, GPIO_FUNC_PWM);
	buzzerPinSlice = pwm_gpio_to_slice_num (buzzerPin);
	buzzerPinChannel = pwm_gpio_to_channel (buzzerPin);

    // enable pin is optional so not required to toggle addon
    if (isValidPin(options.pin)) {
        isSpeakerOn = true;
        buzzerEnablePin = options.enablePin;
        gpio_init(buzzerEnablePin);
        gpio_set_dir(buzzerEnablePin, GPIO_OUT);
        gpio_put(buzzerEnablePin, isSpeakerOn);
    }

	buzzerVolume = options.volume;
	introPlayed = false;
}

void BuzzerSpeakerAddon::process() {
	if (!introPlayed) {
		playIntro();
	}

	processBuzzer();
}

void BuzzerSpeakerAddon::playIntro() {
	if (getMillis() < 1000) {
		return;
	}

	bool isConfigMode = DriverManager::getInstance().isConfigMode();

	if (!get_usb_mounted() || isConfigMode) {
		play(&configModeSong);
	} else {
		play(&introSong);
	}
	introPlayed = true;
}

void BuzzerSpeakerAddon::processBuzzer() {
	if (currentSong == NULL) {
		return;
	}

	uint32_t currentTimeSong = getMillis() - startedSongMils;
	uint32_t totalTimeSong = currentSong->song.size() * currentSong->toneDuration;
	uint16_t currentTonePosition = floor((currentTimeSong * currentSong->song.size()) / totalTimeSong);
	Tone currentTone = currentSong->song[currentTonePosition];

	if (currentTonePosition >= currentSong->song.size()) {
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


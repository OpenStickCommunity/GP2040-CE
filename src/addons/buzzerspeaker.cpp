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
    uint32_t songSize = currentSong->song.size();
    uint32_t totalTimeSong = songSize * currentSong->toneDuration;

    if (currentTimeSong >= totalTimeSong) {
        stop();
        return;
    }

    uint16_t currentTonePosition = currentTimeSong / currentSong->toneDuration;
    Tone currentTone = currentSong->song[currentTonePosition];

    static Tone lastTone = PAUSE; // 前の音を記憶

    if (currentTone == PAUSE) {
        pwm_set_enabled(buzzerPinSlice, false);
    } else {
        // 同じ音が続く場合は再設定せず鳴らし続ける（ブツブツ音防止）
        if (currentTone != lastTone) {
            // 音量係数を 0.15 にアップ
            pwmSetFreqDuty(buzzerPinSlice, buzzerPinChannel, currentTone, 0.15 * ((float) buzzerVolume));
            pwm_set_enabled(buzzerPinSlice, true);
        }
    }
    lastTone = currentTone;
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


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

	pwm_set_freq_duty(buzzerPinSlice, buzzerPinChannel, 50, 75);
	pwm_set_enabled (buzzerPinSlice, true); 

	// test
	gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, 1);
}


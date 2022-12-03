/** 
 * Buzzer Speaker
 * References:
 * https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=1
 * https://www.tomshardware.com/how-to/buzzer-music-raspberry-pi-pico
 * 
 * **/

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

	int i = 1;

	while (i <= 2000) {
		pwmSetFreqDuty(buzzerPinSlice, buzzerPinChannel, i, 75);
		pwm_set_enabled (buzzerPinSlice, true); 
		sleep_ms(100);
		i++;
		if (i >= 2000) {
			i=1;
		}
    }

	
}

void BuzzerSpeakerAddon::process() {
	bool configMode = Storage::getInstance().GetConfigMode();

}

uint32_t BuzzerSpeakerAddon::pwmSetFreqDuty(uint slice_num, uint chan, uint32_t f, int d) {
	uint32_t clock = 125000000;
	uint32_t divider16 = clock / f / 4096 + 
							(clock % (f * 4096) != 0);
	if (divider16 / 16 == 0)
	divider16 = 16;
	uint32_t wrap = clock * 16 / divider16 / f - 1;
	pwm_set_clkdiv_int_frac(slice_num, divider16/16,
										divider16 & 0xF);
	pwm_set_wrap(slice_num, wrap);
	pwm_set_chan_level(slice_num, chan, wrap * d / 100);
	return wrap;
}


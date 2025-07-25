#include "addons/tg16_input.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h"

#if TG16_PAD_DEBUG==true
#include <iostream>
#endif

// Helper macros for pin access
#define SET_PIN(pin) gpio_put(pin, 1)
#define CLR_PIN(pin) gpio_put(pin, 0)
#define READ_PIN(pin) gpio_get(pin)

bool TG16padInput::available()
{
	const TG16Options &tg16Options = Storage::getInstance().getAddonOptions().tg16Options;
	if (tg16Options.oePin < 0 || tg16Options.selectPin < 0)
		return false;
	if (tg16Options.dataPin0 < 0 || tg16Options.dataPin1 < 0 || tg16Options.dataPin2 < 0 || tg16Options.dataPin3 < 0)
		return false;
	return true;
}

void TG16padInput::setup()
{
	const TG16Options &tg16Options = Storage::getInstance().getAddonOptions().tg16Options;
	nextTimer = getMillis();
	uIntervalMS = 0;

	// Set up OE and SELECT as outputs
	gpio_init(tg16Options.oePin);
	gpio_set_dir(tg16Options.oePin, GPIO_OUT);
	gpio_init(tg16Options.selectPin);
	gpio_set_dir(tg16Options.selectPin, GPIO_OUT);

	// Set up data pins as inputs with pull-ups
	int dataPins[4] = {tg16Options.dataPin0, tg16Options.dataPin1, tg16Options.dataPin2, tg16Options.dataPin3};
	for (int i = 0; i < 4; ++i)
	{
		gpio_init(dataPins[i]);
		gpio_set_dir(dataPins[i], GPIO_IN);
		gpio_pull_up(dataPins[i]);
	}
}

uint8_t TG16padInput::readController()
{
	const TG16Options &tg16Options = Storage::getInstance().getAddonOptions().tg16Options;
	int dataPins[4] = {tg16Options.dataPin0, tg16Options.dataPin1, tg16Options.dataPin2, tg16Options.dataPin3};
	uint8_t a = 0, b = 0;
	// Set OE active (active low)
	CLR_PIN(tg16Options.oePin);
	sleep_ms(1);

	// SELECT high, read first nibble
	SET_PIN(tg16Options.selectPin);
	sleep_ms(1);
	for (int i = 0; i < 4; ++i)
	{
		if (!READ_PIN(dataPins[i])) // active low
			a |= (1 << i);
	}

	// SELECT low, read second nibble
	CLR_PIN(tg16Options.selectPin);
	sleep_ms(1);
	for (int i = 0; i < 4; ++i)
	{
		if (!READ_PIN(dataPins[i])) // active low
			b |= (1 << i);
	}

	// Set OE inactive
	SET_PIN(tg16Options.oePin);

	return a | (b << 4);
}

void TG16padInput::updateButtons(uint8_t data)
{
	// Data bits: 7: I, 6: II, 5: Select, 4: Run, 3: Up, 2: Right, 1: Down, 0: Left
	buttonI = data & 0x80;
	buttonII = data & 0x40;
	buttonSelect = data & 0x20;
	buttonRun = data & 0x10;
	dpadUp = data & 0x08;
	dpadRight = data & 0x04;
	dpadDown = data & 0x02;
	dpadLeft = data & 0x01;

	// Map dpad to analog stick
	leftX = dpadLeft ? GAMEPAD_JOYSTICK_MIN : (dpadRight ? GAMEPAD_JOYSTICK_MAX : GAMEPAD_JOYSTICK_MID);
	leftY = dpadUp ? GAMEPAD_JOYSTICK_MIN : (dpadDown ? GAMEPAD_JOYSTICK_MAX : GAMEPAD_JOYSTICK_MID);
	rightX = GAMEPAD_JOYSTICK_MID;
	rightY = GAMEPAD_JOYSTICK_MID;
}

void TG16padInput::process()
{
	if (nextTimer < getMillis())
	{
		uint8_t data = readController();
		// Invert because buttons are active low
		data ^= 0xFF;
		updateButtons(data);
		nextTimer = getMillis() + uIntervalMS;
	}

#if TG16_PAD_DEBUG==true
    stdio_init_all();
	printf("TG16padInput::process()\n");
	printf("data: %d\n", data);
	printf("buttonI: %d\n", buttonI);
	printf("buttonII: %d\n", buttonII);
	printf("buttonSelect: %d\n", buttonSelect);
	printf("buttonRun: %d\n", buttonRun);
	printf("dpadUp: %d\n", dpadUp);
	printf("dpadRight: %d\n", dpadRight);
	printf("dpadDown: %d\n", dpadDown);
#endif

	Gamepad *gamepad = Storage::getInstance().GetGamepad();

	gamepad->state.lx = leftX;
	gamepad->state.ly = leftY;
	gamepad->state.rx = rightX;
	gamepad->state.ry = rightY;

	gamepad->hasAnalogTriggers = false;
	gamepad->hasLeftAnalogStick = true;

	if (buttonI)
		gamepad->state.buttons |= GAMEPAD_MASK_B1;
	if (buttonII)
		gamepad->state.buttons |= GAMEPAD_MASK_B2;
	if (buttonSelect)
		gamepad->state.buttons |= GAMEPAD_MASK_S1;
	if (buttonRun)
		gamepad->state.buttons |= GAMEPAD_MASK_S2;
	if (dpadUp)
		gamepad->state.dpad |= GAMEPAD_MASK_UP;
	if (dpadDown)
		gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
	if (dpadLeft)
		gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
	if (dpadRight)
		gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
}

uint16_t TG16padInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

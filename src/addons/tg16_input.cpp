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

// 6-button support state
static bool sixButtonMode = false;
static uint32_t sixButtonTimeout = 0;
#define SIX_BUTTON_TIMEOUT_MS 150
static int sixButtonSequenceStep = 0;

// Extra buttons for 6-button mode
static bool buttonIII = false;
static bool buttonIV = false;
static bool buttonV = false;
static bool buttonVI = false;

// Helper to detect 6-button sequence (3 rapid reads with SELECT high/low)
static void detectSixButtonSequence(bool selectState) {
    // Sequence: High, Low, High (SEL)
    static int lastSelect = -1;
    if (lastSelect == -1) lastSelect = selectState;
    if (selectState != lastSelect) {
        sixButtonSequenceStep++;
        lastSelect = selectState;
        if (sixButtonSequenceStep >= 3) {
            sixButtonMode = true;
            sixButtonTimeout = getMillis() + SIX_BUTTON_TIMEOUT_MS;
            sixButtonSequenceStep = 0;
        }
    }
}

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
    int dataPins[4] = {tg16Options.dataPin3, tg16Options.dataPin2, tg16Options.dataPin1, tg16Options.dataPin0};
    uint8_t a = 0, b = 0, c = 0;
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
    // Detect 6-button sequence (SEL high)
    detectSixButtonSequence(1);

    // SELECT low, read second nibble
    CLR_PIN(tg16Options.selectPin);
    sleep_ms(1);
    for (int i = 0; i < 4; ++i)
    {
        if (!READ_PIN(dataPins[i])) // active low
            b |= (1 << i);
    }
    // Detect 6-button sequence (SEL low)
    detectSixButtonSequence(0);

    // If in 6-button mode, read third nibble (extra buttons)
    if (sixButtonMode) {
        // SELECT high again
        SET_PIN(tg16Options.selectPin);
        sleep_ms(1);
        for (int i = 0; i < 4; ++i)
        {
            if (!READ_PIN(dataPins[i])) // active low
                c |= (1 << i);
        }
        // After reading, revert SELECT to low
        CLR_PIN(tg16Options.selectPin);
    }

    // Set OE inactive
    SET_PIN(tg16Options.oePin);

    // Pack nibbles: a = 1st (I,II,Select,Run), b = 2nd (Up,Right,Down,Left), c = 3rd (III,IV,V,VI)
    if (sixButtonMode) {
        return (a & 0x0F) | ((b & 0x0F) << 4) | ((c & 0x0F) << 8); // 12 bits
    } else {
        return (a & 0x0F) | ((b & 0x0F) << 4); // 8 bits
    }
}

void TG16padInput::updateButtons(uint16_t data)
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
    // 6-button extra: bits 8-11 (III, IV, V, VI)
    buttonIII = data & 0x100;
    buttonIV  = data & 0x200;
    buttonV   = data & 0x400;
    buttonVI  = data & 0x800;
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
        uint16_t data = readController();
        updateButtons(data);
        nextTimer = getMillis() + uIntervalMS;
    }
    // Timeout for 6-button mode
    if (sixButtonMode && getMillis() > sixButtonTimeout) {
        sixButtonMode = false;
    }
#if TG16_PAD_DEBUG==true
    stdio_init_all();
    const TG16Options &tg16Options = Storage::getInstance().getAddonOptions().tg16Options;
    int oeState = gpio_get(tg16Options.oePin);
    int selectState = gpio_get(tg16Options.selectPin);
    printf(
        "OE: %d SELECT: %d | I=%1d II=%1d III=%1d IV=%1d V=%1d VI=%1d Select=%1d Run=%1d Up=%1d Down=%1d Left=%1d Right=%1d | 6btn: %d\n",
        oeState, selectState,
        buttonI, buttonII, buttonIII, buttonIV, buttonV, buttonVI, buttonSelect, buttonRun, dpadUp, dpadDown, dpadLeft, dpadRight, sixButtonMode
    );
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
    if (buttonIII)
        gamepad->state.buttons |= GAMEPAD_MASK_B3;
    if (buttonIV)
        gamepad->state.buttons |= GAMEPAD_MASK_B4;
    if (buttonV)
        gamepad->state.buttons |= GAMEPAD_MASK_L1;
    if (buttonVI)
        gamepad->state.buttons |= GAMEPAD_MASK_R1;
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

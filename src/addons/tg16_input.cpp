#include "addons/tg16_input.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h"

// Helper macros for pin access
#define SET_PIN(pin) gpio_put(pin, 1)
#define CLR_PIN(pin) gpio_put(pin, 0)
#define READ_PIN(pin) gpio_get(pin)

// Data pins are expected to be an array of 4 GPIOs
static int tg16_data_pins[4] = TG16_PAD_DATA_PINS;
static int tg16_oe_pin = TG16_PAD_OE_PIN;
static int tg16_select_pin = TG16_PAD_SELECT_PIN;

bool TG16padInput::available() {
    // Check if all pins are valid
    if (tg16_oe_pin < 0 || tg16_select_pin < 0)
        return false;
    for (int i = 0; i < 4; ++i) {
        if (tg16_data_pins[i] < 0)
            return false;
    }
    return true;
}

void TG16padInput::setup() {
    nextTimer = getMillis();
    uIntervalMS = 0;

    // Set up OE and SELECT as outputs
    gpio_init(tg16_oe_pin);
    gpio_set_dir(tg16_oe_pin, GPIO_OUT);
    gpio_init(tg16_select_pin);
    gpio_set_dir(tg16_select_pin, GPIO_OUT);

    // Set up data pins as inputs with pull-ups
    for (int i = 0; i < 4; ++i) {
        gpio_init(tg16_data_pins[i]);
        gpio_set_dir(tg16_data_pins[i], GPIO_IN);
        gpio_pull_up(tg16_data_pins[i]);
    }
}

uint8_t TG16padInput::readController() {
    uint8_t a = 0, b = 0;
    // Set OE active (active low)
    CLR_PIN(tg16_oe_pin);
    sleep_ms(1);

    // SELECT high, read first nibble
    SET_PIN(tg16_select_pin);
    sleep_ms(1);
    for (int i = 0; i < 4; ++i) {
        if (!READ_PIN(tg16_data_pins[i])) // active low
            a |= (1 << i);
    }

    // SELECT low, read second nibble
    CLR_PIN(tg16_select_pin);
    sleep_ms(1);
    for (int i = 0; i < 4; ++i) {
        if (!READ_PIN(tg16_data_pins[i])) // active low
            b |= (1 << i);
    }

    // Set OE inactive
    SET_PIN(tg16_oe_pin);

    return a | (b << 4);
}

void TG16padInput::updateButtons(uint8_t data) {
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

void TG16padInput::process() {
    if (nextTimer < getMillis()) {
        uint8_t data = readController();
        // Invert because buttons are active low
        data ^= 0xFF;
        updateButtons(data);
        nextTimer = getMillis() + uIntervalMS;
    }

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    gamepad->state.lx = leftX;
    gamepad->state.ly = leftY;
    gamepad->state.rx = rightX;
    gamepad->state.ry = rightY;

    gamepad->hasAnalogTriggers = false;
    gamepad->hasLeftAnalogStick = true;

    if (buttonI) gamepad->state.buttons |= GAMEPAD_MASK_B1;
    if (buttonII) gamepad->state.buttons |= GAMEPAD_MASK_B2;
    if (buttonSelect) gamepad->state.buttons |= GAMEPAD_MASK_S1;
    if (buttonRun) gamepad->state.buttons |= GAMEPAD_MASK_S2;
    if (dpadUp) gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (dpadDown) gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (dpadLeft) gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (dpadRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
}

uint16_t TG16padInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} 
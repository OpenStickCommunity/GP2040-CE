#include "addons/psx_input.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h"

// Should the Addon be setup and called ?
bool PsxAddon::available() {
    const PsxOptions& psxOptions = Storage::getInstance().getAddonOptions().psxOptions;
    return 
        psxOptions.enabled
        && isValidPin(psxOptions.attentionPin)
        && isValidPin(psxOptions.clockPin)
        && isValidPin(psxOptions.commandPin)
        && isValidPin(psxOptions.dataPin)
    ;
}

// Called once at startup, when GP2040 or GP2040aux registers the addon
void PsxAddon::setup() {
    const PsxOptions& psxOptions = Storage::getInstance().getAddonOptions().psxOptions;

    attentionPin = psxOptions.attentionPin;
    clockPin = psxOptions.clockPin;
    commandPin = psxOptions.commandPin;
    dataPin = psxOptions.dataPin;

    // Data (IN, 1, brown)
    gpio_init(dataPin);
    gpio_disable_pulls(dataPin);
    gpio_set_dir(dataPin, GPIO_IN);

    // Command (OUT, 2, orange)
    gpio_init(commandPin);
    gpio_put(commandPin, 0);
    gpio_set_dir(commandPin, GPIO_OUT);

    // Motor (POWER, 3, grey)

    // Ground (POWER, 4, black)

    // 3V3 (POWER, 5, red)

    // Attention (OUT, 6, yellow)
    gpio_init(attentionPin);
    gpio_put(attentionPin, 1);
    gpio_set_dir(attentionPin, GPIO_OUT);

    // Clock (OUT, 7, blue)
    gpio_init(clockPin);
    gpio_put(clockPin, 1);
    gpio_set_dir(clockPin, GPIO_OUT);

    // Unknown (??, 8, white)

    // Acknowledge (IN, 9, green)

};

// Called after the gamepad is read, but before the read values are processed
void PsxAddon::preprocess() {
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    // Set attention signal low
    gpio_put(attentionPin, 0);

    uint8_t shouldBe0Xff = readwrite(0x01);
    uint8_t shouldBe0X41 = readwrite(0x42);
    uint8_t shouldBe0X5a = readwrite(0x00);
    uint8_t answer1      = readwrite(0x00);     // MSB [LEFT] [DOWN] [RGHT] [ UP ] [STRT] [    ] [    ] [SLCT] LSB
    uint8_t answer2      = readwrite(0x00);     // MSB [ [] ] [ X  ] [ O  ] [ /\ ] [ R1 ] [ L1 ] [ R2 ] [ L2 ] LSB (bits are sent LSB first on protocol but read MSB first by the function)

    gamepad->state.buttons =
        0
        | ((answer2 & 0b01000000) ? 0 : GAMEPAD_MASK_B1)
        | ((answer2 & 0b00100000) ? 0 : GAMEPAD_MASK_B2)
        | ((answer2 & 0b10000000) ? 0 : GAMEPAD_MASK_B3)
        | ((answer2 & 0b00010000) ? 0 : GAMEPAD_MASK_B4)
        | ((answer2 & 0b00000100) ? 0 : GAMEPAD_MASK_L1)
        | ((answer2 & 0b00001000) ? 0 : GAMEPAD_MASK_R1)
        | ((answer2 & 0b00000001) ? 0 : GAMEPAD_MASK_L2)
        | ((answer2 & 0b00000010) ? 0 : GAMEPAD_MASK_R2)
        | ((answer1 & 0b00000001) ? 0 : GAMEPAD_MASK_S1)
        | ((answer1 & 0b00001000) ? 0 : GAMEPAD_MASK_S2)
    ;

    gamepad->state.dpad =
        0
        | ((answer1 & 0b00010000) ? 0 : GAMEPAD_MASK_UP)
        | ((answer1 & 0b00100000) ? 0 : GAMEPAD_MASK_RIGHT)
        | ((answer1 & 0b01000000) ? 0 : GAMEPAD_MASK_DOWN)
        | ((answer1 & 0b10000000) ? 0 : GAMEPAD_MASK_LEFT)
    ;

    // Set attention signal high
    gpio_put(attentionPin, 1);
}

// Called after the gamepad values avec been processed
void PsxAddon::process() {
};

/**
 * Pins must be set up
 * Attention must be taken
 */
uint8_t PsxAddon::readwrite(uint8_t sent) {
    uint8_t received = 0;

    sleep_us(BYTE_DELAY_US);

    for(int bit = 0; bit <= 7; bit++) {
        gpio_put(commandPin, sent & 0b00000001);
        sent = sent >> 1;

        gpio_put(clockPin, 0);
        sleep_us(2);
        gpio_put(clockPin, 1);

        sleep_us(1);
        received = received >> 1;
        received |= gpio_get(dataPin) ? 0b10000000 : 0;
        sleep_us(1);

    }

    return received;
}
#include "addons/psx.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "hardware/gpio.h"
#include "helper.h"

// Should the Addon be setup and called ?
bool PsxAddon::available() {
    const PsxOptions& psxOptions = Storage::getInstance().getAddonOptions().psxOptions;
    return 
        psxOptions.enabled
        && isValidPin(psxOptions.acknowledgePin)
        && isValidPin(psxOptions.attentionPin)
        && isValidPin(psxOptions.clockPin)
        && isValidPin(psxOptions.commandPin)
        && isValidPin(psxOptions.dataPin)
    ;
}

// Called once at startup, when GP2040 or GP2040aux registers the addon
void PsxAddon::setup() {
    const PsxOptions& psxOptions = Storage::getInstance().getAddonOptions().psxOptions;

    acknowledgePin = psxOptions.acknowledgePin;
    attentionPin = psxOptions.attentionPin;
    clockPin = psxOptions.clockPin;
    commandPin = psxOptions.commandPin;
    dataPin = psxOptions.dataPin;

    gpio_init(acknowledgePin);
    gpio_init(attentionPin);
    gpio_init(clockPin);
    gpio_init(commandPin);
    gpio_init(dataPin);

    gpio_put(attentionPin, 1);
    gpio_put(clockPin, 1);
    gpio_put(commandPin, 0);

    gpio_set_dir(acknowledgePin, GPIO_IN);
    gpio_set_dir(attentionPin, GPIO_OUT);
    gpio_set_dir(clockPin, GPIO_OUT);
    gpio_set_dir(commandPin, GPIO_OUT);
    gpio_set_dir(dataPin, GPIO_IN);

    //todo : fill the gamePad so input can be used at boot to change mode and  maybe reboot
};

// Called after the gamepad is read, but before the read values are processed
void preprocess() {}

// Called after the gamepad values avec been processed
void PsxAddon::process() {
    Gamepad *gamepad = Storage::getInstance().GetGamepad();

    takeAttention();

    uint8_t shouldBe0Xff = readwrite(0x01);
    uint8_t shouldBe0X41 = readwrite(0x42);
    uint8_t shouldBe0X5a = readwrite(0x00);
    uint8_t answer1      = readwrite(0x00);     // [SLCT] [    ] [    ] [STRT] [ UP ] [RGHT] [DOWN] [LEFT]
    uint8_t answer2      = readwrite(0x00);     // [ L2 ] [ R2 ] [ L1 ] [ R1 ] [ /\ ] [ O  ] [ X  ] [ [] ]

    gamepad->state.buttons =
        AUX_MASK_FUNCTION
        || (answer2 && 0b00000100) ? GAMEPAD_MASK_B1 : 0
        || (answer2 && 0b00000010) ? GAMEPAD_MASK_B2 : 0
        || (answer2 && 0b00001000) ? GAMEPAD_MASK_B3 : 0
        || (answer2 && 0b00000001) ? GAMEPAD_MASK_B4 : 0
        || (answer2 && 0b00100000) ? GAMEPAD_MASK_L1 : 0
        || (answer2 && 0b00010000) ? GAMEPAD_MASK_R1 : 0
        || (answer2 && 0b10000000) ? GAMEPAD_MASK_L2 : 0
        || (answer2 && 0b01000000) ? GAMEPAD_MASK_R2 : 0
        || (answer1 && 0b10000000) ? GAMEPAD_MASK_S1 : 0
        || (answer1 && 0b00010000) ? GAMEPAD_MASK_S2 : 0
    ;

    gamepad->state.dpad =
        0
        || (answer1 && 0b00001000) ? GAMEPAD_MASK_UP : 0
        || (answer1 && 0b00000100) ? GAMEPAD_MASK_RIGHT : 0
        || (answer1 && 0b00000010) ? GAMEPAD_MASK_DOWN : 0
        || (answer1 && 0b00000001) ? GAMEPAD_MASK_LEFT : 0
    ;

    releaseAttention();
};

void PsxAddon::takeAttention() {
    gpio_put(attentionPin, 0);
}

void PsxAddon::releaseAttention() {
    gpio_put(attentionPin, 1);
}

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
        busy_wait_us(CLOCK_DELAY_US);
        gpio_put(clockPin, 1);

        received = received >> 1;
        if(gpio_get(dataPin)) {
            received |= 0b10000000;
        };

        busy_wait_us(CLOCK_DELAY_US);
    }

    return received;
}


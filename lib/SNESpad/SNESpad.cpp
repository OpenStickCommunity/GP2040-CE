#include "SNESpad.h"

#include <cstring>
#include <cstdio>

SNESpad::SNESpad(int clock, int latch, int data) {
    latchPin = latch;
    clockPin = clock;
    dataPin = data;
}

void SNESpad::begin() {
    init();
#if SNES_PAD_DEBUG==true
    printf("SNESpad::begin\n");
#endif
}

void SNESpad::start() {
    uint32_t packet;

#if SNES_PAD_DEBUG==true
    printf("SNESpad::start\n");
#endif

    type = SNES_PAD_NONE;

    packet = read();

#if SNES_PAD_DEBUG==true
    // printf("Data Packet: %02x\n",packet);
#endif

    if (type != SNES_PAD_NONE) {
#if SNES_PAD_DEBUG==true
        printf("Device Type: %d\n", type);
#endif
        mouseX          = 0;
        mouseY          = 0;

        buttonA         = 0;
        buttonB         = 0;
        buttonX         = 0;
        buttonY         = 0;
        buttonStart     = 0;
        buttonSelect    = 0;
        buttonL         = 0;
        buttonR         = 0;

        directionUp     = 0;
        directionDown   = 0;
        directionLeft   = 0;
        directionRight  = 0;

    } else {
#if SNES_PAD_DEBUG==true
        printf("Unknown Device: %02x\n", packet);
#endif
    }
}

void SNESpad::poll() {
    int32_t state = 0;

#if SNES_PAD_DEBUG==true
    //printf("SNESpad::poll\n");
#endif

    if (type != SNES_PAD_NONE) {
        state = read(); // polls current controller state

        if (state) {
            switch (type) {
                case SNES_PAD_BASIC:
                    directionLeft =  (state & SNES_LEFT);
                    directionUp =    (state & SNES_UP);
                    directionRight = (state & SNES_RIGHT);
                    directionDown =  (state & SNES_DOWN);

                    buttonSelect =   (state & SNES_SELECT);
                    buttonStart =    (state & SNES_START);
                    buttonB =        (state & SNES_B);
                    buttonY =        (state & SNES_Y);
                    buttonA =        (state & SNES_A);
                    buttonX =        (state & SNES_X);
                    buttonL =        (state & SNES_L);
                    buttonR =        (state & SNES_R);

                    break;
                case SNES_PAD_NES:
                    directionLeft =  (state & SNES_LEFT);
                    directionUp =    (state & SNES_UP);
                    directionRight = (state & SNES_RIGHT);
                    directionDown =  (state & SNES_DOWN);

                    buttonSelect =   (state & SNES_SELECT);
                    buttonStart =    (state & SNES_START);
                    buttonB =        (state & SNES_B);
                    buttonA =        (state & SNES_A);

                    break;
                case SNES_PAD_MOUSE:
                    int x = 127;  //set center position [0-255]
                    int y = 127;

                    // Mouse X axis
                    x = (state & SNES_MOUSE_X) >> 25;
                    x = reverse(x) * SNES_MOUSE_PRECISION;
                    if (state & SNES_MOUSE_X_SIGN) x = 127 + x;
                    else x = 127 - x;

                    // Mouse Y axis
                    y = (state & SNES_MOUSE_Y) >> 17;
                    y = reverse(y) * SNES_MOUSE_PRECISION;
                    if (state & SNES_MOUSE_Y_SIGN) y = 127 + y;
                    else y = 127 - y;

                    mouseX  = x;
                    mouseY  = y;
                    buttonB = (state & SNES_X);
                    buttonA = (state & SNES_A);

                    break;
            }

#if SNES_PAD_DEBUG==true
            if (_lastRead != state) {
                printf(
                    "A=%1d B=%1d X=%1d Y=%1d L=%1d R=%1d Select=%1d Start=%1d Mouse X=%4d Y=%4d\n",
                    buttonA, buttonB, buttonX, buttonY, buttonL, buttonR, buttonSelect, buttonStart, mouseX, mouseY
                );
            }
            _lastRead = state;
#endif
        } else {
            // device disconnected or invalid read
            type = SNES_PAD_NONE;
            start();
        }
    } else {
        start();
    }
}

// init gpio pins
void SNESpad::init() {
    gpio_init(clockPin);
    gpio_init(latchPin);
    gpio_init(dataPin);

    gpio_set_dir(clockPin, GPIO_OUT);
    gpio_set_dir(latchPin, GPIO_OUT);
    gpio_set_dir(dataPin, GPIO_IN);
    gpio_pull_up(dataPin);

    return;
}

// signal mouse to go to next speed if not at desired speed
void SNESpad::speed()
{
    // default mouse to fastest speed
    if (type == SNES_PAD_MOUSE
        && mouseSpeed != SNES_MOUSE_FAST
        && mouseSpeedFails < SNES_MOUSE_THRESHOLD
    ) {
        gpio_put(clockPin,0);
        busy_wait_us(6);

        gpio_put(clockPin,1);
        busy_wait_us(12);
    }
}

// clock in a data bit
uint32_t SNESpad::clock()
{
    gpio_put(clockPin,0);
    busy_wait_us(6);

    uint32_t ret = gpio_get(dataPin);
    gpio_put(clockPin,1);

    busy_wait_us(6);
    return ret;
}

// latch to start read
void SNESpad::latch()
{
    gpio_put(latchPin,1);
    busy_wait_us(12);

    speed(); // check/set mouse speed

    gpio_put(latchPin,0);
    busy_wait_us(6);
}

uint32_t SNESpad::read()
{
    uint32_t ret = 0;
    uint8_t i;

    latch();
    for (i = 0; i < 32; i++) {
        uint32_t bit = clock(); // clock shift bit in 
        ret |= bit << i;

        if (i == 15) {
            bool read_extra = !bit; // check if mouse
            if (!read_extra) break; // skip extra bytes if not
        }
    }

    ret = ~ret; // buttons are active low, so invert bits

    // verify controller or mouse is connected
    if (!ret) {
        type = SNES_PAD_NONE;
        mouseSpeedFails = 0;
        mouseSpeed = 0;
        return 0;
    }

    // check device type id
    bool isSNES = ((ret & SNES_DEVICE_ID) >> 12) == SNES_PAD_ID;
    bool isMouse = ((ret & SNES_DEVICE_ID) >> 12) == SNES_MOUSE_ID;
    bool isNES = ((ret >> 8) & 0b11111111) == 0b11111111;

    // verify mouse speed
    if (isSNES) {
        type = SNES_PAD_BASIC;
    } else if (isNES) {
        type = SNES_PAD_NES;
    } else if (isMouse) {
        uint8_t lastMouseSpeed = mouseSpeed;

        // parse mouse speed bits
        mouseSpeed = ((ret & SNES_MOUSE_SPEED) >> 10);
        if (mouseSpeed > 2) mouseSpeed = 0;

        // detect hyperkin mouse failure to change speed to halt further attempts
        if (
            mouseSpeed != SNES_MOUSE_FAST
            && lastMouseSpeed == mouseSpeed
            && mouseSpeedFails < SNES_MOUSE_THRESHOLD
        ) {
            mouseSpeedFails++;
        }

        type = SNES_PAD_MOUSE;
    } else {
        type = SNES_PAD_NONE; //UNKNOWN device id
    }

    return ret;
}

// reverse bits within a byte (ex: 0b1000 -> 0b0001)
uint8_t SNESpad::reverse(uint8_t c) {
    char r = 0;
    for(int i = 0; i < 8; i++) {
        r <<= 1;
        r |= c & 1;
        c >>= 1;
    }
    return r;
}

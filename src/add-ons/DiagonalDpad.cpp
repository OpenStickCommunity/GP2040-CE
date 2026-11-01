#include "addon.h"
#include "pico/stdlib.h"
#include "gamepad/GamepadState.h"
#include "storage/Storage.h"
#include "enums.pb.h"

// 对应接线的GPIO编号，接错了改这里就行
#define UL_PIN 0
#define DL_PIN 1
#define UR_PIN 2
#define DR_PIN 3

static Addon* DiagonalDpadCreate() {
    Addon* addon = new Addon();
    addon->SetName("DiagonalDpad");
    return addon;
}

void DiagonalDpadInit(Addon* addon) {
    // 初始化GPIO为输入，开启内部上拉
    gpio_init(UL_PIN); gpio_set_dir(UL_PIN, GPIO_IN); gpio_pull_up(UL_PIN);
    gpio_init(DL_PIN); gpio_set_dir(DL_PIN, GPIO_IN); gpio_pull_up(DL_PIN);
    gpio_init(UR_PIN); gpio_set_dir(UR_PIN, GPIO_IN); gpio_pull_up(UR_PIN);
    gpio_init(DR_PIN); gpio_set_dir(DR_PIN, GPIO_IN); gpio_pull_up(DR_PIN);
}

void DiagonalDpadProcess(Addon* addon) {
    // 低电平=按下
    bool ul = !gpio_get(UL_PIN);
    bool dl = !gpio_get(DL_PIN);
    bool ur = !gpio_get(UR_PIN);
    bool dr = !gpio_get(DR_PIN);
    
    uint8_t dpad = 0;
    // 优先判断两键组合的正方向
    bool isLeft = ul && dl;
    bool isRight = ur && dr;
    bool isUp = ul && ur;
    bool isDown = dl && dr;
    
    if (isLeft) dpad |= GAMEPAD_MASK_LEFT;
    if (isRight) dpad |= GAMEPAD_MASK_RIGHT;
    if (isUp) dpad |= GAMEPAD_MASK_UP;
    if (isDown) dpad |= GAMEPAD_MASK_DOWN;
    
    // 无组合时输出单键斜向
    if (ul && !isLeft && !isUp) dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT;
    if (dl && !isLeft && !isDown) dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT;
    if (ur && !isRight && !isUp) dpad |= GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT;
    if (dr && !isRight && !isDown) dpad |= GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT;
    
    // SOCD清理后输出
    Gamepad *gamepad = Storage::getInstance().GetGamepad();
    gamepad->state.dpad = runSOCDCleaner(Storage::getInstance().GetConfig()->socdMode, dpad);
}

REGISTER_ADDON(DiagonalDpadCreate, DiagonalDpadInit, nullptr, DiagonalDpadProcess, nullptr);

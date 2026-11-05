#include "DisplaySaverScreen.h"
#include "BitmapScreens.h"

#include "drivermanager.h"
#include "pico/stdlib.h"
#include "version.h"
#include <algorithm>
#include <cstring>

void DisplaySaverScreen::init() {
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    displaySaverMode = options.displaySaverMode;

    getRenderer()->clearScreen();

    switch (displaySaverMode) {
        case DisplaySaverMode::DISPLAY_SAVER_SNOW:
            initSnowScene();
            break;
        case DisplaySaverMode::DISPLAY_SAVER_BOUNCE:
            break;
        case DisplaySaverMode::DISPLAY_SAVER_PIPES:
            initPipeScene();
            break;
        case DisplaySaverMode::DISPLAY_SAVER_TOAST:
            initToasters();
            break;
        default:
            break;
    }
}

void DisplaySaverScreen::shutdown() {
    clearElements();
}

void DisplaySaverScreen::drawScreen() {
    switch (displaySaverMode) {
        case DisplaySaverMode::DISPLAY_SAVER_SNOW:
            drawSnowScene();
            break;
        case DisplaySaverMode::DISPLAY_SAVER_BOUNCE:
            drawBounceScene();
            break;
        case DisplaySaverMode::DISPLAY_SAVER_PIPES:
            drawPipeScene();
            break;
        case DisplaySaverMode::DISPLAY_SAVER_TOAST:
            drawToasterScene();
            break;
        default:
            break;
    }
}

int8_t DisplaySaverScreen::update() {
    if (!DriverManager::getInstance().isConfigMode()) {
        Gamepad * gamepad = Storage::getInstance().GetGamepad();
        
        if (gamepad->state.buttons || gamepad->state.dpad) {
            return DisplayMode::BUTTONS;
        }
    }

    return -1; // -1 means no change in screen state
}

void DisplaySaverScreen::initSnowScene() {
    for (uint8_t x = 0; x < SCREEN_WIDTH; ++x) {
        for (uint8_t y = 0; y < SCREEN_HEIGHT; ++y) {
            snowflakeSpeeds[x][y] = 0;
            snowflakeDrift[x][y] = 0;
            getRenderer()->drawPixel(x, y, 0);
        }
    }
}

void DisplaySaverScreen::drawSnowScene() {
    for (int8_t y = SCREEN_HEIGHT - 1; y >= 0; --y) {
        for (uint8_t x = 0; x < SCREEN_WIDTH; ++x) {
            if (snowflakeSpeeds[x][y] > 0) {
                uint8_t speed = snowflakeSpeeds[x][y];
                uint8_t newY = y + speed;
                int8_t drift = snowflakeDrift[x][y];
                int8_t newX = std::clamp((x + drift), 0, (SCREEN_WIDTH - 1));

                if (newY >= SCREEN_HEIGHT) {
                    getRenderer()->drawPixel(x, y, 0);
                    snowflakeSpeeds[x][y] = 0;
                    snowflakeDrift[x][y] = 0;
                } else {
                    getRenderer()->drawPixel(x, y, 0);
                    getRenderer()->drawPixel(newX, newY, 1);
                    snowflakeSpeeds[newX][newY] = speed;
                    snowflakeDrift[newX][newY] = drift;
                    snowflakeSpeeds[x][y] = 0;
                    snowflakeDrift[x][y] = 0;
                }
            }
        }
    }

    for (uint8_t x = 0; x < SCREEN_WIDTH; ++x) {
        if (rand() % 10 == 0) {
            getRenderer()->drawPixel(x, 0, 1);
            snowflakeSpeeds[x][0] = (rand() % 3) + 1;
            snowflakeDrift[x][0] = (rand() % 3) - 1;
        }
    }
}

void DisplaySaverScreen::drawBounceScene() {
    uint16_t scaledWidth = static_cast<uint16_t>(bounceSpriteWidth * bounceScale);
    uint16_t scaledHeight = static_cast<uint16_t>(bounceSpriteHeight * bounceScale);

    bounceSpriteX += bounceSpriteVelocityX;
    bounceSpriteY += bounceSpriteVelocityY;

    if (bounceSpriteX <= 0 || bounceSpriteX + scaledWidth >= SCREEN_WIDTH) bounceSpriteVelocityX = -bounceSpriteVelocityX;

    if (bounceSpriteY <= 0 || bounceSpriteY + scaledHeight >= SCREEN_HEIGHT) bounceSpriteVelocityY = -bounceSpriteVelocityY;

    getRenderer()->drawSprite((uint8_t *)bitmapGP2040Logo, bounceSpriteWidth, bounceSpriteHeight, 0, bounceSpriteX, bounceSpriteY, 0, bounceScale);
}

enum : uint8_t {
    PIPE_NORTH = 1 << 0,
    PIPE_EAST  = 1 << 1,
    PIPE_SOUTH = 1 << 2,
    PIPE_WEST  = 1 << 3,
    PIPE_JOINT = 1 << 4,
};

static const int8_t pipeDirX[4] = { 0, 1, 0, -1 };
static const int8_t pipeDirY[4] = { -1, 0, 1, 0 };

void DisplaySaverScreen::initPipeScene() {
    srand(time_us_32());
    memset(pipeCells, 0, sizeof(pipeCells));
    for (uint8_t i = 0; i < PIPE_HEADS; ++i) {
        pipeHeads[i].alive = false;
    }
    pipeFilled = 0;
    pipeFullSince = 0;
    pipeLastStep = to_ms_since_boot(get_absolute_time());
}

bool DisplaySaverScreen::isPipeCellFree(int16_t x, int16_t y) {
    return x >= 0 && x < PIPE_COLS && y >= 0 && y < PIPE_ROWS && pipeCells[y][x] == 0;
}

bool DisplaySaverScreen::spawnPipe(PipeHead& head) {
    uint16_t freeCells = PIPE_COLS * PIPE_ROWS - pipeFilled;
    if (freeCells == 0) return false;

    uint16_t pick = rand() % freeCells;
    for (uint8_t y = 0; y < PIPE_ROWS; ++y) {
        for (uint8_t x = 0; x < PIPE_COLS; ++x) {
            if (pipeCells[y][x] == 0 && pick-- == 0) {
                head = { x, y, static_cast<uint8_t>(rand() % 4), 0, true };
                pipeCells[y][x] = PIPE_JOINT;
                ++pipeFilled;
                return true;
            }
        }
    }
    return false;
}

void DisplaySaverScreen::stepPipe(PipeHead& head) {
    const uint8_t PIPE_TURN_CHANCE = 5;
    const uint8_t PIPE_MAX_LENGTH = 30;

    uint8_t left = (head.dir + 3) % 4;
    uint8_t right = (head.dir + 1) % 4;
    if (rand() % 2) std::swap(left, right);
    uint8_t choices[3] = { head.dir, left, right };
    if (rand() % PIPE_TURN_CHANCE == 0) {
        choices[0] = left;
        choices[1] = right;
        choices[2] = head.dir;
    }

    int8_t newDir = -1;
    for (uint8_t i = 0; i < 3; ++i) {
        if (isPipeCellFree(head.x + pipeDirX[choices[i]], head.y + pipeDirY[choices[i]])) {
            newDir = choices[i];
            break;
        }
    }

    if (newDir < 0 || head.length >= PIPE_MAX_LENGTH) {
        pipeCells[head.y][head.x] |= PIPE_JOINT;
        head.alive = false;
        return;
    }

    if (newDir != head.dir) {
        pipeCells[head.y][head.x] |= PIPE_JOINT;
    }
    pipeCells[head.y][head.x] |= (1 << newDir);
    head.x += pipeDirX[newDir];
    head.y += pipeDirY[newDir];
    head.dir = newDir;
    ++head.length;
    pipeCells[head.y][head.x] = (1 << ((newDir + 2) % 4));
    ++pipeFilled;
}

void DisplaySaverScreen::fillPipeRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    for (uint16_t py = y; py < y + h; ++py) {
        for (uint16_t px = x; px < x + w; ++px) {
            getRenderer()->drawPixel(px, py, 1);
        }
    }
}

void DisplaySaverScreen::drawPipeScene() {
    const uint32_t PIPE_STEP_MS = 40;
    const uint8_t PIPE_MAX_STEPS = 4;
    const uint32_t PIPE_FULL_HOLD_MS = 1000;
    const uint8_t offsetX = (SCREEN_WIDTH - PIPE_COLS * PIPE_CELL) / 2;
    const uint8_t offsetY = (SCREEN_HEIGHT - PIPE_ROWS * PIPE_CELL) / 2;

    uint32_t now = to_ms_since_boot(get_absolute_time());
    for (uint8_t steps = 0; steps < PIPE_MAX_STEPS && now - pipeLastStep >= PIPE_STEP_MS; ++steps) {
        pipeLastStep += PIPE_STEP_MS;
        bool anyAlive = false;
        for (uint8_t i = 0; i < PIPE_HEADS; ++i) {
            if (pipeHeads[i].alive || spawnPipe(pipeHeads[i])) {
                stepPipe(pipeHeads[i]);
                anyAlive |= pipeHeads[i].alive;
            }
        }

        if (!anyAlive && pipeFilled >= PIPE_COLS * PIPE_ROWS) {
            if (pipeFullSince == 0) {
                pipeFullSince = now;
            } else if (now - pipeFullSince >= PIPE_FULL_HOLD_MS) {
                initPipeScene();
                break;
            }
        }
    }
    if (now - pipeLastStep >= PIPE_STEP_MS) {
        pipeLastStep = now;
    }

    const uint8_t PIPE_THICKNESS = 3;
    const uint8_t PIPE_JOINT_SIZE = 5;
    const uint8_t body = (PIPE_CELL - PIPE_THICKNESS) / 2;
    const uint8_t joint = (PIPE_CELL - PIPE_JOINT_SIZE) / 2;
    for (uint8_t y = 0; y < PIPE_ROWS; ++y) {
        for (uint8_t x = 0; x < PIPE_COLS; ++x) {
            uint8_t cell = pipeCells[y][x];
            if (cell == 0) continue;

            uint16_t px = offsetX + x * PIPE_CELL;
            uint16_t py = offsetY + y * PIPE_CELL;
            if (cell & PIPE_JOINT) {
                fillPipeRect(px + joint, py + joint, PIPE_JOINT_SIZE, PIPE_JOINT_SIZE);
            } else {
                fillPipeRect(px + body, py + body, PIPE_THICKNESS, PIPE_THICKNESS);
            }
            if (cell & PIPE_NORTH) fillPipeRect(px + body, py, PIPE_THICKNESS, body);
            if (cell & PIPE_SOUTH) fillPipeRect(px + body, py + body + PIPE_THICKNESS, PIPE_THICKNESS, PIPE_CELL - body - PIPE_THICKNESS);
            if (cell & PIPE_WEST)  fillPipeRect(px, py + body, body, PIPE_THICKNESS);
            if (cell & PIPE_EAST)  fillPipeRect(px + body + PIPE_THICKNESS, py + body, PIPE_CELL - body - PIPE_THICKNESS, PIPE_THICKNESS);
        }
    }
}

void DisplaySaverScreen::initToasters() {
    for (uint16_t i = 0; i < numberOfToasters; ++i) {
        double scale = 1.0 / ((i/2)+2);
        int16_t dx = (-1 - rand() % 2);
        int16_t dy = (1 + rand() % 2);

        toasters.push_back({
            scale,
            static_cast<int16_t>(SCREEN_WIDTH - toasterSpriteWidth * scale),
            static_cast<int16_t>(rand() % (SCREEN_HEIGHT - static_cast<int16_t>(toasterSpriteHeight * scale))),
            static_cast<int16_t>(dx),
            static_cast<int16_t>(dy)
        });
    }
}

void DisplaySaverScreen::drawToasterScene() {
    for (uint16_t i = 0; i < toasters.size(); ++i) {
        ToastParams& sprite = toasters[i];

        getRenderer()->drawSprite((uint8_t *)bitmapGP2040Logo, toasterSpriteWidth, toasterSpriteHeight, 0, sprite.x, sprite.y, 0, sprite.scale);

        sprite.x += sprite.dx;
        sprite.y += sprite.dy;

        if (sprite.x + toasterSpriteWidth * sprite.scale < 0) {
            sprite.x = SCREEN_WIDTH;
            sprite.y = rand() % (SCREEN_HEIGHT - static_cast<int16_t>(toasterSpriteHeight * sprite.scale));
            sprite.dx = (-1 - rand() % 2);
            sprite.dy = (1 + rand() % 2);
        }

        if (sprite.y > SCREEN_HEIGHT) {
            sprite.y = 0;
        }
    }
}

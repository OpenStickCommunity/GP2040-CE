#include "StatsScreen.h"

#include "pico/stdlib.h"
#include "version.h"
#include "drivermanager.h"

void StatsScreen::init() {
    getRenderer()->clearScreen();

    header = new GPLabel();
    header->setRenderer(getRenderer());
    header->setText("[GP2040-CE Stats]");
    header->setPosition(2, 0);
    addElement(header);

    version = new GPLabel();
    version->setRenderer(getRenderer());
    version->setText("Version: " GP2040VERSIONID);
    version->setPosition(0, 1);
    addElement(version);

    build = new GPLabel();
    build->setRenderer(getRenderer());
    build->setText("Build: " GP2040BUILD);
    build->setPosition(0, 2); 
    addElement(build);

    board = new GPLabel();
    board->setRenderer(getRenderer());
    board->setText("Board: " GP2040_BOARDCONFIG);
    board->setPosition(0, 3);
    addElement(board);

    boardType = new GPLabel();
    boardType->setRenderer(getRenderer());
    boardType->setText("Type: " GP2040CONFIG);
    boardType->setPosition(0, 4); 
    addElement(boardType);

    arch = new GPLabel();
    arch->setRenderer(getRenderer());
    arch->setText("Arch: " GP2040PLATFORM);
    arch->setPosition(0, 5); 
    addElement(arch);

    exit = new GPLabel();
    exit->setRenderer(getRenderer());
    exit->setText("B2 to Return");
    exit->setPosition(5, 7); 
    addElement(exit);
}

void StatsScreen::shutdown() {
    clearElements();
}

void StatsScreen::drawScreen() {
}

int8_t StatsScreen::update() {
    if (DriverManager::getInstance().isConfigMode()) {
        uint16_t buttonState = getGamepad()->state.buttons;
        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B2) {
                prevButtonState = 0;
                return DisplayMode::CONFIG_INSTRUCTION;
            }
        }
        prevButtonState = buttonState;
    }

    return -1; // -1 means no change in screen state
}

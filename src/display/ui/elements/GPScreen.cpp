#include "GPScreen.h"

void GPScreen::draw() {
    getRenderer()->clearScreen();

    // draw the display list
    for (uint16_t i=0; i<displayList.size(); i++) {
        displayList.at(i)->draw();
    }
    drawScreen();
    
    getRenderer()->render();
}

int8_t GPScreen::update() { 
    return 0;
}

void GPScreen::setGamepadState(GamepadState state) {
    GPWidget::setGamepadState(state);
    for (uint16_t i=0; i<displayList.size(); i++) {
        displayList.at(i)->setGamepadState(state);
    }
}

uint16_t GPScreen::addElement(GPWidget* element) {
    displayList.push_back(element);
    element->setID(displayList.size()-1);
    return element->getID();
}

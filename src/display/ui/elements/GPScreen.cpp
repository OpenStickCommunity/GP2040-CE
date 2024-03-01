#include "GPScreen.h"

void GPScreen::draw() {
    getRenderer()->clearScreen();

    // draw the display list
    std::sort(displayList.begin(), displayList.end(), [](GPWidget* a, GPWidget* b){ return a->getPriority() > b->getPriority(); });
    for (uint16_t i=0; i<displayList.size(); i++) {
        displayList.at(i)->draw();
    }
    drawScreen();
    
    getRenderer()->render();
}

int8_t GPScreen::update() { 
    return 0;
}

GPWidget* GPScreen::addElement(GPWidget* element) {
    displayList.push_back(element);
    element->setID(displayList.size()-1);
    return element;
}

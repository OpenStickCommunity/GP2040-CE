#include "GPScreen.h"

void GPScreen::draw() {
    getRenderer()->clearScreen();

    // draw the display list
    if ( displayList.size() > 0 ) {
        std::sort(displayList.begin(), displayList.end(), [](GPWidget* a, GPWidget* b) {
            return a->getPriority() > b->getPriority();
        });
        for(std::vector<GPWidget*>::iterator it = displayList.begin(); it != displayList.end(); it++) {
            // Gamepad value actions are read in the component draw functions
            (*it)->draw();
        }
    }
    drawScreen();
    getRenderer()->render();
}

GPWidget* GPScreen::addElement(GPWidget* element) {
    displayList.push_back(element);
    element->setID(displayList.size()-1);
    return element;
}

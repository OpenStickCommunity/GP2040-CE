#include "GPScreen.h"

void GPScreen::draw() {
    getRenderer()->clearScreen();

    // draw the display list
    if ( displayList.size() > 0 ) {
        std::sort(displayList.begin(), displayList.end(), [](std::unique_ptr<GPWidget>& a, std::unique_ptr<GPWidget>& b){ return a->getPriority() > b->getPriority(); });
        for(std::vector<std::unique_ptr<GPWidget>>::iterator it = displayList.begin(); it != displayList.end(); ++it) {
            (*it)->draw();
        }
    }
    drawScreen();
    getRenderer()->render();
}

int8_t GPScreen::update() { 
    return 0;
}

void GPScreen::clear() {
    if (displayList.size() > 0) {
        displayList.clear();
        displayList.shrink_to_fit();
    }
}

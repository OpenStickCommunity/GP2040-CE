#include "GPMenu.h"

#include <string>

void GPMenu::draw() {
    uint16_t baseX = this->x;
    uint16_t baseY = this->y;

    uint16_t menuWidth = this->menuSizeX * 6;
    uint16_t menuHeight = this->menuSizeY * 8;

    uint16_t dataSize = this->getDataSize();
    uint16_t totalPages = (dataSize + this->menuSizeY - 1) / this->menuSizeY;
    uint16_t itemPage = (this->menuIndex / this->menuSizeY);

    std::string pageDisplay = "";
    pageDisplay += "ID: " + std::to_string(this->menuIndex);
    getRenderer()->drawText(0, 1, pageDisplay.c_str());

    if (this->menuEntryData->size() > 0) {
        for (uint8_t menuLine = 0; menuLine < this->menuSizeY; menuLine++) {
            getRenderer()->drawText(2, 2+menuLine, this->menuEntryData->at((itemPage*dataSize)+menuLine).label);
        }
    }

    //getRenderer()->drawRectangle(baseX, baseY, (baseX+menuWidth)-1, (baseY+menuHeight)-1, 0, 0, 0);

    // draw cursor
    getRenderer()->drawText(1, 2+this->menuIndex, ">");

    if (this->menuSizeY < dataSize) {
        // the number of lines available to draw is larger than the size of the menu. clip and show pagination.
        bool showUpNav = ((itemPage-1) > -1);
        bool showDownNav = ((itemPage+1) < totalPages);

        // check position in relation to displayed page items
        if ((itemPage > 0) && (showUpNav)) {
            getRenderer()->drawRectangle(baseX, baseY, (baseX+menuWidth)-1, baseY+5, 1, 1, 0);
        }

        if ((itemPage < totalPages) && (showDownNav)) {
            getRenderer()->drawRectangle(baseX, (baseY+menuHeight), (baseX+menuWidth)-1, (baseY+menuHeight)+5, 1, 1, 0);
        }
    }
}

#include "GPMenu.h"

#include <string>

void GPMenu::draw() {
    if (this->getVisibility()) {
        uint16_t baseX = this->x;
        uint16_t baseY = this->y;

        uint16_t menuWidth = this->menuSizeX * 6;
        uint16_t menuHeight = this->menuSizeY * 8;

        uint16_t dataSize = this->getDataSize();
        uint16_t totalPages = (dataSize + this->menuSizeY - 1) / this->menuSizeY;
        uint16_t itemPage = (this->menuIndex / this->menuSizeY);

        int16_t currPageItems = (dataSize - (itemPage * this->menuSizeY));
        if (currPageItems > this->menuSizeY) {
            currPageItems = this->menuSizeY;
        } else if (currPageItems <= 0) {
            currPageItems = 0;
        }

        getRenderer()->drawText((21-this->menuTitle.length()) / 2, 0, this->menuTitle.c_str());

        std::string pageDisplay = "";
        pageDisplay += "Page: " + std::to_string(itemPage+1) + "/" + std::to_string(totalPages);
        getRenderer()->drawText(11, 7, pageDisplay.c_str());

        if (this->menuEntryData->size() > 0) {
            for (uint8_t menuLine = 0; menuLine < currPageItems; menuLine++) {
                uint8_t pageLine = (this->menuSizeY * itemPage) + menuLine;
                int32_t lineValue = this->menuEntryData->at(pageLine).optionValue;
                bool showCurrentOption = false;
                if (lineValue != -1) {
                    showCurrentOption = (this->menuEntryData->at(pageLine).currentValue() == this->menuEntryData->at(pageLine).optionValue);
                }
                getRenderer()->drawText(2, 2+menuLine, this->menuEntryData->at(pageLine).label + (showCurrentOption ? " *" : ""));
            }
        }

        // draw cursor
        getRenderer()->drawText(1, 2+(this->menuIndex % this->menuSizeY), CHAR_RIGHT);
    }
}

#include "GPMenu.h"

#include <string>

extern uint32_t getMillis();

// screen is 21 characters wide, menu items start at column 2
#define MENU_SCREEN_COLUMNS 21
#define MENU_ITEM_COLUMN 2
#define MENU_SCROLL_HOLD_MS 1500
#define MENU_SCROLL_STEP_MS 300

std::string GPMenu::scrollText(const std::string& text, uint16_t width, ScrollState& state) {
    if (text.size() <= width) return text;

    uint16_t overflow = text.size() - width;
    uint32_t now = getMillis();
    // hold at the start and the end so both ends can be read
    uint32_t delay = (state.position == 0 || state.position == overflow) ? MENU_SCROLL_HOLD_MS : MENU_SCROLL_STEP_MS;
    if (now - state.lastStep >= delay) {
        state.position = (state.position < overflow) ? state.position + 1 : 0;
        state.lastStep = now;
    }
    return text.substr(state.position, width);
}

void GPMenu::draw() {
    if (this->getVisibility()) {
        uint16_t dataSize = this->getDataSize();
        uint16_t totalPages = (dataSize + this->menuSizeY - 1) / this->menuSizeY;
        uint16_t itemPage = (this->menuIndex / this->menuSizeY);

        int16_t currPageItems = (dataSize - (itemPage * this->menuSizeY));
        if (currPageItems > this->menuSizeY) {
            currPageItems = this->menuSizeY;
        } else if (currPageItems <= 0) {
            currPageItems = 0;
        }

        // start scrolling from the beginning when the selection or title changes
        if (this->scrollMenu != this->menuEntryData || this->scrollIndex != this->menuIndex) {
            this->scrollMenu = this->menuEntryData;
            this->scrollIndex = this->menuIndex;
            this->itemScroll = ScrollState{0, getMillis()};
        }
        if (this->scrollTitle != this->menuTitle) {
            this->scrollTitle = this->menuTitle;
            this->titleScroll = ScrollState{0, getMillis()};
        }

        if (this->menuTitle.length() > MENU_SCREEN_COLUMNS) {
            getRenderer()->drawText(0, 0, scrollText(this->menuTitle, MENU_SCREEN_COLUMNS, this->titleScroll));
        } else {
            getRenderer()->drawText((MENU_SCREEN_COLUMNS-this->menuTitle.length()) / 2, 0, this->menuTitle);
        }

        std::string pageDisplay = "";
        pageDisplay += "Page: " + std::to_string(itemPage+1) + "/" + std::to_string(totalPages);
        getRenderer()->drawText(11, 7, pageDisplay);

        if (this->menuEntryData->size() > 0) {
            for (uint8_t menuLine = 0; menuLine < currPageItems; menuLine++) {
                uint8_t pageLine = (this->menuSizeY * itemPage) + menuLine;
                int32_t lineValue = this->menuEntryData->at(pageLine).optionValue;
                bool showCurrentOption = false;
                if (lineValue != -1) {
                    showCurrentOption = (this->menuEntryData->at(pageLine).currentValue() == this->menuEntryData->at(pageLine).optionValue);
                }
                // keep the " *" marker in place and scroll or cut the label to fit
                const std::string suffix = showCurrentOption ? " *" : "";
                const uint16_t labelWidth = MENU_SCREEN_COLUMNS - MENU_ITEM_COLUMN - suffix.size();
                const std::string& label = this->menuEntryData->at(pageLine).label;
                std::string shown = (pageLine == this->menuIndex) ? scrollText(label, labelWidth, this->itemScroll) : label.substr(0, labelWidth);
                getRenderer()->drawText(MENU_ITEM_COLUMN, 2+menuLine, shown + suffix);
            }
        }

        // draw cursor
        getRenderer()->drawText(1, 2+(this->menuIndex % this->menuSizeY), CHAR_RIGHT);
    }
}

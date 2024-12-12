#ifndef _GPMENU_H_
#define _GPMENU_H_

#include "GPWidget.h"
#include "GPShape.h"
#include "GPGFX_UI_types.h"

class GPMenu : public GPShape {
    public:
        void draw();
        GPMenu* setMenuSize(uint16_t sizeX, uint16_t sizeY) { this->menuSizeX = sizeX; this->menuSizeY = sizeY; return this; }

        uint16_t getDataSize() { return this->menuEntryData->size(); };

        void setIndex(uint16_t pos) { this->menuIndex = pos; };
        uint16_t getIndex() { return this->menuIndex; };

        void setMenuData(std::vector<MenuEntry>* menu) { this->menuEntryData = menu; };
        void setMenuTitle(std::string title) { this->menuTitle = title; };
    private:
        uint16_t menuSizeX = 0;
        uint16_t menuSizeY = 0;

        uint16_t menuLines = 15;
        uint16_t menuIndex = 0;

        std::vector<MenuEntry>* menuEntryData;
        std::string menuTitle;
};

#endif
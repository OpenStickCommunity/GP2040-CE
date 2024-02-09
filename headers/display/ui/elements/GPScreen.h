#ifndef _GPSCREEN_H_
#define _GPSCREEN_H_

#include <vector>
#include <cstring>
#include "GPWidget.h"

class GPScreen : public GPWidget {
    public:
        void draw();
        std::string header = "";
        std::string footer = "";
        virtual int8_t update();
        uint16_t addElement(GPWidget* element);
    protected:
        virtual void drawScreen() = 0;

    private:
        std::vector<GPWidget*> displayList;
};

#endif
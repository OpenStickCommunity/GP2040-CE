#ifndef _GPSCREEN_H_
#define _GPSCREEN_H_

#include <vector>
#include <cstring>
#include "GPWidget.h"

class GPScreen : public GPWidget {
    public:
        void draw();
        virtual int8_t update() = 0;
        void clear();
        virtual void init() = 0;
    protected:
        virtual void drawScreen() = 0;
        GPWidget* addElement(GPWidget* element) {
            displayList.push_back(element);
            element->setID(displayList.size()-1);
            return element;
        }
    private:
        std::vector<GPWidget*> displayList;
};

#endif
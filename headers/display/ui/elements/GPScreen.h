#ifndef _GPSCREEN_H_
#define _GPSCREEN_H_

#include <vector>
#include <cstring>
#include "GPWidget.h"

class GPScreen : public GPWidget {
    public:
        GPScreen(){}
        virtual ~GPScreen() {}
        void draw();
        virtual int8_t update() = 0;
        void clear();
        virtual void init() = 0;
        virtual void shutdown() = 0;
    protected:
        virtual void drawScreen() = 0;
        GPWidget * addElement(GPWidget* element) {
            displayList.push_back(element);
            element->setID(displayList.size()-1);
            return element;
        }
        void clearElements() {
            for(std::vector<GPWidget*>::iterator it = displayList.begin(); it != displayList.end(); ++it) {
                delete (*it);
            }
            displayList.clear();
        }
    private:
        std::vector<GPWidget*> displayList;
};

#endif
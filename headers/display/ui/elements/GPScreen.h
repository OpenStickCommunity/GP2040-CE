#ifndef _GPSCREEN_H_
#define _GPSCREEN_H_

#include <vector>
#include <cstring>
#include "GPWidget.h"

class GPScreen : public GPWidget {
    public:
        void draw();
        virtual int8_t update() = 0;
        virtual void init() = 0;
    protected:
        virtual void drawScreen() = 0;
        GPWidget* addElement(GPWidget* element);
    private:
        std::vector<GPWidget*> displayList;
};

#endif
#ifndef _GPWIDGET_H_
#define _GPWIDGET_H_

#include "GPGFX.h"
#include "GPGFX_UI.h"

class GPWidget : public GPGFX_UI {
    public:
        GPWidget() {}
        GPWidget(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~GPWidget(){}
        virtual void draw() {}
        virtual int8_t update() { return 0; }

        void setPosition(uint16_t x, uint16_t y) { this->x = x; this->y = y; }

        void setStrokeColor(uint16_t color) { this->strokeColor = color; }
        void setFillColor(uint16_t color) { this->fillColor = color; }

        void setID(uint16_t id) { this->_ID = id; }
        uint16_t getID() { return this->_ID; }
        
        void setPriority(uint16_t priority) { this->_priority = priority; }
        uint16_t getPriority() { return this->_priority; }

        void setViewport(uint16_t top, uint16_t left, uint16_t bottom, uint16_t right) { this->_viewport.top = top; this->_viewport.left = left; this->_viewport.bottom = bottom; this->_viewport.right = right; }
        void setViewport(GPViewport viewport) { this->_viewport = viewport; }
        GPViewport getViewport() { return this->_viewport; }

        double getScaleX() { return ((double)(this->getViewport().right - this->getViewport().left) / (double)(getRenderer()->getDriver()->getMetrics()->width)); }
        double getScaleY() { return ((double)(this->getViewport().bottom - this->getViewport().top) / (double)(getRenderer()->getDriver()->getMetrics()->height)); }

        void setVisibility(bool visible) { this->_visibility = visible; }
        bool getVisibility() { return this->_visibility; }
    protected:
        uint16_t x = 0;
        uint16_t y = 0;

        uint16_t strokeColor = 0;
        uint16_t fillColor = 0;
        uint16_t _ID;
        uint16_t _priority = 0;
        bool _visibility = true;

        GPViewport _viewport;
};

#endif
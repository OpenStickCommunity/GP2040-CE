#ifndef _GPLABEL_H_
#define _GPLABEL_H_

#include "GPWidget.h"

class GPLabel : public GPWidget {
    public:
        void draw();

        void setWidth(uint16_t width) { this->_width = width; }
        uint16_t getWidth() { return this->_width; }

        void setText(std::string text) { this->_text = text; }
        std::string getText() { return this->_text; }

        void setScrolling(bool scrolling) { this->_scrolling = scrolling; }
        void setDelimiter(std::string delimiter) { this->_delimiter = delimiter; }
    private:
        std::string _text = "";
        uint16_t _width = 21;

        bool _scrolling = true;
        uint16_t _scrollPosition = 0;
        uint32_t _scrollDelay = 500;
        uint32_t _scrollDelayStart = 2000;
        uint32_t _lastScrollTime = 0;

        std::string _delimiter = "";
};

#endif
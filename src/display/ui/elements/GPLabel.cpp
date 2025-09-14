#include "GPLabel.h"

void GPLabel::draw() {
    std::string label = this->getText();

    this->_delimiter = ": ";

    if (!this->_scrolling) {
        getRenderer()->drawText(x, y, label.c_str());
    } else {
        std::string prefix, scrollText;
        size_t delimiterPos = label.find(this->_delimiter);

        if (delimiterPos != std::string::npos) {
            prefix = label.substr(0, delimiterPos + this->_delimiter.size()); // include the colon
            scrollText = label.substr(delimiterPos + this->_delimiter.size());
        } else {
            // No delimiter -> scroll whole thing
            prefix = "";
            scrollText = label;
        }

        size_t scrollWidth = (delimiterPos != std::string::npos) ? (this->_width > prefix.size() ? this->_width - prefix.size() : 0) : this->_width;

        if ((scrollWidth > 0) && (scrollWidth < scrollText.size())) {
            uint16_t labelOverflow = scrollText.size() - scrollWidth;

            if (labelOverflow > 0) {
                std::string doubled = scrollText + scrollText;
                std::string window = doubled.substr(this->_scrollPosition, scrollWidth);

                std::string display = prefix + window;
                getRenderer()->drawText(x, y, display.c_str());

                uint32_t now = getMillis();
                uint32_t delay = (_scrollPosition == 0) ? _scrollDelayStart : _scrollDelay;

                if (now - this->_lastScrollTime >= delay) {
                    if (this->_scrollPosition < labelOverflow) {
                        this->_scrollPosition++;
                    } else {
                        this->_scrollPosition = 0;
                    }
                    this->_lastScrollTime = now;
                }
            } else {
                getRenderer()->drawText(x, y, label.c_str());
            }
        } else {
            getRenderer()->drawText(x, y, label.c_str());
        }
    }
}

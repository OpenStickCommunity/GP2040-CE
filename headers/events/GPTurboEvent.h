#ifndef _GPTURBOEVENT_H_
#define _GPTURBOEVENT_H_

#include <string>

class GPTurboEvent : public GPEvent {
    public:
        GPTurboEvent() {}
        ~GPTurboEvent() {}

        std::string name() { return this->_name; }
    private:
        std::string _name = "turbo";
};

class GPTurboReinitEvent : public GPTurboEvent {
    public:
        GPTurboReinitEvent() {}
        ~GPTurboReinitEvent() {}

        std::string name() { return this->_name; }
    private:
        std::string _name = "turboReinit";
};

class GPTurboChangeEvent : public GPTurboEvent {
    public:
        GPTurboChangeEvent() {}
        GPTurboChangeEvent(uint8_t prev, uint8_t curr) {
            this->previousValue = prev;
            this->currentValue = curr;
        }
        ~GPTurboChangeEvent() {}

        std::string name() { return this->_name; }

        uint8_t previousValue;
        uint8_t currentValue;
    private:
        std::string _name = "turboChange";
};

#endif
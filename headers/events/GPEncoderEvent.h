#ifndef _GPENCODEREVENT_H_
#define _GPENCODEREVENT_H_

#include <string>

class GPEncoderEvent : public GPEvent {
    public:
        GPEncoderEvent() {}
        GPEncoderEvent(uint8_t id, int8_t dir) {
            this->encoder = id;
            this->direction = dir;
        }
        ~GPEncoderEvent() {}

        uint8_t encoder = 0;
        int8_t direction = 0;

        std::string name() { return this->_name; }
    private:
        std::string _name = "encoder";
};

#endif
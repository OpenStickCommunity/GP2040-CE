#ifndef _GPEVENT_H_
#define _GPEVENT_H_

#include <string>

#define GPEVENT_CALLBACK(x) ([this](GPEvent* event){x})

class GPEvent {
    public:
        GPEvent() {}
        ~GPEvent() {}

        virtual std::string name() { return this->_name; }
    private:
        std::string _name = "event";
};

#endif
#ifndef _GPEVENT_H_
#define _GPEVENT_H_

#define GPEVENT_CALLBACK(x) ([this](GPEvent* event){x;})

class GPEvent {
    public:
        GPEvent() {}
        virtual ~GPEvent() {}

        virtual GPEventType eventType() { return this->_eventType; }
    private:
        GPEventType _eventType = GP_EVENT_BASE;
};

#endif
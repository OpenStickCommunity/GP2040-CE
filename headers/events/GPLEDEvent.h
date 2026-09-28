#ifndef _GPLEDEVENT_H_
#define _GPLEDEVENT_H_

class GPLEDEvent : public GPEvent {
    public:
        GPLEDEvent() {}
        GPLEDEvent(bool changeProfile, bool changeBase, bool changeCase, bool changePressed, bool changeBrightness) {
            bChangeProfile = changeProfile;
            bChangeBase = changeBase;
            bChangeCase = changeCase;
            bChangePressed = changePressed;
            bChangeBrightness = changeBrightness;
        }
        virtual ~GPLEDEvent() {}

        GPEventType eventType() { return this->_eventType; }

        bool bChangeProfile;
        bool bChangeBase;
        bool bChangeCase;
        bool bChangePressed;
        bool bChangeBrightness;
    private:
        GPEventType _eventType = GP_EVENT_LED_CHANGE;

};

#endif
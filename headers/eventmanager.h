#ifndef _EVENTMANAGER_H_
#define _EVENTMANAGER_H_

#include <map>
#include <vector>
#include <string>
#include <deque>
#include <array>
#include <functional>
#include <cctype>
#include "config.pb.h"
#include "enums.pb.h"

#include "GPEvent.h"
#include "GPGamepadEvent.h"
#include "GPEncoderEvent.h"
#include "GPProfileEvent.h"
#include "GPRestartEvent.h"
#include "GPUSBHostEvent.h"

#define EVENTMGR EventManager::getInstance()

class EventManager {
    public:
        typedef std::function<void(GPEvent* event)> EventFunction;
        typedef std::pair<GPEventType, std::vector<EventFunction>> EventEntry;

        EventManager(EventManager const&) = delete;
        void operator=(EventManager const&)  = delete;
        static EventManager& getInstance() // Thread-safe storage ensures cross-thread talk
        {
            static EventManager instance;
            return instance;
        }

        void registerEventHandler(GPEventType eventType, EventFunction handler);
        void triggerEvent(GPEvent* event);
    private:
        EventManager(){}

        std::vector<std::pair<GPEventType, std::vector<EventFunction>>> eventList;
};

#endif
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
#include "GPMenuNavigateEvent.h"
#include "GPProfileEvent.h"
#include "GPRestartEvent.h"
#include "GPStorageSaveEvent.h"
#include "GPSystemErrorEvent.h"
#include "GPSystemRebootEvent.h"
#include "GPUSBHostEvent.h"

#define EVENTMGR EventManager::getInstance()

class EventManager {
    public:
        typedef std::function<void(GPEvent* event)> EventFunction;
        // Each handler is paired with an opaque owner token (typically `this` of the
        // registering object) so we can reliably unregister capturing lambdas, whose
        // std::function::target<void(*)(GPEvent*)>() is always nullptr and cannot be
        // compared by callable target.
        struct HandlerEntry {
            void* owner;
            EventFunction handler;
        };
        typedef std::pair<GPEventType, std::vector<HandlerEntry>> EventEntry;

        EventManager(EventManager const&) = delete;
        void operator=(EventManager const&)  = delete;
        static EventManager& getInstance() // Thread-safe storage ensures cross-thread talk
        {
            static EventManager instance;
            return instance;
        }

        void init();
        void clearEventHandlers();

        // Register a handler. `owner` should be a stable pointer (typically `this`)
        // so unregisterEventHandler can match it later.
        void registerEventHandler(GPEventType eventType, EventFunction handler, void* owner = nullptr);
        // Remove every handler registered for `eventType` by `owner`.
        void unregisterEventHandler(GPEventType eventType, void* owner);
        void triggerEvent(GPEvent* event);
    private:
        EventManager(){}

        std::vector<EventEntry> eventList;
};

#endif
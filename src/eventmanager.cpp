#include "eventmanager.h"
#include "storagemanager.h"
#include "enums.pb.h"

void EventManager::init() {
    clearEventHandlers();
}

void EventManager::registerEventHandler(GPEventType eventType, EventFunction handler, void* owner) {
    typename std::vector<EventEntry>::iterator it = std::find_if(eventList.begin(), eventList.end(), [&eventType](const EventEntry& entry) { return entry.first == eventType; });

    HandlerEntry entry{owner, handler};
    if (it != eventList.end()) {
        it->second.push_back(entry);
    } else {
        eventList.emplace_back(eventType, std::vector<HandlerEntry>{entry});
    }
}

void EventManager::unregisterEventHandler(GPEventType eventType, void* owner) {
    typename std::vector<EventEntry>::iterator it = std::find_if(eventList.begin(), eventList.end(), [&eventType](const EventEntry& entry) { return entry.first == eventType; });

    if (it == eventList.end()) return;

    // Remove every handler registered by this owner. std::function has no general
    // operator==, and capturing lambdas always return nullptr from target<plain
    // function pointer>(), so identity must come from the owner token recorded at
    // register time.
    auto& handlers = it->second;
    handlers.erase(
        std::remove_if(handlers.begin(), handlers.end(),
            [owner](const HandlerEntry& e) { return e.owner == owner; }),
        handlers.end());
}

void EventManager::triggerEvent(GPEvent* event) {
    GPEventType eventType = event->eventType();
    for (typename std::vector<EventEntry>::const_iterator it = eventList.begin(); it != eventList.end(); ++it) {
        if (it->first == eventType) {
            const std::vector<HandlerEntry>& handlers = it->second;
            for (typename std::vector<HandlerEntry>::const_iterator entry = handlers.begin(); entry != handlers.end(); ++entry) {
                entry->handler(event);
            }
        }
    }
    delete event;
}

void EventManager::clearEventHandlers() {
    eventList.clear();
}

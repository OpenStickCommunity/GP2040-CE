#include "eventmanager.h"
#include <algorithm>
#include "storagemanager.h"
#include "enums.pb.h"

void EventManager::init() {
    clearEventHandlers();
}

void EventManager::registerEventHandler(GPEventType eventType, EventFunction handler, const void* owner) {
    typename std::vector<EventEntry>::iterator it = std::find_if(eventList.begin(), eventList.end(), [&eventType](const EventEntry& entry) { return entry.first == eventType; });

    if (it != eventList.end()) {
        // If the event already exists, add the handler to its vector
        it->second.emplace_back(owner, handler);
    } else {
        // If the event does not exist, create a new entry with the handler
        eventList.emplace_back(eventType, std::vector<EventHandler>{{owner, handler}});
    }
}

void EventManager::unregisterEventHandlers(const void* owner) {
    for (EventEntry& entry : eventList) {
        std::vector<EventHandler>& handlers = entry.second;
        handlers.erase(std::remove_if(handlers.begin(), handlers.end(),
            [owner](const EventHandler& handler) { return handler.first == owner; }), handlers.end());
    }
}

void EventManager::triggerEvent(GPEvent* event) {
    GPEventType eventType = event->eventType();
    for (typename std::vector<EventEntry>::const_iterator it = eventList.begin(); it != eventList.end(); ++it) {
        if (it->first == eventType) {
            // Call all event handlers for the specified event
            const std::vector<EventHandler>& handlers = it->second;
            for (typename std::vector<EventHandler>::const_iterator handler = handlers.begin(); handler != handlers.end(); ++handler) {
                handler->second(event);
            }
        }
    }
    delete event;
}

void EventManager::clearEventHandlers() {

}

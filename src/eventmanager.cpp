#include "eventmanager.h"
#include "storagemanager.h"
#include "enums.pb.h"

void EventManager::init() {
    clearEventHandlers();
}

void EventManager::registerEventHandler(GPEventType eventType, EventFunction handler) {
    typename std::vector<EventEntry>::iterator it = std::find_if(eventList.begin(), eventList.end(), [&eventType](const EventEntry& entry) { return entry.first == eventType; });

    if (it != eventList.end()) {
        // If the event already exists, add the handler to its vector
        it->second.push_back(handler);
    } else {
        // If the event does not exist, create a new entry with the handler
        eventList.emplace_back(eventType, std::vector<EventFunction>{handler});
    }
}

void EventManager::unregisterEventHandler(GPEventType eventType, EventFunction handler) {
    typename std::vector<EventEntry>::iterator it = std::find_if(eventList.begin(), eventList.end(), [&eventType](const EventEntry& entry) { return entry.first == eventType; });

    // Verify we have this event in our pair list
    if (it != eventList.end()) {
        // Verify we have this function in our function vector
        for(std::vector<EventFunction>::iterator funcIt = it->second.begin(); funcIt != it->second.end(); it++){
            if(*(uint32_t *)(uint8_t *)&handler == *(uint32_t *)(uint8_t *)&(*funcIt)) {
                it->second.erase(funcIt);
                break;
            }
        }
    }
}

void EventManager::triggerEvent(GPEvent* event) {
    GPEventType eventType = event->eventType();
    for (typename std::vector<EventEntry>::const_iterator it = eventList.begin(); it != eventList.end(); ++it) {
        if (it->first == eventType) {
            // Call all event handlers for the specified event
            const std::vector<EventFunction>& handlers = it->second;
            for (typename std::vector<EventFunction>::const_iterator handler = handlers.begin(); handler != handlers.end(); ++handler) {
                (*handler)(event);
            }
        }
    }
    delete event;
}

void EventManager::clearEventHandlers() {

}

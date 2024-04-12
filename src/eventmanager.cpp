#include "eventmanager.h"
#include "storagemanager.h"
#include "enums.pb.h"

void EventManager::registerEventHandler(std::string eventName, EventFunction handler) {
    //printf("EventManager::registerEventHandler %s\n", eventName.c_str());
    typename std::vector<EventEntry>::iterator it = std::find_if(eventList.begin(), eventList.end(), [&eventName](const EventEntry& entry) { return entry.first == eventName; });

    if (it != eventList.end()) {
        // If the event already exists, add the handler to its vector
        it->second.push_back(handler);
    } else {
        // If the event does not exist, create a new entry with the handler
        eventList.emplace_back(eventName, std::vector<EventFunction>{handler});
    }
}

void EventManager::triggerEvent(GPEvent* event) {
    std::string eventName = event->name();
    //printf("EventManager::triggerEvent %s\n", eventName.c_str());
    for (typename std::vector<EventEntry>::const_iterator it = eventList.begin(); it != eventList.end(); ++it) {
        if (it->first == eventName) {
            // Call all event handlers for the specified event
            const std::vector<EventFunction>& handlers = it->second;
            for (typename std::vector<EventFunction>::const_iterator handler = handlers.begin(); handler != handlers.end(); ++handler) {
                (*handler)(event);
            }
        }
    }
}
#ifndef EVENTS_H
#define EVENTS_H

#include <unordered_map>
#include <vector>
#include <string>
enum EventType {
    EVENT_ON_START = 0,
    EVENT_ON_KEY_PRESSED = 1,
    EVENT_ON_SPRITE_CLICKED = 2,
    EVENT_ON_MESSAGE = 3
};

struct EventKey {
    EventType type;
    int code;
};

inline bool operator==(const EventKey& a, const EventKey& b) {
    return a.type == b.type && a.code == b.code;
}

struct EventKeyHash {
    size_t operator()(const EventKey& k) const {
        return (static_cast<size_t>(k.type) * 1315423911u) ^ static_cast<size_t>(k.code);
    }
};

struct EventManager {
    std::unordered_map<EventKey, std::vector<int>, EventKeyHash> table;
};

void event_clear(EventManager& em);
void event_bind(EventManager& em, EventType type, int code, int startBlockId);
const std::vector<int>* event_get(const EventManager& em, EventType type, int code);
void event_bind_message(EventManager& em, const std::string& message, int startBlockId);
const std::vector<int>* event_get_message(const EventManager& em, const std::string& message);
#endif

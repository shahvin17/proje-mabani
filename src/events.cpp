#include "events.h"

void event_clear(EventManager& em) {
    em.table.clear();
}

void event_bind(EventManager& em, EventType type, int code, int startBlockId) {
    EventKey key{type, code};
    em.table[key].push_back(startBlockId);
}

const std::vector<int>* event_get(const EventManager& em, EventType type, int code) {
    EventKey key{type, code};
    auto it = em.table.find(key);
    if (it == em.table.end()) return nullptr;
    return &it->second;
}
static int hash_message(const std::string& msg) {
    return static_cast<int>(std::hash<std::string>{}(msg));
}
void event_bind_message(EventManager& em, const std::string& message, int startBlockId) {
    int msgCode = hash_message(message);
    event_bind(em, EVENT_ON_MESSAGE, msgCode, startBlockId);
}
const std::vector<int>* event_get_message(const EventManager& em, const std::string& message) {
    int msgCode = hash_message(message);
    return event_get(em, EVENT_ON_MESSAGE, msgCode);
}
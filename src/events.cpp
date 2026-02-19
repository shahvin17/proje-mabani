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

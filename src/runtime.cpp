#include "runtime.h"
#include <iostream>

// پیدا کردن بلاک با id
static Block* findBlockById(Project* project, int id) {
    for (size_t i = 0; i < project->blocks.size(); i++) {
        if (project->blocks[i].id == id) {
            return &project->blocks[i];
        }
    }
    return nullptr;
}

void runtime_init(Runtime* rt, Project* project) {
    rt->project = project;
    rt->currentBlockId = -1;
    rt->state = RUNTIME_STOPPED;
    rt->watchdogCounter = 0;
    rt->watchdogLimit = 1000;   // محدودیت ساده برای جلوگیری از حلقه بی‌نهایت
}

void runtime_start(Runtime* rt) {
    if (rt->project->blocks.empty()) {
        std::cout << "No blocks to execute.\n";
        return;
    }

    rt->currentBlockId = rt->project->blocks[0].id;  // فعلاً اولین بلاک
    rt->watchdogCounter = 0;
    rt->state = RUNTIME_RUNNING;

    std::cout << "Runtime started.\n";
}

void runtime_stop(Runtime* rt) {
    rt->state = RUNTIME_STOPPED;
    std::cout << "Runtime stopped.\n";
}

void runtime_executeCurrent(Runtime* rt) {
    if (rt->state != RUNTIME_RUNNING) return;

    Block* block = findBlockById(rt->project, rt->currentBlockId);

    if (!block) {
        std::cout << "Block not found. Stopping.\n";
        runtime_stop(rt);
        return;
    }

    // 🔹 Watchdog
    rt->watchdogCounter++;
    if (rt->watchdogCounter > rt->watchdogLimit) {
        std::cout << "Watchdog limit reached. Possible infinite loop.\n";
        runtime_stop(rt);
        return;
    }

    // 🔹 اجرای ساده (فعلاً فقط print)
    std::cout << "Executing block ID: " << block->id << "\n";

    switch (block->type) {

        case BLOCK_MOVE:
            std::cout << "Move block\n";
            break;

        case BLOCK_TURN:
            std::cout << "Turn block\n";
            break;

        case BLOCK_EVENT_START:
            std::cout << "Event start\n";
            break;

        default:
            std::cout << "Unknown block type\n";
            break;
    }

    // 🔹 رفتن به بلاک بعدی (اجرای ترتیبی ساده)
    if (block->next != -1) {
        rt->currentBlockId = block->next;
    } else {
        runtime_stop(rt);
    }
}

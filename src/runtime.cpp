#include "runtime.h"
#include <iostream>
using namespace std;

static Block* findBlockById(Project* project, int id) {
    if (!project) return nullptr;

    for (size_t i = 0; i < project->blocks.size(); i++) {
        if (project->blocks[i].id == id) {
            return &project->blocks[i];
        }
    }
    return nullptr;
}

static int getFirstBlockId(Project* project) {
    if (!project) return -1;
    if (project->blocks.empty()) return -1;
    return project->blocks[0].id;
}

static void executeBlock(Runtime* rt, Block* block) {
    // Day 2: فعلاً فقط print (اتصال به motion/looks روزهای بعد)
    cout << "Executing block id=" << block->id << " type=" << block->type << endl;

    if (block->type == "move") {
        cout << "  -> Move block" << endl;
    } else if (block->type == "turn") {
        cout << "  -> Turn block" << endl;
    } else {
        cout << "  -> Unknown block type: " << block->type << endl;
    }

    rt->lastExecutedBlockId = block->id;
}

void runtime_init(Runtime* rt, Project* project) {
    rt->project = project;
    rt->currentBlockId = -1;
    rt->state = RUNTIME_STOPPED;

    rt->watchdogCounter = 0;
    rt->watchdogLimit = 1000;

    rt->lastExecutedBlockId = -1;
}

void runtime_start(Runtime* rt) {
    if (!rt || !rt->project) return;

    int firstId = getFirstBlockId(rt->project);
    if (firstId == -1) {
        cout << "Runtime: No blocks to execute." << endl;
        rt->state = RUNTIME_STOPPED;
        rt->currentBlockId = -1;
        return;
    }

    rt->watchdogCounter = 0;
    rt->currentBlockId = firstId;
    rt->state = RUNTIME_RUNNING;

    cout << "Runtime started. firstBlockId=" << rt->currentBlockId << endl;
}

void runtime_stop(Runtime* rt) {
    if (!rt) return;
    rt->state = RUNTIME_STOPPED;
    rt->currentBlockId = -1;
    cout << "Runtime stopped." << endl;
}

void runtime_pause(Runtime* rt) {
    if (!rt) return;
    if (rt->state == RUNTIME_RUNNING) {
        rt->state = RUNTIME_PAUSED;
        cout << "Runtime paused." << endl;
    }
}

void runtime_resume(Runtime* rt) {
    if (!rt) return;
    if (rt->state == RUNTIME_PAUSED) {
        rt->state = RUNTIME_RUNNING;
        cout << "Runtime resumed." << endl;
    }
}

void runtime_tick(Runtime* rt) {
    if (!rt || !rt->project) return;

    if (rt->state != RUNTIME_RUNNING) {
        return; // اگر paused/stopped هست کاری نکن
    }

    Block* block = findBlockById(rt->project, rt->currentBlockId);

    if (!block) {
        cout << "Runtime: Block not found id=" << rt->currentBlockId << endl;
        runtime_stop(rt);
        return;
    }

    // Watchdog: هر tick یک قدم
    rt->watchdogCounter++;
    if (rt->watchdogCounter > rt->watchdogLimit) {
        cout << "Runtime: Watchdog limit reached (" << rt->watchdogLimit << ")." << endl;
        runtime_stop(rt);
        return;
    }

    executeBlock(rt, block);

    // حرکت به بلاک بعدی
    if (block->nextBlockId != -1) {
        rt->currentBlockId = block->nextBlockId;
    } else {
        runtime_stop(rt);
    }
}

bool runtime_isRunning(const Runtime* rt) {
    return rt && rt->state == RUNTIME_RUNNING;
}

bool runtime_isPaused(const Runtime* rt) {
    return rt && rt->state == RUNTIME_PAUSED;
}

void runtime_setWatchdogLimit(Runtime* rt, int limit) {
    if (!rt) return;
    if (limit < 1) limit = 1;
    rt->watchdogLimit = limit;
}
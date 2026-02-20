#include "runtime.h"
#include <iostream>
using namespace std;

/* ===============================
   Helpers
================================ */

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

/* ===============================
   Execute Single Block (Primitive)
================================ */

static void executePrimitive(Runtime* rt, Block* block) {

    cout << "Executing block id=" << block->id
         << " type=" << block->type << endl;

    if (block->type == "move") {
        cout << "  -> Move block" << endl;
    }
    else if (block->type == "turn") {
        cout << "  -> Turn block" << endl;
    }
    else {
        cout << "  -> Unknown block type: "
             << block->type << endl;
    }

    rt->lastExecutedBlockId = block->id;
}

/* ===============================
   Forward declare dispatcher
================================ */
static void executeBlock(Runtime* rt, Block* block);

/* ===============================
   Execute Chain (Day 4)
================================ */

static void executeChain(Runtime* rt, int startBlockId) {

    int currentId = startBlockId;

    while (currentId != -1) {

        Block* block = findBlockById(rt->project, currentId);
        if (!block) return;

        rt->watchdogCounter++;
        if (rt->watchdogCounter > rt->watchdogLimit) {
            cout << "Watchdog limit reached inside chain." << endl;
            runtime_stop(rt);
            return;
        }

        // ✅ FIX: chain باید dispatcher را صدا بزند (نه primitive)
        executeBlock(rt, block);

        currentId = block->nextBlockId;
    }
}

/* ===============================
   Control Blocks
================================ */

static void executeRepeat(Runtime* rt, Block* block) {

    if (block->inputs.size() < 2) {
        cout << "Repeat block missing inputs." << endl;
        return;
    }

    int count   = block->inputs[0];
    int childId = block->inputs[1];

    if (count <= 0) return;

    cout << "Repeat " << count << " times" << endl;

    for (int i = 0; i < count; i++) {

        rt->watchdogCounter++;
        if (rt->watchdogCounter > rt->watchdogLimit) {
            cout << "Watchdog limit reached inside repeat." << endl;
            runtime_stop(rt);
            return;
        }

        cout << "  Repeat iteration " << i+1 << endl;

        executeChain(rt, childId);
    }
}

static void executeIf(Runtime* rt, Block* block) {

    if (block->inputs.size() < 2) {
        cout << "If block missing inputs." << endl;
        return;
    }

    int condition = block->inputs[0];
    int childId   = block->inputs[1];

    if (condition) {
        cout << "If condition TRUE" << endl;
        executeChain(rt, childId);
    }
    else {
        cout << "If condition FALSE" << endl;
    }
}

/* ===============================
   Dispatcher
================================ */

static void executeBlock(Runtime* rt, Block* block) {

    // ✅ FIX: چاپ تکراری حذف شد (primitive خودش چاپ می‌کند)
    // فقط برای controlها چاپ اضافه می‌گذاریم (اختیاری)
    if (block->type == "repeat") {
        cout << "Executing block id=" << block->id
             << " type=" << block->type << endl;
        executeRepeat(rt, block);
    }
    else if (block->type == "if") {
        cout << "Executing block id=" << block->id
             << " type=" << block->type << endl;
        executeIf(rt, block);
    }
    else {
        executePrimitive(rt, block);
    }

    rt->lastExecutedBlockId = block->id;
}

/* ===============================
   Runtime Lifecycle
================================ */

void runtime_init(Runtime* rt, Project* project) {

    rt->project = project;
    rt->currentBlockId = -1;
    rt->state = RUNTIME_STOPPED;

    rt->watchdogCounter = 0;
    rt->watchdogLimit   = 1000;

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
    rt->currentBlockId  = firstId;
    rt->state = RUNTIME_RUNNING;

    cout << "Runtime started. firstBlockId="
         << rt->currentBlockId << endl;
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

/* ===============================
   Tick Engine
================================ */

void runtime_tick(Runtime* rt) {

    if (!rt || !rt->project) return;

    if (rt->state != RUNTIME_RUNNING)
        return;

    Block* block = findBlockById(rt->project,
                                 rt->currentBlockId);

    if (!block) {
        cout << "Runtime: Block not found id="
             << rt->currentBlockId << endl;
        runtime_stop(rt);
        return;
    }

    rt->watchdogCounter++;
    if (rt->watchdogCounter > rt->watchdogLimit) {
        cout << "Runtime: Watchdog limit reached ("
             << rt->watchdogLimit << ")." << endl;
        runtime_stop(rt);
        return;
    }

    executeBlock(rt, block);

    if (block->nextBlockId != -1) {
        rt->currentBlockId = block->nextBlockId;
    } else {
        runtime_stop(rt);
    }
}

/* ===============================
   Utility
================================ */

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
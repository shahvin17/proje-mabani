#include "runtime.h"
#include <iostream>
using namespace std;

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
    rt->watchdogLimit = 1000;
}

void runtime_start(Runtime* rt) {
    if (rt->project->blocks.empty()) {
        cout << "No blocks to execute." << endl;
        return;
    }

    rt->currentBlockId = rt->project->blocks[0].id;
    rt->watchdogCounter = 0;
    rt->state = RUNTIME_RUNNING;

    cout << "Runtime started." << endl;
}

void runtime_stop(Runtime* rt) {
    rt->state = RUNTIME_STOPPED;
    cout << "Runtime stopped." << endl;
}

void runtime_executeCurrent(Runtime* rt) {
    if (rt->state != RUNTIME_RUNNING)
        return;

    Block* block = findBlockById(rt->project, rt->currentBlockId);

    if (!block) {
        cout << "Block not found." << endl;
        runtime_stop(rt);
        return;
    }

    rt->watchdogCounter++;
    if (rt->watchdogCounter > rt->watchdogLimit) {
        cout << "Watchdog limit reached." << endl;
        runtime_stop(rt);
        return;
    }

    cout << "Executing block ID: " << block->id << endl;

    if (block->type == "move") {
        cout << "Move block" << endl;
    }
    else if (block->type == "turn") {
        cout << "Turn block" << endl;
    }
    else {
        cout << "Unknown block type: " << block->type << endl;
    }

    if (block->nextBlockId != -1) {
        rt->currentBlockId = block->nextBlockId;
    }
    else {
        runtime_stop(rt);
    }
}
#ifndef RUNTIME_H
#define RUNTIME_H

#include "core_types.h"

enum RuntimeState {
    RUNTIME_STOPPED,
    RUNTIME_RUNNING,
    RUNTIME_PAUSED
};

struct Runtime {
    Project* project;
    int currentBlockId;
    RuntimeState state;
    int watchdogCounter;
    int watchdogLimit;
};

void runtime_init(Runtime* rt, Project* project);
void runtime_start(Runtime* rt);
void runtime_stop(Runtime* rt);
void runtime_executeCurrent(Runtime* rt);

#endif
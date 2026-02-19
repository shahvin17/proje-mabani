#ifndef RUNTIME_H
#define RUNTIME_H

#include "core_types.h"
#include <string>
#include <vector>
using namespace std;

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

    // برای گزارش/دیباگ
    int lastExecutedBlockId;

    //day4
    std::vector<int> returnStack;   // برگشت بعد از child
    std::vector<int> loopCounter;   // شمارنده repeat
};

// init / lifecycle
void runtime_init(Runtime* rt, Project* project);
void runtime_start(Runtime* rt);
void runtime_stop(Runtime* rt);

// pause/resume
void runtime_pause(Runtime* rt);
void runtime_resume(Runtime* rt);

// tick-based execution (روز 2)
void runtime_tick(Runtime* rt);

// helpers
bool runtime_isRunning(const Runtime* rt);
bool runtime_isPaused(const Runtime* rt);
void runtime_setWatchdogLimit(Runtime* rt, int limit);

#endif
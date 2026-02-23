#ifndef RUNTIME_H
#define RUNTIME_H
#include "core_types.h"
#include <vector>
using namespace std;

enum RuntimeState { RUNTIME_STOPPED, RUNTIME_RUNNING, RUNTIME_PAUSED };

struct ControlFrame {
    int blockId;
    int counter;
    int loop_target;
    int childHeadId;
    int after_loop_id = -1;  // بلوک بعد از پایان loop
    bool is_forever   = false;
};

struct Runtime {
    Project* project;
    int currentBlockId;
    RuntimeState state;
    int watchdogCounter;
    int watchdogLimit;
    std::vector<ControlFrame> controlStack;
    int lastExecutedBlockId;
    std::vector<int> returnStack;
    std::vector<int> loopCounter;
};

void runtime_init(Runtime* rt, Project* project);
void runtime_start(Runtime* rt);
void runtime_stop(Runtime* rt);
void runtime_pause(Runtime* rt);
void runtime_resume(Runtime* rt);
void runtime_tick(Runtime* rt);
bool runtime_isRunning(const Runtime* rt);
bool runtime_isPaused(const Runtime* rt);
void runtime_setWatchdogLimit(Runtime* rt, int limit);

#endif // RUNTIME_H
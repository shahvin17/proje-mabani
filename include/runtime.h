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

// --- تغییر اصلی: اضافه کردن ساختار برای مدیریت حالت حلقه ---
struct ControlFrame {
    int blockId;      // آی‌دی بلوک repeat
    int counter;      // شمارنده فعلی حلقه
    int loop_target;  // تعداد تکرار مورد نیاز
    int childHeadId;  // آی‌دی اولین بلوک داخل حلقه
};

struct Runtime {
    Project* project;
    int currentBlockId;
    RuntimeState state;

    int watchdogCounter;
    int watchdogLimit;

    // --- پشته برای مدیریت حلقه‌های تو در تو ---
    std::vector<ControlFrame> controlStack;

    int lastExecutedBlockId;

    //day4
    std::vector<int> returnStack;   // برگشت بعد از child
    std::vector<int> loopCounter;   // شمارنده repeat

};

// بقیه تعاریف توابع بدون تغییر
void runtime_init(Runtime* rt, Project* project);
void runtime_start(Runtime* rt);
void runtime_stop(Runtime* rt);
void runtime_pause(Runtime* rt);
void runtime_resume(Runtime* rt);
void runtime_tick(Runtime* rt);
bool runtime_isRunning(const Runtime* rt);
bool runtime_isPaused(const Runtime* rt);
void runtime_setWatchdogLimit(Runtime* rt, int limit);

#endif

#ifndef RUNTIME_H
#define RUNTIME_H

#include <SDL2/SDL.h>
#include "core_types.h"
#include <vector>
#include <string>

using namespace std;

enum RuntimeState { RUNTIME_STOPPED, RUNTIME_RUNNING, RUNTIME_PAUSED };

struct ControlFrame {
    int  blockId;
    int  counter;
    int  loop_target;
    int  childHeadId;
    int  after_loop_id = -1;
    bool is_forever    = false;
    bool is_wait       = false;
    unsigned int wait_end_ms = 0;
};

// forward declare
struct SensingManager;

// ── Runtime یک Sprite ────────────────────────────────────────────────────────
// هر sprite برنامه مستقل خودش رو داره
struct SpriteRuntime {
    int  sprite_id      = -1;      // کدام sprite
    int  currentBlockId = -1;      // بلوک فعلی در اجرا
    int  start_block_id = -1;      // when_start block id این sprite
    RuntimeState state  = RUNTIME_STOPPED;
    int  watchdogCounter = 0;
    std::vector<ControlFrame> controlStack;
    int  lastExecutedBlockId = -1;
};

// ── Runtime کل پروژه ────────────────────────────────────────────────────────
// forward declare for SoundManager
struct SoundManager;

struct Runtime {
    Project*               project   = nullptr;
    SensingManager*        sensing   = nullptr;
    SoundManager*          sound_mgr = nullptr;
    std::vector<SpriteRuntime> sprites;
    RuntimeState           state    = RUNTIME_STOPPED;
    int                    watchdogLimit = 100000;
    // backward compat (برای کد قدیمی)
    int  currentBlockId      = -1;
    int  lastExecutedBlockId = -1;
    std::vector<ControlFrame> controlStack;
    std::vector<int> returnStack;
    std::vector<int> loopCounter;
};

void runtime_init(Runtime* rt, Project* project);
void runtime_start(Runtime* rt);
void runtime_stop(Runtime* rt);
void runtime_pause(Runtime* rt);
void runtime_resume(Runtime* rt);
void runtime_tick(Runtime* rt);          // tick همه sprite ها
bool runtime_isRunning(const Runtime* rt);
bool runtime_isPaused(const Runtime* rt);
void runtime_setWatchdogLimit(Runtime* rt, int limit);

// برای هر sprite جداگانه
void runtime_start_sprite(Runtime* rt, int sprite_id);
void runtime_stop_sprite(Runtime* rt, int sprite_id);

#endif // RUNTIME_H
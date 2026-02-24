#ifndef RUNTIME_H
#define RUNTIME_H

#include <SDL2/SDL.h>
#include "core_types.h"
#include <vector>
#include <string>
#include <queue>

using namespace std;

enum RuntimeState { RUNTIME_STOPPED, RUNTIME_RUNNING, RUNTIME_PAUSED };

struct ControlFrame {
    int  blockId=-1, counter=0, loop_target=0, childHeadId=-1, after_loop_id=-1;
    bool is_forever=false, is_wait=false;
    unsigned int wait_end_ms=0;
};

struct SensingManager;
struct SoundManager;

// ── SpriteRuntime ────────────────────────────────────────────────────────────
struct SpriteRuntime {
    int          sprite_id=-1, clone_id=-1;
    int          currentBlockId=-1, start_block_id=-1;
    RuntimeState state=RUNTIME_STOPPED;
    int          watchdogCounter=0;
    bool         is_clone=false;
    bool         waiting_for_answer=false;
    bool         waiting_for_broadcast=false;
    std::string  ask_question="";
    std::vector<ControlFrame> controlStack;
    int          lastExecutedBlockId=-1;
    // step debugger: highlight این بلوک
    int          step_highlight_id=-1;
};

// ── Clone ────────────────────────────────────────────────────────────────────
struct CloneSprite {
    int    clone_id=-1, sprite_id=-1;
    Sprite data;
};

// ── Broadcast pending ────────────────────────────────────────────────────────
struct BroadcastMsg {
    std::string name;
    bool        wait;   // broadcast and wait
};

// ── Runtime ──────────────────────────────────────────────────────────────────
struct Runtime {
    Project*                   project=nullptr;
    SensingManager*            sensing=nullptr;
    SoundManager*              sound_mgr=nullptr;
    std::vector<SpriteRuntime> sprites;
    std::vector<SpriteRuntime> clone_runtimes;
    std::vector<CloneSprite>   clones;
    RuntimeState               state=RUNTIME_STOPPED;
    int                        watchdogLimit=100000;

    // Turbo
    bool turbo_mode=false;
    int  turbo_ticks=30;

    // Ask/Answer
    bool        ask_active=false;
    std::string ask_buffer="";
    std::string answer="";
    int         ask_sprite_id=-1, ask_resume_block=-1;

    // Broadcast queue
    std::queue<BroadcastMsg> broadcast_queue;

    // Step debugger
    bool step_mode=false;         // یک tick در هر فریم و منتظر
    bool step_pending=false;      // کاربر دکمه Step زده
    int  step_highlight_block=-1; // بلوک در حال اجرا

    // Clone
    int next_clone_id=1;

    // backward compat
    int currentBlockId=-1, lastExecutedBlockId=-1;
    std::vector<ControlFrame> controlStack;
};

// ── API ──────────────────────────────────────────────────────────────────────
void runtime_init(Runtime* rt, Project* project);
void runtime_start(Runtime* rt);
void runtime_stop(Runtime* rt);
void runtime_pause(Runtime* rt);
void runtime_resume(Runtime* rt);
void runtime_tick(Runtime* rt);
bool runtime_isRunning(const Runtime* rt);
bool runtime_isPaused(const Runtime* rt);
void runtime_setWatchdogLimit(Runtime* rt, int limit);
void runtime_start_sprite(Runtime* rt, int sprite_id);
void runtime_stop_sprite(Runtime* rt, int sprite_id);

// Clone
void runtime_create_clone(Runtime* rt, int sprite_id);
void runtime_delete_clone(Runtime* rt, int clone_id);

// Ask/Answer
void runtime_submit_answer(Runtime* rt, const std::string& ans);

// Broadcast
void runtime_broadcast(Runtime* rt, const std::string& msg_name, bool wait=false);
void runtime_fire_event(Runtime* rt, const std::string& event_type, const std::string& param="");

// Step
void runtime_step(Runtime* rt);

#endif
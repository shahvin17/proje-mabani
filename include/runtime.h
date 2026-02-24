#ifndef RUNTIME_H
#define RUNTIME_H

#include <SDL2/SDL.h>
#include "core_types.h"
#include <vector>
#include <string>

using namespace std;

enum RuntimeState { RUNTIME_STOPPED, RUNTIME_RUNNING, RUNTIME_PAUSED };

struct ControlFrame {
    int  blockId       = -1;
    int  counter       = 0;
    int  loop_target   = 0;
    int  childHeadId   = -1;
    int  after_loop_id = -1;
    bool is_forever    = false;
    bool is_wait       = false;
    unsigned int wait_end_ms = 0;
};

// forward declares
struct SensingManager;
struct SoundManager;

// ── SpriteRuntime: وضعیت اجرای یک sprite (یا clone) ─────────────────────────
struct SpriteRuntime {
    int          sprite_id      = -1;   // id sprite اصلی
    int          clone_id       = -1;   // -1 = sprite اصلی، >=0 = clone
    int          currentBlockId = -1;
    int          start_block_id = -1;   // when_start یا when_clone_start
    RuntimeState state          = RUNTIME_STOPPED;
    int          watchdogCounter = 0;
    bool         is_clone       = false;
    std::vector<ControlFrame> controlStack;
    int          lastExecutedBlockId = -1;

    // Ask/Answer: آیا این sprite در حال انتظار جواب است
    bool         waiting_for_answer = false;
    std::string  ask_question       = "";
};

// ── Clone: کپی runtime از یک sprite ─────────────────────────────────────────
struct CloneSprite {
    int     clone_id   = -1;    // شناسه منحصر به فرد
    int     sprite_id  = -1;    // sprite اصلی
    Sprite  data;               // کپی داده‌های sprite (x,y,dir,...)
};

// ── Runtime کل پروژه ────────────────────────────────────────────────────────
struct Runtime {
    Project*                   project   = nullptr;
    SensingManager*            sensing   = nullptr;
    SoundManager*              sound_mgr = nullptr;
    std::vector<SpriteRuntime> sprites;          // sprite های اصلی
    std::vector<SpriteRuntime> clone_runtimes;   // runtime هر clone
    std::vector<CloneSprite>   clones;           // داده‌های clone ها
    RuntimeState               state     = RUNTIME_STOPPED;
    int                        watchdogLimit = 100000;

    // Turbo Mode: چند tick در یک فریم
    bool turbo_mode    = false;
    int  turbo_ticks   = 30;    // تعداد tick در هر فریم در حالت turbo

    // Ask/Answer
    bool        ask_active   = false;   // آیا dialog باز است
    std::string ask_buffer   = "";      // متن تایپ‌شده
    std::string answer       = "";      // آخرین جواب ذخیره‌شده
    int         ask_sprite_id = -1;     // کدام sprite سوال پرسیده
    int         ask_resume_block = -1;  // بعد از جواب به کجا برگردیم

    // Clone ID counter
    int next_clone_id = 1;

    // backward compat
    int currentBlockId      = -1;
    int lastExecutedBlockId = -1;
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

// Clone API
void runtime_create_clone(Runtime* rt, int sprite_id);
void runtime_delete_clone(Runtime* rt, int clone_id);

// Ask/Answer API
void runtime_submit_answer(Runtime* rt, const std::string& ans);

#endif // RUNTIME_H
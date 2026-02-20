#include "runtime.h"
#include <iostream>

// تابع کمکی برای پیدا کردن بلوک (بدون تغییر)
static Block* findBlockById(Project* project, int id) {
    if (!project) return nullptr;
    for (auto& b : project->blocks) {
        if (b.id == id) return &b;
    }
    return nullptr;
}

// Forward-declarations برای توابع اجرا
static void executeBlock(Runtime* rt, Block* block);
static void executePrimitive(Runtime* rt, Block* block);
static void executeRepeat(Runtime* rt, Block* block);

// ===============================
// بخش ۱: توابع چرخه حیات Runtime
// ===============================

void runtime_init(Runtime* rt, Project* project) {
    rt->project = project;
    rt->currentBlockId = -1;
    rt->state = RUNTIME_STOPPED;
    rt->watchdogCounter = 0;
    rt->watchdogLimit = 10000; // محدودیت بالا برای اطمینان
    rt->controlStack.clear();
    rt->lastExecutedBlockId = -1;
}

void runtime_start(Runtime* rt) {
    if (!rt) return;
    if (rt->currentBlockId == -1) {
        std::cout << "[Runtime] Error: No start block specified." << std::endl;
        rt->state = RUNTIME_STOPPED;
        return;
    }
    rt->state = RUNTIME_RUNNING;
    rt->watchdogCounter = 0; // ریست کردن نگهبان
    rt->controlStack.clear(); // پاک کردن حالت‌های قدیمی
    std::cout << "Runtime started. firstBlockId=" << rt->currentBlockId << std::endl;
}

void runtime_stop(Runtime* rt) {
    if (!rt) return;
    if (rt->state == RUNTIME_STOPPED) return; // جلوگیری از چاپ چندباره
    rt->state = RUNTIME_STOPPED;
    rt->currentBlockId = -1;
    rt->controlStack.clear();
    std::cout << "Runtime stopped." << std::endl;
}

// ===============================
// بخش ۲: منطق اصلی اجرای بلوک‌ها
// ===============================

// اجراکننده‌ی بلوک‌های ساده (move, turn, say, when_start)
void executePrimitive(Runtime* rt, Block* block) {
    // فقط منطق خود بلوک را اجرا می‌کند
    std::cout << "  -> Primitive action for: " << block->type << std::endl;
    // در آینده، اینجا کد واقعی حرکت یا گفتن قرار می‌گیرد
    
    // مسئولیت رفتن به بلوک بعدی با runtime_tick است
    rt->currentBlockId = block->nextBlockId;
}

// اجراکننده‌ی بلوک repeat
void executeRepeat(Runtime* rt, Block* block) {
    if (block->inputs.size() < 2) {
        rt->currentBlockId = block->nextBlockId; // اگر ورودی ناقص است، از آن بگذر
        return;
    }

    ControlFrame frame;
    frame.blockId = block->id;
    frame.loop_target = block->inputs[0];
    frame.childHeadId = block->inputs[1];
    frame.counter = 0; // اولین تکرار

    // اگر تعداد تکرار صفر یا منفی است، اصلاً وارد حلقه نشو
    if (frame.loop_target <= 0) {
        rt->currentBlockId = block->nextBlockId;
        return;
    }

    std::cout << "  -> Entering Repeat (target: " << frame.loop_target << ")" << std::endl;
    rt->controlStack.push_back(frame); // اضافه کردن حالت حلقه به پشته
    rt->currentBlockId = frame.childHeadId; // برو به اولین بلوک داخل حلقه
}

// تابع اصلی توزیع‌کننده (Dispatcher)
void executeBlock(Runtime* rt, Block* block) {
    std::cout << "Executing block id=" << block->id << ", type=" << block->type << std::endl;
    rt->watchdogCounter++; // افزایش نگهبان برای هر بلوک

    if (block->type == "repeat") {
        executeRepeat(rt, block);
    } else {
        executePrimitive(rt, block);
    }
}


// ===============================
// بخش ۳: تابع اصلی موتور اجرا (Tick)
// ===============================

void runtime_tick(Runtime* rt) {
    if (rt->state != RUNTIME_RUNNING) return;

    // ۱. چک کردن نگهبان
    if (rt->watchdogCounter > rt->watchdogLimit) {
        std::cout << "!!! Watchdog limit reached! Halting execution. !!!" << std::endl;
        runtime_stop(rt);
        return;
    }

    // ۲. منطق اصلی: چه کاری باید انجام شود؟
    // اگر به انتهای یک زنجیره رسیده‌ایم...
    if (rt->currentBlockId == -1) {
        // ...و در یک حلقه هستیم
        if (!rt->controlStack.empty()) {
            ControlFrame& top_frame = rt->controlStack.back();
            top_frame.counter++; // شمارنده تکرار را یکی اضافه کن

            if (top_frame.counter < top_frame.loop_target) {
                // اگر حلقه تمام نشده، دوباره به ابتدای آن برگرد
                rt->currentBlockId = top_frame.childHeadId;
            } else {
                // اگر حلقه تمام شده، از آن خارج شو
                std::cout << "  -> Exiting Repeat Block." << std::endl;
                Block* control_block = findBlockById(rt->project, top_frame.blockId);
                // برو به بلوک بعدی که بعد از repeat قرار دارد
                rt->currentBlockId = control_block ? control_block->nextBlockId : -1;
                rt->controlStack.pop_back(); // حذف حالت حلقه از پشته
            }
        } else {
            // اگر در هیچ حلقه‌ای نیستیم و به انتهای زنجیره رسیدیم، کار تمام است
            runtime_stop(rt);
        }
        return;
    }

    // اگر در وسط یک زنجیره هستیم
    Block* block_to_run = findBlockById(rt->project, rt->currentBlockId);
    if (block_to_run) {
        executeBlock(rt, block_to_run);
    } else {
        std::cout << "[Runtime] Error: Block to run not found (id=" << rt->currentBlockId << ")" << std::endl;
        runtime_stop(rt);
    }
}

// ===============================
// بخش ۴: توابع کمکی (بدون تغییر)
// ===============================
bool runtime_isRunning(const Runtime* rt) { return rt && rt->state == RUNTIME_RUNNING; }
bool runtime_isPaused(const Runtime* rt) { return rt && rt->state == RUNTIME_PAUSED; }
void runtime_pause(Runtime* rt) { /* برای آینده */ }
void runtime_resume(Runtime* rt) { /* برای آینده */ }
void runtime_setWatchdogLimit(Runtime* rt, int limit) { if(rt) rt->watchdogLimit = limit; }

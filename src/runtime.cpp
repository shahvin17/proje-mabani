#include "runtime.h"
#include "vars_ops.h"
#include "motion.h"
#include "looks.h"
#include "pen.h"
#include <iostream>
#include <cmath>

static Block* findBlockById(Project* project, int id){
    if(!project) return nullptr;
    for(auto& b : project->blocks) if(b.id == id) return &b;
    return nullptr;
}

void runtime_init(Runtime* rt, Project* project){
    rt->project = project;
    rt->currentBlockId = -1;
    rt->state = RUNTIME_STOPPED;
    rt->watchdogCounter = 0;
    rt->watchdogLimit = 100000;
    rt->controlStack.clear();
    rt->lastExecutedBlockId = -1;
    rt->returnStack.clear();
    rt->loopCounter.clear();
}

void runtime_start(Runtime* rt){
    if(!rt) return;
    if(rt->currentBlockId == -1){
        std::cout << "[Runtime] No start block\n";
        rt->state = RUNTIME_STOPPED;
        return;
    }
    rt->state = RUNTIME_RUNNING;
    rt->watchdogCounter = 0;
    rt->controlStack.clear();
    std::cout << "Runtime started. firstBlockId=" << rt->currentBlockId << "\n";
}

void runtime_stop(Runtime* rt){
    if(!rt || rt->state == RUNTIME_STOPPED) return;
    rt->state = RUNTIME_STOPPED;
    rt->currentBlockId = -1;
    rt->controlStack.clear();
    std::cout << "Runtime stopped.\n";
}

// ─── اجرای primitive blocks ───────────────────────────────────────────────────
static void executePrimitive(Runtime* rt, Block* block){
    rt->lastExecutedBlockId = block->id;

    Sprite* spr = rt->project->sprites.empty() ? nullptr : &rt->project->sprites[0];
    PenCanvas& canvas = rt->project->pen_canvas;

    // ── Motion ──
    if(block->type == "move" && spr){
        float steps = block->inputs.empty() ? 10.0f : (float)block->inputs[0];
        motion_move(*spr, steps);
        pen_move_to(spr->pen_state, canvas, spr->x, spr->y);
    }
    else if(block->type == "turn_right" && spr){
        motion_turn_right(*spr, block->inputs.empty() ? 15 : (float)block->inputs[0]);
    }
    else if(block->type == "turn_left" && spr){
        motion_turn_left(*spr, block->inputs.empty() ? 15 : (float)block->inputs[0]);
    }
    else if(block->type == "goto_xy" && spr){
        float x = block->inputs.size()>=1 ? (float)block->inputs[0] : 0;
        float y = block->inputs.size()>=2 ? (float)block->inputs[1] : 0;
        motion_goto(*spr, x, y);
        pen_move_to(spr->pen_state, canvas, spr->x, spr->y);
    }
    else if(block->type == "set_x" && spr){
        motion_set_x(*spr, block->inputs.empty() ? 0 : (float)block->inputs[0]);
        pen_move_to(spr->pen_state, canvas, spr->x, spr->y);
    }
    else if(block->type == "set_y" && spr){
        motion_set_y(*spr, block->inputs.empty() ? 0 : (float)block->inputs[0]);
        pen_move_to(spr->pen_state, canvas, spr->x, spr->y);
    }
    else if(block->type == "change_x" && spr){
        motion_change_x(*spr, block->inputs.empty() ? 10 : (float)block->inputs[0]);
        pen_move_to(spr->pen_state, canvas, spr->x, spr->y);
    }
    else if(block->type == "change_y" && spr){
        motion_change_y(*spr, block->inputs.empty() ? 10 : (float)block->inputs[0]);
        pen_move_to(spr->pen_state, canvas, spr->x, spr->y);
    }
    else if(block->type == "bounce" && spr){
        if(spr->x >  220){ spr->x =  220; spr->direction = 180 - spr->direction; }
        if(spr->x < -220){ spr->x = -220; spr->direction = 180 - spr->direction; }
        if(spr->y >  165){ spr->y =  165; spr->direction = -spr->direction; }
        if(spr->y < -165){ spr->y = -165; spr->direction = -spr->direction; }
    }

    // ── Looks ──
    else if(block->type == "say" && spr)     { looks_say(*spr, "Hello!"); }
    else if(block->type == "show" && spr)    { looks_show(*spr); }
    else if(block->type == "hide" && spr)    { looks_hide(*spr); }
    else if(block->type == "set_size" && spr){
        looks_set_size(*spr, block->inputs.empty() ? 100 : (float)block->inputs[0]);
    }
    else if(block->type == "change_size" && spr){
        looks_set_size(*spr, spr->size_percent + (block->inputs.empty() ? 10 : (float)block->inputs[0]));
    }

    // ── Pen ──
    else if(block->type == "pen_down" && spr){
        pen_down(spr->pen_state, spr->x, spr->y);
    }
    else if(block->type == "pen_up" && spr){
        pen_up(spr->pen_state);
    }
    else if(block->type == "pen_erase_all"){
        pen_erase_all(canvas);
    }
    else if(block->type == "pen_stamp" && spr){
        pen_stamp(canvas, spr->x, spr->y,
                  spr->width, spr->height, spr->size_percent, spr->costume_path);
    }
    else if(block->type == "pen_set_color" && spr){
        int r = block->inputs.size()>=1 ? block->inputs[0] : 0;
        int g = block->inputs.size()>=2 ? block->inputs[1] : 0;
        int b = block->inputs.size()>=3 ? block->inputs[2] : 255;
        pen_set_color_rgb(spr->pen_state, r, g, b);
    }
    else if(block->type == "pen_change_color" && spr){
        pen_change_hue(spr->pen_state, block->inputs.empty() ? 10 : (float)block->inputs[0]);
    }
    else if(block->type == "pen_set_size" && spr){
        pen_set_size(spr->pen_state, block->inputs.empty() ? 1 : (float)block->inputs[0]);
    }
    else if(block->type == "pen_change_size" && spr){
        pen_change_size(spr->pen_state, block->inputs.empty() ? 1 : (float)block->inputs[0]);
    }
    else if(block->type == "pen_set_hue" && spr){
        pen_set_hue(spr->pen_state, (block->inputs.empty() ? 100 : (float)block->inputs[0]) * 1.8f);
    }
    else if(block->type == "pen_change_hue" && spr){
        pen_change_hue(spr->pen_state, (block->inputs.empty() ? 10 : (float)block->inputs[0]) * 1.8f);
    }
    else if(block->type == "pen_set_brightness" && spr){
        pen_set_brightness(spr->pen_state, block->inputs.empty() ? 100 : (float)block->inputs[0]);
    }
    else if(block->type == "pen_change_brightness" && spr){
        pen_change_brightness(spr->pen_state, block->inputs.empty() ? 10 : (float)block->inputs[0]);
    }
    else if(block->type == "pen_set_saturation" && spr){
        pen_set_saturation(spr->pen_state, block->inputs.empty() ? 100 : (float)block->inputs[0]);
    }
    else if(block->type == "pen_change_saturation" && spr){
        pen_change_saturation(spr->pen_state, block->inputs.empty() ? 10 : (float)block->inputs[0]);
    }

    // ── Control ──
    else if(block->type == "wait"){
        // wait در این سیستم ساده: فقط یه tick delay نه timer واقعی
        // TODO: timer واقعی با SDL_GetTicks
    }
    else if(block->type == "stop_all"){
        runtime_stop(rt);
        return;
    }
    else if(block->type == "forever"){
        // forever: برگرد به ابتدای زنجیر خودش
        // در سیستم ما، forever = repeat بی‌نهایت
        ControlFrame frame;
        frame.blockId     = block->id;
        frame.loop_target = 999999;   // بی‌نهایت
        frame.childHeadId = block->nextBlockId;
        frame.counter     = 0;
        frame.is_forever  = true;
        if(frame.childHeadId != -1){
            rt->controlStack.push_back(frame);
            rt->currentBlockId = frame.childHeadId;
            return;
        }
    }

    // ── Operators ──
    else if(block->type.size() >= 3 && block->type.substr(0,3) == "op_"){
        if(block->inputs.size() >= 2){
            Value a  = value_number(block->inputs[0]);
            Value bv = value_number(block->inputs[1]);
            bool err = false; Value res = value_number(0);
            const std::string& op = block->type;
            if(op=="op_add") res=op_add(a,bv,err);
            else if(op=="op_sub") res=op_sub(a,bv,err);
            else if(op=="op_mul") res=op_mul(a,bv,err);
            else if(op=="op_div") res=op_div(a,bv,err);
            else if(op=="op_mod") res=op_modulo(a,bv,err);
            else if(op=="op_gt")  res=op_greater_than(a,bv,err);
            else if(op=="op_lt")  res=op_less_than(a,bv,err);
            else if(op=="op_eq")  res=op_equals(a,bv);
            else if(op=="op_and") res=op_and(a,bv);
            else if(op=="op_or")  res=op_or(a,bv);
            else if(op=="op_xor") res=op_xor(a,bv);
            if(!err) std::cout << "  op=" << op << " result=" << res.num << "\n";
        } else if(block->inputs.size() >= 1){
            Value a = value_number(block->inputs[0]);
            bool err = false; Value res = value_number(0);
            const std::string& op = block->type;
            if(op=="op_not")   res=op_not(a);
            else if(op=="op_abs")   res=op_abs(a);
            else if(op=="op_floor") res=op_floor(a);
            else if(op=="op_ceil")  res=op_ceil(a);
            else if(op=="op_sqrt")  res=op_sqrt(a,err);
            else if(op=="op_sin")   res=op_sin(a);
            else if(op=="op_cos")   res=op_cos(a);
            else if(op=="op_round") res=value_number(std::round(a.num));
            if(!err) std::cout << "  op=" << op << " result=" << res.num << "\n";
        }
    }

    else if(block->type == "end_repeat"){
        // پایان یک دور loop
        if(!rt->controlStack.empty()){
            ControlFrame& f = rt->controlStack.back();
            f.counter++;
            if(f.is_forever || f.counter < f.loop_target){
                // دور بعدی
                rt->currentBlockId = f.childHeadId;
                return;
            } else {
                // loop تموم شد - برو به بعد از end_repeat
                rt->controlStack.pop_back();
                rt->currentBlockId = block->nextBlockId;
                std::cout << "  -> Repeat done" << std::endl;
                return;
            }
        }
        rt->currentBlockId = block->nextBlockId;
        return;
    }
    else if(block->type == "when_start"){ /* no-op */ }

    rt->currentBlockId = block->nextBlockId;
}

// ─── repeat: بازنویسی کامل ───────────────────────────────────────────────────
// منطق: repeat count بلوک‌های بعد از خودش رو count بار اجرا می‌کنه
// ساختار زنجیر: repeat → A → B → C → -1
// repeat باید A → B → C رو count بار اجرا کنه، بعد به next of repeat برره
//
// چون در سیستم drag&drop ما همه بلوک‌ها در یک زنجیر nextBlockId هستند،
// repeat باید ابتدای loop (بلوک بعدی خودش در زنجیر) را N بار بزند
// و "پایان loop" را با نگه‌داشتن آخرین بلوک قبل از خروج تشخیص دهد.
// 
// SIMPLE APPROACH: repeat block->nextBlockId را N بار اجرا می‌کند
// و بعد از هر دور، به همان ابتدا برمی‌گردد تا counter تموم شه
static void executeRepeat(Runtime* rt, Block* block){
    int count = block->inputs.empty() ? 10 : block->inputs[0];
    if(count <= 0){
        rt->currentBlockId = block->nextBlockId;
        return;
    }

    // پیدا کردن آخرین بلوک در زنجیر repeat (بلوکی که nextBlockId=-1 یا خارج از loop)
    // در سیستم ما، همه بلوک‌های بعد از repeat داخل loop هستن
    // پس loop از block->nextBlockId شروع می‌شه و تا -1 ادامه داره
    int head_id = block->nextBlockId;
    if(head_id == -1){
        // هیچ بلوکی داخل loop نیست
        rt->currentBlockId = -1;
        return;
    }

    ControlFrame frame;
    frame.blockId     = block->id;
    frame.loop_target = count;
    frame.childHeadId = head_id;
    frame.counter     = 0;       // وقتی به end_repeat رسید، counter++ می‌شه
    frame.is_forever  = false;
    frame.after_loop_id = -1;    // end_repeat->nextBlockId را runtime تنظیم می‌کند

    rt->controlStack.push_back(frame);
    rt->currentBlockId = head_id;
    std::cout << "  -> Repeat x" << count << " head=" << head_id << "\n";
}

static void executeBlock(Runtime* rt, Block* block){
    std::cout << "Exec id=" << block->id << " type=" << block->type << "\n";
    rt->watchdogCounter++;

    if(block->type == "repeat")       executeRepeat(rt, block);
    else if(block->type == "forever") executePrimitive(rt, block);
    else                              executePrimitive(rt, block);
}

void runtime_tick(Runtime* rt){
    if(rt->state != RUNTIME_RUNNING) return;

    if(rt->watchdogCounter > rt->watchdogLimit){
        std::cout << "!!! Watchdog! Halting !!!\n";
        runtime_stop(rt);
        return;
    }

    if(rt->currentBlockId == -1){
        if(!rt->controlStack.empty()){
            // forever loop که end_repeat نداره: برگرد به head
            ControlFrame& f = rt->controlStack.back();
            if(f.is_forever){
                rt->currentBlockId = f.childHeadId;
            } else {
                // repeat بدون end_repeat: loop تموم شد
                rt->controlStack.pop_back();
                rt->currentBlockId = -1;
            }
        } else {
            runtime_stop(rt);
        }
        return;
    }

    Block* b = findBlockById(rt->project, rt->currentBlockId);
    if(b) executeBlock(rt, b);
    else{
        std::cout << "[Runtime] Block not found id=" << rt->currentBlockId << "\n";
        runtime_stop(rt);
    }
}

bool runtime_isRunning(const Runtime* rt){ return rt && rt->state == RUNTIME_RUNNING; }
bool runtime_isPaused(const Runtime* rt) { return rt && rt->state == RUNTIME_PAUSED; }
void runtime_pause(Runtime* rt)  { if(rt && rt->state == RUNTIME_RUNNING) rt->state = RUNTIME_PAUSED; }
void runtime_resume(Runtime* rt) { if(rt && rt->state == RUNTIME_PAUSED)  rt->state = RUNTIME_RUNNING; }
void runtime_setWatchdogLimit(Runtime* rt, int limit){ if(rt) rt->watchdogLimit = limit; }
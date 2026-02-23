#include "runtime.h"
#include "vars_ops.h"
#include "motion.h"
#include "looks.h"
#include "pen.h"
#include "sensing.h"
#include "sound.h"
#include <iostream>
#include <cmath>

// ─── کمک: پیدا کردن بلوک با id ──────────────────────────────────────────────
static Block* findBlock(Project* p, int id) {
    if (!p || id < 0) return nullptr;
    for (auto& b : p->blocks) if (b.id == id) return &b;
    return nullptr;
}

// ─── پیدا کردن sprite فعال در SpriteRuntime ─────────────────────────────────
static Sprite* findSprite(Project* p, int sprite_id) {
    if (!p) return nullptr;
    for (auto& s : p->sprites) if (s.id == sprite_id) return &s;
    return nullptr;
}

// ─── اجرای یک بلوک برای یک SpriteRuntime ────────────────────────────────────
static void execBlock(Runtime* rt, SpriteRuntime& sr, Block* block) {
    sr.lastExecutedBlockId = block->id;
    Sprite* spr    = findSprite(rt->project, sr.sprite_id);
    PenCanvas& cvs = rt->project->pen_canvas;
    const std::string& t = block->type;

    // ── Motion ──
    if (t=="move" && spr) {
        float s = block->inputs.empty() ? 10.f : (float)block->inputs[0];
        motion_move(*spr, s);
        pen_move_to(spr->pen_state, cvs, spr->x, spr->y);
    }
    else if (t=="turn_right" && spr)
        motion_turn_right(*spr, block->inputs.empty()?15:(float)block->inputs[0]);
    else if (t=="turn_left" && spr)
        motion_turn_left(*spr, block->inputs.empty()?15:(float)block->inputs[0]);
    else if (t=="goto_xy" && spr) {
        float x=block->inputs.size()>=1?(float)block->inputs[0]:0;
        float y=block->inputs.size()>=2?(float)block->inputs[1]:0;
        motion_goto(*spr,x,y);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if (t=="set_x" && spr) {
        motion_set_x(*spr,block->inputs.empty()?0:(float)block->inputs[0]);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if (t=="set_y" && spr) {
        motion_set_y(*spr,block->inputs.empty()?0:(float)block->inputs[0]);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if (t=="change_x" && spr) {
        motion_change_x(*spr,block->inputs.empty()?10:(float)block->inputs[0]);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if (t=="change_y" && spr) {
        motion_change_y(*spr,block->inputs.empty()?10:(float)block->inputs[0]);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if (t=="bounce" && spr) {
        if(spr->x> 220){spr->x= 220;spr->direction=180-spr->direction;}
        if(spr->x<-220){spr->x=-220;spr->direction=180-spr->direction;}
        if(spr->y> 165){spr->y= 165;spr->direction=-spr->direction;}
        if(spr->y<-165){spr->y=-165;spr->direction=-spr->direction;}
    }

    // ── Looks ──
    else if (t=="say"        && spr) looks_say(*spr,"Hello!");
    else if (t=="show"       && spr) looks_show(*spr);
    else if (t=="hide"       && spr) looks_hide(*spr);
    else if (t=="set_size"   && spr)
        looks_set_size(*spr,block->inputs.empty()?100:(float)block->inputs[0]);
    else if (t=="change_size"&& spr)
        looks_set_size(*spr,spr->size_percent+(block->inputs.empty()?10:(float)block->inputs[0]));

    // ── Costumes ──
    else if (t=="next_costume"&& spr && !spr->costumes.empty()){
        spr->costume_index=(spr->costume_index+1)%(int)spr->costumes.size();
        spr->costume_path=spr->costumes[spr->costume_index].path;
    }
    else if (t=="prev_costume"&& spr && !spr->costumes.empty()){
        int n=(int)spr->costumes.size();
        spr->costume_index=(spr->costume_index-1+n)%n;
        spr->costume_path=spr->costumes[spr->costume_index].path;
    }
    else if (t=="set_costume" && spr){
        int i=block->inputs.empty()?0:block->inputs[0];
        if(i>=0&&i<(int)spr->costumes.size()){
            spr->costume_index=i; spr->costume_path=spr->costumes[i].path;
        }
    }

    // ── Backdrops ──
    else if (t=="next_backdrop"){
        auto& bds=rt->project->backdrops;
        if(!bds.empty())
            rt->project->active_backdrop_idx=(rt->project->active_backdrop_idx+1)%(int)bds.size();
    }
    else if (t=="prev_backdrop"){
        auto& bds=rt->project->backdrops;
        if(!bds.empty()){
            int n=(int)bds.size();
            rt->project->active_backdrop_idx=(rt->project->active_backdrop_idx-1+n)%n;
        }
    }
    else if (t=="set_backdrop"){
        int i=block->inputs.empty()?0:block->inputs[0];
        if(i>=0&&i<(int)rt->project->backdrops.size())
            rt->project->active_backdrop_idx=i;
    }

    // ── Pen ──
    else if (t=="pen_down"   && spr) pen_down(spr->pen_state,spr->x,spr->y);
    else if (t=="pen_up"     && spr) pen_up(spr->pen_state);
    else if (t=="pen_erase_all")     pen_erase_all(cvs);
    else if (t=="pen_stamp"  && spr)
        pen_stamp(cvs,spr->x,spr->y,spr->width,spr->height,spr->size_percent,spr->costume_path);
    else if (t=="pen_set_color"&&spr){
        int r=block->inputs.size()>=1?block->inputs[0]:0;
        int g=block->inputs.size()>=2?block->inputs[1]:0;
        int b=block->inputs.size()>=3?block->inputs[2]:255;
        pen_set_color_rgb(spr->pen_state,r,g,b);
    }
    else if (t=="pen_change_color"&&spr)
        pen_change_hue(spr->pen_state,block->inputs.empty()?10:(float)block->inputs[0]);
    else if (t=="pen_set_size"&&spr)
        pen_set_size(spr->pen_state,block->inputs.empty()?1:(float)block->inputs[0]);
    else if (t=="pen_change_size"&&spr)
        pen_change_size(spr->pen_state,block->inputs.empty()?1:(float)block->inputs[0]);
    else if (t=="pen_set_hue"&&spr)
        pen_set_hue(spr->pen_state,(block->inputs.empty()?100:(float)block->inputs[0])*1.8f);
    else if (t=="pen_change_hue"&&spr)
        pen_change_hue(spr->pen_state,(block->inputs.empty()?10:(float)block->inputs[0])*1.8f);
    else if (t=="pen_set_brightness"&&spr)
        pen_set_brightness(spr->pen_state,block->inputs.empty()?100:(float)block->inputs[0]);
    else if (t=="pen_change_brightness"&&spr)
        pen_change_brightness(spr->pen_state,block->inputs.empty()?10:(float)block->inputs[0]);
    else if (t=="pen_set_saturation"&&spr)
        pen_set_saturation(spr->pen_state,block->inputs.empty()?100:(float)block->inputs[0]);
    else if (t=="pen_change_saturation"&&spr)
        pen_change_saturation(spr->pen_state,block->inputs.empty()?10:(float)block->inputs[0]);

    // ── Sound ──
    else if (t=="play_sound" || t=="play_sound_wait"){
        if(rt->sound_mgr){
            int idx = block->inputs.empty() ? 0 : block->inputs[0];
            sound_play_index(*rt->sound_mgr, idx);
        }
    }
    else if (t=="stop_sounds"){
        if(rt->sound_mgr) sound_stop_all(*rt->sound_mgr);
    }
    else if (t=="set_volume"){
        if(rt->sound_mgr){
            float v = block->inputs.empty() ? 100.f : (float)block->inputs[0];
            sound_set_volume(*rt->sound_mgr, v/100.f);
        }
    }
    else if (t=="change_volume"){
        if(rt->sound_mgr){
            float d = block->inputs.empty() ? -10.f : (float)block->inputs[0];
            float cur = rt->sound_mgr->volume * 100.f;
            sound_set_volume(*rt->sound_mgr, (cur+d)/100.f);
        }
    }

    // ── Variables ──
    else if (t=="set_var" && block->inputs.size()>=2){
        int vi=(int)block->inputs[0]; float val=(float)block->inputs[1];
        if(vi>=0&&vi<(int)rt->project->variables.size())
            rt->project->variables[vi].value=val;
    }
    else if (t=="change_var" && block->inputs.size()>=2){
        int vi=(int)block->inputs[0]; float d=(float)block->inputs[1];
        if(vi>=0&&vi<(int)rt->project->variables.size())
            rt->project->variables[vi].value+=d;
    }
    else if (t=="show_var" && !block->inputs.empty()){
        int vi=block->inputs[0];
        if(vi>=0&&vi<(int)rt->project->variables.size())
            rt->project->variables[vi].visible=true;
    }
    else if (t=="hide_var" && !block->inputs.empty()){
        int vi=block->inputs[0];
        if(vi>=0&&vi<(int)rt->project->variables.size())
            rt->project->variables[vi].visible=false;
    }

    // ── Sensing ──
    else if (t=="touching_mouse"&&spr&&rt->sensing){
        bool r=sense_touching_mouse(*spr,*rt->sensing);
        std::cout<<"touching_mouse="<<r<<"\n";
    }
    else if (t=="key_pressed"&&rt->sensing&&!block->inputs.empty())
        sense_is_key_pressed(*rt->sensing,block->inputs[0]);
    else if (t=="mouse_x"&&spr&&rt->sensing){
        spr->x=sense_get_mouse_x(*rt->sensing);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if (t=="mouse_y"&&spr&&rt->sensing){
        spr->y=sense_get_mouse_y(*rt->sensing);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if (t=="reset_timer"&&rt->sensing) sense_reset_timer(*rt->sensing);
    else if (t=="set_drag_mode"&&spr)
        sense_set_drag_mode(*spr,block->inputs.empty()?true:(block->inputs[0]!=0));

    // ── Control ──
    else if (t=="wait"){
        float secs=block->inputs.empty()?1.f:(float)block->inputs[0];
        ControlFrame f;
        f.blockId=block->id; f.loop_target=0; f.childHeadId=-1;
        f.counter=0; f.is_forever=false; f.is_wait=true;
        f.wait_end_ms=SDL_GetTicks()+(unsigned int)(secs*1000.f);
        f.after_loop_id=block->nextBlockId;
        sr.controlStack.push_back(f);
        return;  // خروج بدون پیشروی
    }
    else if (t=="stop_all")  { runtime_stop(rt); return; }
    else if (t=="forever"){
        ControlFrame f;
        f.blockId=block->id; f.loop_target=999999;
        f.childHeadId=block->nextBlockId; f.counter=0;
        f.is_forever=true; f.is_wait=false;
        if(f.childHeadId!=-1){ sr.controlStack.push_back(f); sr.currentBlockId=f.childHeadId; return; }
    }
    else if (t=="end_repeat"){
        if(!sr.controlStack.empty()){
            ControlFrame& cf=sr.controlStack.back();
            cf.counter++;
            if(cf.is_forever||cf.counter<cf.loop_target){
                sr.currentBlockId=cf.childHeadId; return;
            } else {
                int nxt=block->nextBlockId; sr.controlStack.pop_back(); sr.currentBlockId=nxt; return;
            }
        }
        sr.currentBlockId=block->nextBlockId; return;
    }
    else if (t=="if_then"){
        int cond=block->inputs.empty()?0:block->inputs[0];
        int body=block->inputs.size()>=2?block->inputs[1]:-1;
        if(cond&&body!=-1){ sr.currentBlockId=body; return; }
    }
    else if (t=="if_else"){
        int cond=block->inputs.empty()?0:block->inputs[0];
        int then_=block->inputs.size()>=2?block->inputs[1]:-1;
        int else_=block->inputs.size()>=3?block->inputs[2]:-1;
        int body=(cond?then_:else_);
        if(body!=-1){ sr.currentBlockId=body; return; }
    }

    // ── Operators ──
    else if (t.size()>=3 && t.substr(0,3)=="op_"){
        if(block->inputs.size()>=2){
            Value a=value_number(block->inputs[0]),bv=value_number(block->inputs[1]);
            bool err=false; Value res=value_number(0);
            if(t=="op_add")res=op_add(a,bv,err);
            else if(t=="op_sub")res=op_sub(a,bv,err);
            else if(t=="op_mul")res=op_mul(a,bv,err);
            else if(t=="op_div")res=op_div(a,bv,err);
            else if(t=="op_mod")res=op_modulo(a,bv,err);
            else if(t=="op_gt") res=op_greater_than(a,bv,err);
            else if(t=="op_lt") res=op_less_than(a,bv,err);
            else if(t=="op_eq") res=op_equals(a,bv);
            else if(t=="op_and")res=op_and(a,bv);
            else if(t=="op_or") res=op_or(a,bv);
            else if(t=="op_xor")res=op_xor(a,bv);
        } else if(block->inputs.size()>=1){
            Value a=value_number(block->inputs[0]); bool err=false; Value res=value_number(0);
            if(t=="op_not")res=op_not(a);
            else if(t=="op_abs")res=op_abs(a);
            else if(t=="op_floor")res=op_floor(a);
            else if(t=="op_ceil")res=op_ceil(a);
            else if(t=="op_sqrt")res=op_sqrt(a,err);
            else if(t=="op_sin")res=op_sin(a);
            else if(t=="op_cos")res=op_cos(a);
            else if(t=="op_round")res=value_number(std::round(a.num));
        }
    }
    else if (t=="when_start") { /* no-op */ }

    sr.currentBlockId = block->nextBlockId;
}

// ─── repeat block ─────────────────────────────────────────────────────────────
static void execRepeat(SpriteRuntime& sr, Block* block) {
    int count = block->inputs.empty() ? 10 : block->inputs[0];
    int head  = block->nextBlockId;
    if (count <= 0 || head == -1) { sr.currentBlockId = -1; return; }
    ControlFrame f;
    f.blockId=block->id; f.loop_target=count; f.childHeadId=head;
    f.counter=0; f.is_forever=false; f.is_wait=false; f.after_loop_id=-1;
    sr.controlStack.push_back(f);
    sr.currentBlockId = head;
}

// ─── tick یک SpriteRuntime ────────────────────────────────────────────────────
static void tickSprite(Runtime* rt, SpriteRuntime& sr) {
    if (sr.state != RUNTIME_RUNNING) return;

    // بررسی wait
    if (!sr.controlStack.empty() && sr.controlStack.back().is_wait) {
        ControlFrame& wf = sr.controlStack.back();
        if (SDL_GetTicks() >= wf.wait_end_ms) {
            int nxt = wf.after_loop_id;
            sr.controlStack.pop_back();
            sr.currentBlockId = nxt;
        }
        return;
    }

    if (sr.currentBlockId == -1) {
        if (!sr.controlStack.empty()) {
            ControlFrame& f = sr.controlStack.back();
            if (f.is_forever) { sr.currentBlockId = f.childHeadId; return; }
            sr.controlStack.pop_back(); sr.currentBlockId = -1;
        } else {
            sr.state = RUNTIME_STOPPED;
        }
        return;
    }

    Block* b = findBlock(rt->project, sr.currentBlockId);
    if (!b) { sr.state = RUNTIME_STOPPED; return; }

    if (b->type == "repeat") execRepeat(sr, b);
    else                     execBlock(rt, sr, b);
}

// ═════════════════════════════════════════════════════════════════════════════
// PUBLIC API
// ═════════════════════════════════════════════════════════════════════════════

void runtime_init(Runtime* rt, Project* project) {
    rt->project = project;
    rt->state   = RUNTIME_STOPPED;
    rt->sprites.clear();
    rt->watchdogLimit = 100000;
    rt->currentBlockId = -1;
    // ساخت SpriteRuntime برای هر sprite
    for (auto& s : project->sprites) {
        SpriteRuntime sr;
        sr.sprite_id     = s.id;
        sr.state         = RUNTIME_STOPPED;
        sr.start_block_id = s.when_start_id;  // ← مستقیم از sprite

        // اگه when_start_id تنظیم نشده، جستجوی fallback
        if (sr.start_block_id == -1) {
            for (auto& b : project->blocks)
                if (b.type == "when_start" &&
                    (b.sprite_owner == s.id || b.sprite_owner == -1)) {
                    sr.start_block_id = b.id;
                    break;
                }
        }
        rt->sprites.push_back(sr);
    }
}

void runtime_start(Runtime* rt) {
    if (!rt) return;
    rt->state = RUNTIME_RUNNING;
    rt->controlStack.clear();
    // شروع همه sprite ها
    for (auto& sr : rt->sprites) {
        if (sr.start_block_id == -1) continue;
        Block* sb = findBlock(rt->project, sr.start_block_id);
        if (!sb) continue;
        sr.state          = RUNTIME_RUNNING;
        sr.currentBlockId = sb->nextBlockId;  // بلوک بعد از when_start
        sr.controlStack.clear();
        sr.watchdogCounter = 0;
        std::cout << "[Runtime] Sprite " << sr.sprite_id
                  << " started at block " << sr.currentBlockId << "\n";
    }
    // backward compat
    if (!rt->sprites.empty() && rt->sprites[0].state == RUNTIME_RUNNING)
        rt->currentBlockId = rt->sprites[0].currentBlockId;
}

void runtime_stop(Runtime* rt) {
    if (!rt) return;
    rt->state = RUNTIME_STOPPED;
    for (auto& sr : rt->sprites) {
        sr.state = RUNTIME_STOPPED;
        sr.controlStack.clear();
        sr.currentBlockId = -1;
    }
    std::cout << "[Runtime] All stopped.\n";
}

void runtime_start_sprite(Runtime* rt, int sprite_id) {
    for (auto& sr : rt->sprites) {
        if (sr.sprite_id != sprite_id) continue;
        if (sr.start_block_id == -1) return;
        Block* sb = findBlock(rt->project, sr.start_block_id);
        if (!sb) return;
        sr.state = RUNTIME_RUNNING;
        sr.currentBlockId = sb->nextBlockId;
        sr.controlStack.clear();
        return;
    }
}

void runtime_stop_sprite(Runtime* rt, int sprite_id) {
    for (auto& sr : rt->sprites)
        if (sr.sprite_id == sprite_id) { sr.state = RUNTIME_STOPPED; return; }
}

void runtime_tick(Runtime* rt) {
    if (!rt || rt->state == RUNTIME_STOPPED) return;
    if (rt->state == RUNTIME_PAUSED) return;
    // tick هر sprite
    for (auto& sr : rt->sprites)
        tickSprite(rt, sr);
    // backward compat
    if (!rt->sprites.empty())
        rt->currentBlockId = rt->sprites[0].currentBlockId;
}

bool runtime_isRunning(const Runtime* rt) {
    if (!rt || rt->state == RUNTIME_STOPPED) return false;
    if (rt->state == RUNTIME_PAUSED) return false;
    for (auto& sr : rt->sprites)
        if (sr.state == RUNTIME_RUNNING) return true;
    return false;
}

bool runtime_isPaused(const Runtime* rt) {
    return rt && rt->state == RUNTIME_PAUSED;
}

void runtime_pause(Runtime* rt) {
    if (rt && runtime_isRunning(rt)) rt->state = RUNTIME_PAUSED;
}

void runtime_resume(Runtime* rt) {
    if (rt && rt->state == RUNTIME_PAUSED) rt->state = RUNTIME_RUNNING;
}

void runtime_setWatchdogLimit(Runtime* rt, int limit) {
    if (rt) rt->watchdogLimit = limit;
}
#include "runtime.h"
#include "vars_ops.h"
#include "motion.h"
#include "looks.h"
#include "pen.h"
#include "sensing.h"
#include "sound.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// ─── helpers ─────────────────────────────────────────────────────────────────
static Block* findBlock(Project* p, int id){
    if(!p||id<0) return nullptr;
    for(auto& b:p->blocks) if(b.id==id) return &b;
    return nullptr;
}
static Sprite* findSprite(Project* p, int id){
    if(!p) return nullptr;
    for(auto& s:p->sprites) if(s.id==id) return &s;
    return nullptr;
}
static Sprite* resolveSprite(Runtime* rt, SpriteRuntime& sr){
    if(sr.is_clone){
        for(auto& c:rt->clones) if(c.clone_id==sr.clone_id) return &c.data;
        return nullptr;
    }
    return findSprite(rt->project, sr.sprite_id);
}

// ─── execBlock ───────────────────────────────────────────────────────────────
static void execBlock(Runtime* rt, SpriteRuntime& sr, Block* block){
    if(block->disabled){ sr.currentBlockId=block->nextBlockId; return; }
    sr.lastExecutedBlockId = block->id;
    rt->step_highlight_block = block->id;
    Sprite* spr = resolveSprite(rt,sr);
    PenCanvas& cvs = rt->project->pen_canvas;
    const std::string& t = block->type;

    // ── Motion ──
    if(t=="move"&&spr){
        float s=block->inputs.empty()?10:(float)block->inputs[0];
        motion_move(*spr,s); pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if(t=="turn_right"&&spr) motion_turn_right(*spr,block->inputs.empty()?15:(float)block->inputs[0]);
    else if(t=="turn_left"&&spr)  motion_turn_left(*spr,block->inputs.empty()?15:(float)block->inputs[0]);
    else if(t=="goto_xy"&&spr){
        float x=block->inputs.size()>=1?(float)block->inputs[0]:0;
        float y=block->inputs.size()>=2?(float)block->inputs[1]:0;
        motion_goto(*spr,x,y); pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }
    else if(t=="set_x"&&spr){ motion_set_x(*spr,block->inputs.empty()?0:(float)block->inputs[0]); pen_move_to(spr->pen_state,cvs,spr->x,spr->y); }
    else if(t=="set_y"&&spr){ motion_set_y(*spr,block->inputs.empty()?0:(float)block->inputs[0]); pen_move_to(spr->pen_state,cvs,spr->x,spr->y); }
    else if(t=="change_x"&&spr){ motion_change_x(*spr,block->inputs.empty()?10:(float)block->inputs[0]); pen_move_to(spr->pen_state,cvs,spr->x,spr->y); }
    else if(t=="change_y"&&spr){ motion_change_y(*spr,block->inputs.empty()?10:(float)block->inputs[0]); pen_move_to(spr->pen_state,cvs,spr->x,spr->y); }
    else if(t=="bounce"&&spr){
        if(spr->x> 220){spr->x= 220;spr->direction=180-spr->direction;}
        if(spr->x<-220){spr->x=-220;spr->direction=180-spr->direction;}
        if(spr->y> 165){spr->y= 165;spr->direction=-spr->direction;}
        if(spr->y<-165){spr->y=-165;spr->direction=-spr->direction;}
    }
    else if(t=="goto_mouse"&&spr&&rt->sensing){
        spr->x=sense_get_mouse_x(*rt->sensing);
        spr->y=sense_get_mouse_y(*rt->sensing);
        pen_move_to(spr->pen_state,cvs,spr->x,spr->y);
    }

    // ── Looks ──
    else if(t=="say"&&spr){ spr->say_message="Hello!"; spr->say_timer=2.f; }
    else if(t=="say_for_secs"&&spr){
        spr->say_message="Hello!";
        spr->say_timer=block->inputs.size()>=2?(float)block->inputs[1]:2.f;
    }
    else if(t=="think"&&spr){ spr->say_message="Hmm..."; spr->say_timer=2.f; }
    else if(t=="show"&&spr)  looks_show(*spr);
    else if(t=="hide"&&spr)  looks_hide(*spr);
    else if(t=="set_size"&&spr)    looks_set_size(*spr,block->inputs.empty()?100:(float)block->inputs[0]);
    else if(t=="change_size"&&spr) looks_set_size(*spr,spr->size_percent+(block->inputs.empty()?10:(float)block->inputs[0]));
    else if(t=="next_costume"&&spr&&!spr->costumes.empty()){
        spr->costume_index=(spr->costume_index+1)%(int)spr->costumes.size();
        spr->costume_path=spr->costumes[spr->costume_index].path;
    }
    else if(t=="prev_costume"&&spr&&!spr->costumes.empty()){
        int n=(int)spr->costumes.size();
        spr->costume_index=(spr->costume_index-1+n)%n;
        spr->costume_path=spr->costumes[spr->costume_index].path;
    }
    else if(t=="set_costume"&&spr){
        int i=block->inputs.empty()?0:block->inputs[0];
        if(i>=0&&i<(int)spr->costumes.size()){ spr->costume_index=i; spr->costume_path=spr->costumes[i].path; }
    }
    // Backdrops
    else if(t=="next_backdrop"){
        auto& bds=rt->project->backdrops;
        if(!bds.empty()) rt->project->active_backdrop_idx=(rt->project->active_backdrop_idx+1)%(int)bds.size();
    }
    else if(t=="prev_backdrop"){
        auto& bds=rt->project->backdrops; int n=(int)bds.size();
        if(n) rt->project->active_backdrop_idx=(rt->project->active_backdrop_idx-1+n)%n;
    }
    else if(t=="set_backdrop"){
        int i=block->inputs.empty()?0:block->inputs[0];
        if(i>=0&&i<(int)rt->project->backdrops.size()) rt->project->active_backdrop_idx=i;
    }

    // ── Pen ──
    else if(t=="pen_down"&&spr) pen_down(spr->pen_state,spr->x,spr->y);
    else if(t=="pen_up"&&spr)   pen_up(spr->pen_state);
    else if(t=="pen_erase_all") pen_erase_all(cvs);
    else if(t=="pen_stamp"&&spr) pen_stamp(cvs,spr->x,spr->y,spr->width,spr->height,spr->size_percent,spr->costume_path);
    else if(t=="pen_set_color"&&spr){
        int r=block->inputs.size()>=1?block->inputs[0]:0;
        int g=block->inputs.size()>=2?block->inputs[1]:0;
        int b=block->inputs.size()>=3?block->inputs[2]:255;
        pen_set_color_rgb(spr->pen_state,r,g,b);
    }
    else if(t=="pen_change_color"&&spr)    pen_change_hue(spr->pen_state,block->inputs.empty()?10:(float)block->inputs[0]);
    else if(t=="pen_set_size"&&spr)         pen_set_size(spr->pen_state,block->inputs.empty()?1:(float)block->inputs[0]);
    else if(t=="pen_change_size"&&spr)      pen_change_size(spr->pen_state,block->inputs.empty()?1:(float)block->inputs[0]);
    else if(t=="pen_set_hue"&&spr)          pen_set_hue(spr->pen_state,(block->inputs.empty()?100:(float)block->inputs[0])*1.8f);
    else if(t=="pen_change_hue"&&spr)       pen_change_hue(spr->pen_state,(block->inputs.empty()?10:(float)block->inputs[0])*1.8f);
    else if(t=="pen_set_brightness"&&spr)   pen_set_brightness(spr->pen_state,block->inputs.empty()?100:(float)block->inputs[0]);
    else if(t=="pen_change_brightness"&&spr)pen_change_brightness(spr->pen_state,block->inputs.empty()?10:(float)block->inputs[0]);
    else if(t=="pen_set_saturation"&&spr)   pen_set_saturation(spr->pen_state,block->inputs.empty()?100:(float)block->inputs[0]);
    else if(t=="pen_change_saturation"&&spr)pen_change_saturation(spr->pen_state,block->inputs.empty()?10:(float)block->inputs[0]);

    // ── Sound ──
    else if((t=="play_sound"||t=="play_sound_wait")&&rt->sound_mgr)
        sound_play_index(*rt->sound_mgr,block->inputs.empty()?0:block->inputs[0]);
    else if(t=="stop_sounds"&&rt->sound_mgr) sound_stop_all(*rt->sound_mgr);
    else if(t=="set_volume"&&rt->sound_mgr)  sound_set_volume(*rt->sound_mgr,(block->inputs.empty()?100:(float)block->inputs[0])/100.f);
    else if(t=="change_volume"&&rt->sound_mgr){
        float d=block->inputs.empty()?-10:(float)block->inputs[0];
        sound_set_volume(*rt->sound_mgr,rt->sound_mgr->volume+d/100.f);
    }

    // ── Variables ──
    else if(t=="set_var"&&block->inputs.size()>=2){
        int vi=block->inputs[0]; float val=(float)block->inputs[1];
        if(vi>=0&&vi<(int)rt->project->variables.size()) rt->project->variables[vi].value=val;
    }
    else if(t=="change_var"&&block->inputs.size()>=2){
        int vi=block->inputs[0]; float d=(float)block->inputs[1];
        if(vi>=0&&vi<(int)rt->project->variables.size()) rt->project->variables[vi].value+=d;
    }
    else if(t=="show_var"&&!block->inputs.empty()){
        int vi=block->inputs[0];
        if(vi>=0&&vi<(int)rt->project->variables.size()) rt->project->variables[vi].visible=true;
    }
    else if(t=="hide_var"&&!block->inputs.empty()){
        int vi=block->inputs[0];
        if(vi>=0&&vi<(int)rt->project->variables.size()) rt->project->variables[vi].visible=false;
    }

    // ── List Variables ───────────────────────────────────────────────────────
    else if(t=="list_add"&&block->inputs.size()>=2){
        // inputs[0]=list index, inputs[1]=value (int encoded)
        int li=block->inputs[0];
        if(li>=0&&li<(int)rt->project->lists.size())
            rt->project->lists[li].items.push_back(std::to_string(block->inputs[1]));
    }
    else if(t=="list_delete"&&block->inputs.size()>=2){
        int li=block->inputs[0]; int idx=block->inputs[1]-1;
        auto& lst=rt->project->lists;
        if(li>=0&&li<(int)lst.size()&&idx>=0&&idx<(int)lst[li].items.size())
            lst[li].items.erase(lst[li].items.begin()+idx);
    }
    else if(t=="list_delete_all"&&!block->inputs.empty()){
        int li=block->inputs[0];
        if(li>=0&&li<(int)rt->project->lists.size()) rt->project->lists[li].items.clear();
    }
    else if(t=="list_insert"&&block->inputs.size()>=3){
        int li=block->inputs[0]; int idx=block->inputs[1]-1; int val=block->inputs[2];
        auto& lst=rt->project->lists;
        if(li>=0&&li<(int)lst.size()){
            idx=std::max(0,std::min(idx,(int)lst[li].items.size()));
            lst[li].items.insert(lst[li].items.begin()+idx,std::to_string(val));
        }
    }
    else if(t=="list_replace"&&block->inputs.size()>=3){
        int li=block->inputs[0]; int idx=block->inputs[1]-1; int val=block->inputs[2];
        auto& lst=rt->project->lists;
        if(li>=0&&li<(int)lst.size()&&idx>=0&&idx<(int)lst[li].items.size())
            lst[li].items[idx]=std::to_string(val);
    }
    else if(t=="list_show"&&!block->inputs.empty()){
        int li=block->inputs[0];
        if(li>=0&&li<(int)rt->project->lists.size()) rt->project->lists[li].visible=true;
    }
    else if(t=="list_hide"&&!block->inputs.empty()){
        int li=block->inputs[0];
        if(li>=0&&li<(int)rt->project->lists.size()) rt->project->lists[li].visible=false;
    }

    // ── Sensing ──
    else if(t=="touching_mouse"&&spr&&rt->sensing){ (void)sense_touching_mouse(*spr,*rt->sensing); }
    else if(t=="mouse_x"&&spr&&rt->sensing){ spr->x=sense_get_mouse_x(*rt->sensing); pen_move_to(spr->pen_state,cvs,spr->x,spr->y); }
    else if(t=="mouse_y"&&spr&&rt->sensing){ spr->y=sense_get_mouse_y(*rt->sensing); pen_move_to(spr->pen_state,cvs,spr->x,spr->y); }
    else if(t=="reset_timer"&&rt->sensing) sense_reset_timer(*rt->sensing);
    else if(t=="set_drag_mode"&&spr) sense_set_drag_mode(*spr,block->inputs.empty()?true:(block->inputs[0]!=0));
    else if(t=="ask"){
        std::string q = block->str_param.empty() ? "What's your name?" : block->str_param;
        rt->ask_active=true; rt->ask_buffer="";
        rt->ask_sprite_id=sr.sprite_id; rt->ask_resume_block=block->nextBlockId;
        sr.waiting_for_answer=true; sr.ask_question=q;
        if(spr){ spr->say_message=q; spr->say_timer=9999.f; }
        return;
    }

    // ── Broadcast ────────────────────────────────────────────────────────────
    else if(t=="broadcast"){
        std::string msg = block->str_param.empty() ? "message1" : block->str_param;
        runtime_broadcast(rt, msg, false);
    }
    else if(t=="broadcast_wait"){
        std::string msg = block->str_param.empty() ? "message1" : block->str_param;
        runtime_broadcast(rt, msg, true);
        // این sprite رو pause کن تا همه receivers تموم کنن
        sr.waiting_for_broadcast=true;
        sr.currentBlockId=block->nextBlockId;
        return;
    }
    else if(t=="when_receive"||t=="when_backdrop_switch"){ /* event trigger - no-op در exec */ }

    // ── Multiple Event Triggers ───────────────────────────────────────────────
    else if(t=="when_key_pressed"){ /* فقط اعلام - fire از main */ }
    else if(t=="when_clicked"){     /* فقط اعلام - fire از main */ }
    else if(t=="when_sprite_clicked"){ /* همان */ }

    // ── Clone ──
    else if(t=="create_clone"){
        int tid=sr.sprite_id;
        if(!block->inputs.empty()&&block->inputs[0]>=0) tid=block->inputs[0];
        runtime_create_clone(rt,tid);
    }
    else if(t=="delete_clone"){ if(sr.is_clone){ runtime_delete_clone(rt,sr.clone_id); return; } }

    // ── Control ──
    else if(t=="wait"){
        float secs=block->inputs.empty()?1.f:(float)block->inputs[0];
        ControlFrame f; f.blockId=block->id; f.is_wait=true;
        f.wait_end_ms=SDL_GetTicks()+(unsigned int)(secs*1000.f);
        f.after_loop_id=block->nextBlockId;
        sr.controlStack.push_back(f); return;
    }
    else if(t=="stop_all")  { runtime_stop(rt); return; }
    else if(t=="stop_this") { sr.state=RUNTIME_STOPPED; return; }
    else if(t=="forever"){
        ControlFrame f; f.blockId=block->id; f.loop_target=999999;
        f.childHeadId=block->nextBlockId; f.is_forever=true;
        if(f.childHeadId!=-1){ sr.controlStack.push_back(f); sr.currentBlockId=f.childHeadId; return; }
    }
    else if(t=="end_repeat"){
        if(!sr.controlStack.empty()){
            ControlFrame& cf=sr.controlStack.back(); cf.counter++;
            if(cf.is_forever||cf.counter<cf.loop_target){ sr.currentBlockId=cf.childHeadId; return; }
            else { int nxt=block->nextBlockId; sr.controlStack.pop_back(); sr.currentBlockId=nxt; return; }
        }
        sr.currentBlockId=block->nextBlockId; return;
    }
    else if(t=="if_then"){
        int cond=block->inputs.empty()?0:block->inputs[0];
        int body=block->inputs.size()>=2?block->inputs[1]:-1;
        if(cond&&body!=-1){ sr.currentBlockId=body; return; }
    }
    else if(t=="if_else"){
        int cond=block->inputs.empty()?0:block->inputs[0];
        int then_=block->inputs.size()>=2?block->inputs[1]:-1;
        int else_=block->inputs.size()>=3?block->inputs[2]:-1;
        int body=(cond?then_:else_);
        if(body!=-1){ sr.currentBlockId=body; return; }
    }
    else if(t=="when_start"||t=="when_clone_start"){ /* no-op */ }

    sr.currentBlockId=block->nextBlockId;
}

static void execRepeat(SpriteRuntime& sr, Block* block){
    int count=block->inputs.empty()?10:block->inputs[0];
    int head=block->nextBlockId;
    if(count<=0||head==-1){ sr.currentBlockId=-1; return; }
    ControlFrame f; f.blockId=block->id; f.loop_target=count; f.childHeadId=head;
    sr.controlStack.push_back(f); sr.currentBlockId=head;
}

static void tickSprite(Runtime* rt, SpriteRuntime& sr){
    if(sr.state!=RUNTIME_RUNNING) return;
    if(sr.waiting_for_answer) return;
    if(sr.waiting_for_broadcast) return;  // منتظر پایان broadcast_wait
    if(!sr.controlStack.empty()&&sr.controlStack.back().is_wait){
        ControlFrame& wf=sr.controlStack.back();
        if(SDL_GetTicks()>=wf.wait_end_ms){
            int nxt=wf.after_loop_id; sr.controlStack.pop_back(); sr.currentBlockId=nxt;
        }
        return;
    }
    if(sr.currentBlockId==-1){
        if(!sr.controlStack.empty()){
            ControlFrame& f=sr.controlStack.back();
            if(f.is_forever){ sr.currentBlockId=f.childHeadId; return; }
            sr.controlStack.pop_back(); sr.currentBlockId=-1;
        } else { sr.state=RUNTIME_STOPPED; }
        return;
    }
    Block* b=findBlock(rt->project,sr.currentBlockId);
    if(!b){ sr.state=RUNTIME_STOPPED; return; }
    if(b->type=="repeat") execRepeat(sr,b);
    else                  execBlock(rt,sr,b);
}

// ── processBroadcasts ─────────────────────────────────────────────────────────
static void processBroadcasts(Runtime* rt){
    while(!rt->broadcast_queue.empty()){
        BroadcastMsg msg=rt->broadcast_queue.front();
        rt->broadcast_queue.pop();
        std::cout<<"[Broadcast] "<<msg.name<<"\n";

        // پیدا کردن همه when_receive handlers
        for(auto& spr: rt->project->sprites){
            for(auto& hook: spr.event_hooks){
                if(hook.event_type=="when_receive" && hook.param==msg.name){
                    // پیدا کردن SpriteRuntime
                    for(auto& sr:rt->sprites){
                        if(sr.sprite_id==spr.id){
                            Block* sb=findBlock(rt->project,hook.block_id);
                            if(sb){
                                sr.state=RUNTIME_RUNNING;
                                sr.currentBlockId=sb->nextBlockId;
                                sr.controlStack.clear();
                            }
                        }
                    }
                }
            }
            // همچنین when_receive blocks در blocks list
            for(auto& b:rt->project->blocks){
                if(b.type=="when_receive" && b.str_param==msg.name && b.sprite_owner==spr.id){
                    for(auto& sr:rt->sprites){
                        if(sr.sprite_id==spr.id){
                            sr.state=RUNTIME_RUNNING;
                            sr.currentBlockId=b.nextBlockId;
                            sr.controlStack.clear();
                        }
                    }
                }
            }
        }

        // اگه broadcast_wait بود: resume سازنده‌اش
        if(msg.wait){
            for(auto& sr:rt->sprites)
                sr.waiting_for_broadcast=false;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
void runtime_broadcast(Runtime* rt, const std::string& name, bool wait){
    rt->broadcast_queue.push({name,wait});
}

void runtime_fire_event(Runtime* rt, const std::string& event_type, const std::string& param){
    // اجرا کردن همه بلوک‌هایی که event_type==type و param match
    for(auto& spr:rt->project->sprites){
        for(auto& b:rt->project->blocks){
            if(b.type==event_type && b.sprite_owner==spr.id &&
               (param.empty()||b.str_param==param)){
                for(auto& sr:rt->sprites){
                    if(sr.sprite_id==spr.id){
                        sr.state=RUNTIME_RUNNING;
                        sr.currentBlockId=b.nextBlockId;
                        sr.controlStack.clear();
                        sr.waiting_for_answer=false;
                    }
                }
            }
        }
    }
}

void runtime_create_clone(Runtime* rt, int sprite_id){
    Sprite* orig=findSprite(rt->project,sprite_id);
    if(!orig) return;
    int cid=rt->next_clone_id++;
    CloneSprite cs; cs.clone_id=cid; cs.sprite_id=sprite_id; cs.data=*orig;
    rt->clones.push_back(cs);
    int clone_start_id=-1;
    for(auto& b:rt->project->blocks)
        if(b.type=="when_clone_start"&&b.sprite_owner==sprite_id){ clone_start_id=b.id; break; }
    SpriteRuntime sr; sr.sprite_id=sprite_id; sr.clone_id=cid; sr.is_clone=true;
    sr.start_block_id=clone_start_id;
    sr.state=clone_start_id!=-1?RUNTIME_RUNNING:RUNTIME_STOPPED;
    if(clone_start_id!=-1){
        Block* sb=findBlock(rt->project,clone_start_id);
        sr.currentBlockId=sb?sb->nextBlockId:-1;
    }
    rt->clone_runtimes.push_back(sr);
}

void runtime_delete_clone(Runtime* rt, int clone_id){
    rt->clones.erase(std::remove_if(rt->clones.begin(),rt->clones.end(),
        [clone_id](const CloneSprite& c){return c.clone_id==clone_id;}),rt->clones.end());
    rt->clone_runtimes.erase(std::remove_if(rt->clone_runtimes.begin(),rt->clone_runtimes.end(),
        [clone_id](const SpriteRuntime& sr){return sr.clone_id==clone_id;}),rt->clone_runtimes.end());
}

void runtime_submit_answer(Runtime* rt, const std::string& ans){
    rt->answer=ans; rt->ask_active=false; rt->ask_buffer="";
    for(auto& v:rt->project->variables) if(v.name=="answer"){ v.value=0; break; }
    for(auto& sr:rt->sprites){
        if(sr.sprite_id==rt->ask_sprite_id&&sr.waiting_for_answer){
            sr.waiting_for_answer=false; sr.currentBlockId=rt->ask_resume_block;
            Sprite* spr=findSprite(rt->project,sr.sprite_id);
            if(spr){ spr->say_message=""; spr->say_timer=0; }
            break;
        }
    }
    rt->ask_sprite_id=-1; rt->ask_resume_block=-1;
}

void runtime_step(Runtime* rt){
    if(!rt) return;
    rt->step_pending=true;
}

void runtime_init(Runtime* rt, Project* project){
    rt->project=project; rt->state=RUNTIME_STOPPED;
    rt->sprites.clear(); rt->clone_runtimes.clear(); rt->clones.clear();
    rt->ask_active=false; rt->turbo_mode=false; rt->step_mode=false;
    while(!rt->broadcast_queue.empty()) rt->broadcast_queue.pop();
    for(auto& s:project->sprites){
        SpriteRuntime sr; sr.sprite_id=s.id; sr.is_clone=false;
        sr.start_block_id=s.when_start_id;
        if(sr.start_block_id==-1)
            for(auto& b:project->blocks)
                if(b.type=="when_start"&&(b.sprite_owner==s.id||b.sprite_owner==-1)){ sr.start_block_id=b.id; break; }
        rt->sprites.push_back(sr);
    }
}

void runtime_start(Runtime* rt){
    if(!rt) return;
    rt->state=RUNTIME_RUNNING;
    rt->clones.clear(); rt->clone_runtimes.clear(); rt->ask_active=false;
    rt->controlStack.clear();
    while(!rt->broadcast_queue.empty()) rt->broadcast_queue.pop();
    for(auto& sr:rt->sprites){
        Block* sb=findBlock(rt->project,sr.start_block_id);
        if(!sb) continue;
        sr.state=RUNTIME_RUNNING; sr.currentBlockId=sb->nextBlockId;
        sr.controlStack.clear(); sr.waiting_for_answer=false; sr.waiting_for_broadcast=false;
    }
    if(!rt->sprites.empty()) rt->currentBlockId=rt->sprites[0].currentBlockId;
}

void runtime_stop(Runtime* rt){
    if(!rt) return;
    rt->state=RUNTIME_STOPPED;
    for(auto& sr:rt->sprites){ sr.state=RUNTIME_STOPPED; sr.controlStack.clear(); sr.currentBlockId=-1; }
    rt->clone_runtimes.clear(); rt->clones.clear(); rt->ask_active=false;
    rt->step_highlight_block=-1;
}

void runtime_start_sprite(Runtime* rt, int sid){
    for(auto& sr:rt->sprites) if(sr.sprite_id==sid){
        Block* sb=findBlock(rt->project,sr.start_block_id);
        if(!sb) return;
        sr.state=RUNTIME_RUNNING; sr.currentBlockId=sb->nextBlockId; sr.controlStack.clear(); return;
    }
}
void runtime_stop_sprite(Runtime* rt, int sid){
    for(auto& sr:rt->sprites) if(sr.sprite_id==sid){ sr.state=RUNTIME_STOPPED; return; }
}

void runtime_tick(Runtime* rt){
    if(!rt||rt->state==RUNTIME_STOPPED) return;
    if(rt->state==RUNTIME_PAUSED) return;

    // Step mode: فقط وقتی step_pending باشه
    if(rt->step_mode && !rt->step_pending) return;
    rt->step_pending=false;

    int ticks=rt->turbo_mode?rt->turbo_ticks:1;
    for(int tt=0;tt<ticks;++tt){
        processBroadcasts(rt);
        for(auto& sr:rt->sprites)       tickSprite(rt,sr);
        for(auto& sr:rt->clone_runtimes) tickSprite(rt,sr);
    }
    // حذف clone های stopped
    auto it=rt->clone_runtimes.begin();
    while(it!=rt->clone_runtimes.end()){
        if(it->state==RUNTIME_STOPPED) it=rt->clone_runtimes.erase(it); else ++it;
    }
    if(!rt->sprites.empty()) rt->currentBlockId=rt->sprites[0].currentBlockId;
}

bool runtime_isRunning(const Runtime* rt){
    if(!rt||rt->state==RUNTIME_STOPPED) return false;
    if(rt->state==RUNTIME_PAUSED) return false;
    for(auto& sr:rt->sprites) if(sr.state==RUNTIME_RUNNING) return true;
    for(auto& sr:rt->clone_runtimes) if(sr.state==RUNTIME_RUNNING) return true;
    return false;
}
bool runtime_isPaused(const Runtime* rt){ return rt&&rt->state==RUNTIME_PAUSED; }
void runtime_pause(Runtime* rt){ if(rt&&runtime_isRunning(rt)) rt->state=RUNTIME_PAUSED; }
void runtime_resume(Runtime* rt){ if(rt&&rt->state==RUNTIME_PAUSED) rt->state=RUNTIME_RUNNING; }
void runtime_setWatchdogLimit(Runtime* rt, int limit){ if(rt) rt->watchdogLimit=limit; }
#include "runtime.h"
#include "vars_ops.h"
#include <cmath>
#include "motion.h"
#include "looks.h"
#include <iostream>

static Block* findBlockById(Project* project, int id){
    if(!project)return nullptr;
    for(auto& b:project->blocks) if(b.id==id)return &b;
    return nullptr;
}

void runtime_init(Runtime* rt, Project* project){
    rt->project=project; rt->currentBlockId=-1;
    rt->state=RUNTIME_STOPPED; rt->watchdogCounter=0;
    rt->watchdogLimit=10000; rt->controlStack.clear();
    rt->lastExecutedBlockId=-1;
    rt->returnStack.clear(); rt->loopCounter.clear();
}

void runtime_start(Runtime* rt){
    if(!rt)return;
    if(rt->currentBlockId==-1){ std::cout<<"[Runtime] No start block\n"; rt->state=RUNTIME_STOPPED; return; }
    rt->state=RUNTIME_RUNNING; rt->watchdogCounter=0; rt->controlStack.clear();
    std::cout<<"Runtime started. firstBlockId="<<rt->currentBlockId<<"\n";
}

void runtime_stop(Runtime* rt){
    if(!rt||rt->state==RUNTIME_STOPPED)return;
    rt->state=RUNTIME_STOPPED; rt->currentBlockId=-1; rt->controlStack.clear();
    std::cout<<"Runtime stopped.\n";
}

// اجرای واقعی یک بلوک primitive
static void executePrimitive(Runtime* rt, Block* block){
    rt->lastExecutedBlockId = block->id;
    std::cout<<"  -> exec: "<<block->type<<"\n";

    // اگر sprite‌ای وجود دارد، روی اولین sprite عمل کن
    Sprite* spr = rt->project->sprites.empty() ? nullptr : &rt->project->sprites[0];

    if(block->type=="move" && spr){
        float steps = block->inputs.empty() ? 10.0f : (float)block->inputs[0];
        motion_move(*spr, steps);
    }
    else if(block->type=="turn_right" && spr){
        float deg = block->inputs.empty() ? 15.0f : (float)block->inputs[0];
        motion_turn_right(*spr, deg);
    }
    else if(block->type=="turn_left" && spr){
        float deg = block->inputs.empty() ? 15.0f : (float)block->inputs[0];
        motion_turn_left(*spr, deg);
    }
    else if(block->type=="turn" && spr){
        float deg = block->inputs.empty() ? 15.0f : (float)block->inputs[0];
        motion_turn_right(*spr, deg);
    }
    else if(block->type=="goto_xy" && spr){
        float x = block->inputs.size()>=1 ? (float)block->inputs[0] : 0;
        float y = block->inputs.size()>=2 ? (float)block->inputs[1] : 0;
        motion_goto(*spr, x, y);
    }
    else if(block->type=="set_x" && spr){
        motion_set_x(*spr, block->inputs.empty() ? 0 : (float)block->inputs[0]);
    }
    else if(block->type=="set_y" && spr){
        motion_set_y(*spr, block->inputs.empty() ? 0 : (float)block->inputs[0]);
    }
    else if(block->type=="change_x" && spr){
        motion_change_x(*spr, block->inputs.empty() ? 10 : (float)block->inputs[0]);
    }
    else if(block->type=="change_y" && spr){
        motion_change_y(*spr, block->inputs.empty() ? 10 : (float)block->inputs[0]);
    }
    else if(block->type=="say" && spr){
        looks_say(*spr, "Hello!");
    }
    else if(block->type=="show" && spr){ looks_show(*spr); }
    else if(block->type=="hide" && spr){ looks_hide(*spr); }
    else if(block->type=="set_size" && spr){
        looks_set_size(*spr, block->inputs.empty() ? 100 : (float)block->inputs[0]);
    }
    // ── عملگرها ──
    else if(block->type.substr(0,3)=="op_"){
        if(block->inputs.size() >= 2){
            Value a = value_number(block->inputs[0]);
            Value b_val = value_number(block->inputs[1]);
            bool err = false;
            Value result = value_number(0);
            const std::string& op = block->type;
            if(op=="op_add")    result = op_add(a,b_val,err);
            else if(op=="op_sub")    result = op_sub(a,b_val,err);
            else if(op=="op_mul")    result = op_mul(a,b_val,err);
            else if(op=="op_div")    result = op_div(a,b_val,err);
            else if(op=="op_mod")    result = op_modulo(a,b_val,err);
            else if(op=="op_gt")     result = op_greater_than(a,b_val,err);
            else if(op=="op_lt")     result = op_less_than(a,b_val,err);
            else if(op=="op_eq")     result = op_equals(a,b_val);
            else if(op=="op_and")    result = op_and(a,b_val);
            else if(op=="op_or")     result = op_or(a,b_val);
            else if(op=="op_xor")    result = op_xor(a,b_val);
            if(!err) std::cout << "  op result: " << result.num << std::endl;
        } else if(block->inputs.size() >= 1){
            Value a = value_number(block->inputs[0]);
            bool err = false;
            Value result = value_number(0);
            const std::string& op = block->type;
            if(op=="op_not")   result = op_not(a);
            else if(op=="op_abs")   result = op_abs(a);
            else if(op=="op_floor") result = op_floor(a);
            else if(op=="op_ceil")  result = op_ceil(a);
            else if(op=="op_sqrt")  result = op_sqrt(a,err);
            else if(op=="op_sin")   result = op_sin(a);
            else if(op=="op_cos")   result = op_cos(a);
            else if(op=="op_round") result = value_number(std::round(a.num));
            if(!err) std::cout << "  op result: " << result.num << std::endl;
        }
    }
    else if(block->type=="when_start"){
        // no-op: فقط شروع اجرا
    }

    rt->currentBlockId = block->nextBlockId;
}

static void executeRepeat(Runtime* rt, Block* block){
    if(block->inputs.size()<2){ rt->currentBlockId=block->nextBlockId; return; }
    ControlFrame frame;
    frame.blockId=block->id; frame.loop_target=block->inputs[0];
    frame.childHeadId=block->inputs[1]; frame.counter=0;
    if(frame.loop_target<=0){ rt->currentBlockId=block->nextBlockId; return; }
    std::cout<<"  -> Entering Repeat (target:"<<frame.loop_target<<")\n";
    rt->controlStack.push_back(frame);
    rt->currentBlockId=frame.childHeadId;
}

static void executeBlock(Runtime* rt, Block* block){
    std::cout<<"Executing block id="<<block->id<<", type="<<block->type<<"\n";
    rt->watchdogCounter++;
    if(block->type=="repeat") executeRepeat(rt,block);
    else executePrimitive(rt,block);
}

void runtime_tick(Runtime* rt){
    if(rt->state!=RUNTIME_RUNNING)return;
    if(rt->watchdogCounter>rt->watchdogLimit){
        std::cout<<"!!! Watchdog! Halting !!!\n"; runtime_stop(rt); return;
    }
    if(rt->currentBlockId==-1){
        if(!rt->controlStack.empty()){
            ControlFrame& f=rt->controlStack.back(); f.counter++;
            if(f.counter<f.loop_target){ rt->currentBlockId=f.childHeadId; }
            else{
                std::cout<<"  -> Exiting Repeat\n";
                Block* cb=findBlockById(rt->project,f.blockId);
                rt->currentBlockId=cb?cb->nextBlockId:-1;
                rt->controlStack.pop_back();
            }
        } else { runtime_stop(rt); }
        return;
    }
    Block* b=findBlockById(rt->project,rt->currentBlockId);
    if(b) executeBlock(rt,b);
    else{ std::cout<<"[Runtime] Block not found id="<<rt->currentBlockId<<"\n"; runtime_stop(rt); }
}

bool runtime_isRunning(const Runtime* rt){ return rt&&rt->state==RUNTIME_RUNNING; }
bool runtime_isPaused(const Runtime* rt){ return rt&&rt->state==RUNTIME_PAUSED; }
void runtime_pause(Runtime* rt){ if(rt&&rt->state==RUNTIME_RUNNING)rt->state=RUNTIME_PAUSED; }
void runtime_resume(Runtime* rt){ if(rt&&rt->state==RUNTIME_PAUSED)rt->state=RUNTIME_RUNNING; }
void runtime_setWatchdogLimit(Runtime* rt,int limit){ if(rt)rt->watchdogLimit=limit; }
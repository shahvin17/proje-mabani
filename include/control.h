#ifndef CONTROL_H
#define CONTROL_H

#include "core_types.h"
#include "runtime.h"
using namespace std;

// repeat handler
void control_repeat(Runtime* rt, Block* block);

// if handler
void control_if(Runtime* rt, Block* block);

#endif

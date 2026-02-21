#ifndef CONTROL_H
#define CONTROL_H

// control.h باید runtime.h رو include کنه چون Runtime* در پارامترها استفاده میشه
#include "core_types.h"
#include "runtime.h"

void control_repeat(Runtime* rt, Block* block);
void control_if(Runtime* rt, Block* block);

#endif // CONTROL_H
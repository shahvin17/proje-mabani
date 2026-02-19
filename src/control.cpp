#include "control.h"
#include <iostream>
using namespace std;

/*
    مهم:
    در معماری فعلی، اجرای واقعی repeat و if
    در runtime.cpp انجام می‌شود.

    این توابع فعلاً فقط log می‌کنند
    و اجرای واقعی انجام نمی‌دهند
    تا duplicate logic ایجاد نشود.
*/

void control_repeat(Runtime* rt, Block* block) {

    if (!rt || !block) return;

    cout << "[control] repeat handler called for block id="
         << block->id << endl;

    // اجرای واقعی داخل runtime.cpp انجام می‌شود
}

void control_if(Runtime* rt, Block* block) {

    if (!rt || !block) return;

    cout << "[control] if handler called for block id="
         << block->id << endl;

    // اجرای واقعی داخل runtime.cpp انجام می‌شود
}
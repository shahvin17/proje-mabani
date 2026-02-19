#ifndef CONTROL_H
#define CONTROL_H

#include "core_types.h"
#include "runtime.h"

// توجه:
// در نسخه فعلی پروژه، منطق اجرای repeat و if
// داخل runtime.cpp پیاده‌سازی شده است.
// این فایل فقط به عنوان abstraction باقی می‌ماند
// تا در صورت refactor بعدی استفاده شود.

// repeat block handler
void control_repeat(Runtime* rt, Block* block);

// if block handler
void control_if(Runtime* rt, Block* block);

#endif

#include "runtime.h"
#include <iostream>
#include <cmath> // برای sin و cos

// ... (findBlockById و توابع lifecycle بدون تغییر) ...
// (برای اختصار، کدهای بدون تغییر حذف شده‌اند)

void executePrimitive(Runtime* rt, Block* block) {
    // پیدا کردن اولین Sprite در پروژه برای اعمال دستورات
    if (rt->project->sprites.empty()) {
        rt->currentBlockId = block->nextBlockId;
        return;
    }
    Sprite& target_sprite = rt->project->sprites[0];

    // --- اتصال منطق واقعی به بلوک‌ها ---
    if (block->type == "move") {
        if (!block->inputs.empty()) {
            float steps = block->inputs[0];
            float rad = target_sprite.direction * (3.14159 / 180.0);
            target_sprite.x += steps * sin(rad);
            target_sprite.y -= steps * cos(rad); // محور Y در SDL معکوس است
        }
    } else if (block->type == "turn") {
        if (!block->inputs.empty()) {
            target_sprite.direction += block->inputs[0];
        }
    } else if (block->type == "pen_down") {
        penDown(target_sprite.pen_state);
    } else if (block->type == "pen_up") {
        penUp(target_sprite.pen_state);
    }
    // بلوک say فعلاً فقط در کنسول چاپ می‌کند
    else if (block->type == "say") {
        std::cout << "Sprite says: Hello!" << std::endl;
    }

    rt->currentBlockId = block->nextBlockId;
}

// ... (بقیه توابع runtime.cpp که قبلا اصلاح کردیم، بدون تغییر باقی می‌مانند) ...
// (executeRepeat, executeBlock, runtime_tick)

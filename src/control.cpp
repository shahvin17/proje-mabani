#include "control.h"
#include <iostream>
using namespace std;

// ساده‌ترین پیاده‌سازی:
// block->inputs[0] = count
// block->inputs[1] = child block id

void control_repeat(Runtime* rt, Block* block) {
    if (!rt || !block) return;

    if (block->inputs.size() < 2) {
        cout << "Repeat block missing inputs." << endl;
        return;
    }

    int count = block->inputs[0];
    int childId = block->inputs[1];

    if (count <= 0) return;

    cout << "Repeat " << count << " times" << endl;

    for (int i = 0; i < count; i++) {
        Block* child = nullptr;

        // پیدا کردن child
        for (auto& b : rt->project->blocks) {
            if (b.id == childId) {
                child = &b;
                break;
            }
        }

        if (!child) {
            cout << "Repeat child not found." << endl;
            return;
        }

        cout << "  Repeat iteration " << i+1 << endl;

        // اجرای child
        cout << "  Executing child block id=" << child->id << endl;

        // watchdog هم اینجا چک میشه
        rt->watchdogCounter++;
        if (rt->watchdogCounter > rt->watchdogLimit) {
            cout << "Watchdog limit reached in repeat." << endl;
            runtime_stop(rt);
            return;
        }
    }
}

void control_if(Runtime* rt, Block* block) {
    if (!rt || !block) return;

    if (block->inputs.size() < 2) {
        cout << "If block missing inputs." << endl;
        return;
    }

    int condition = block->inputs[0];
    int childId   = block->inputs[1];

    if (condition) {
        cout << "If condition TRUE" << endl;

        Block* child = nullptr;

        for (auto& b : rt->project->blocks) {
            if (b.id == childId) {
                child = &b;
                break;
            }
        }

        if (child) {
            cout << "Executing IF child id=" << child->id << endl;
        }
    } else {
        cout << "If condition FALSE" << endl;
    }
}
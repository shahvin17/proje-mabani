#include <iostream>
#include "core_types.h"
#include "runtime.h"

int main() {

    // 🔹 ساخت پروژه تست
    Project project;

    Block b1 {1, BLOCK_MOVE, 2, -1, {0}};
    Block b2 {2, BLOCK_TURN, -1, -1, {0}};

    project.blocks.push_back(b1);
    project.blocks.push_back(b2);

    // 🔹 ساخت Runtime
    Runtime rt;
    runtime_init(&rt, &project);

    runtime_start(&rt);

    while (rt.state == RUNTIME_RUNNING) {
        runtime_executeCurrent(&rt);
    }

    return 0;
}
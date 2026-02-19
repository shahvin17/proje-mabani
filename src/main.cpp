#include <iostream>
#include "core_types.h"
#include "runtime.h"
#include "persist.h"
#include <vars_ops.h>
using namespace std;

int main() {

    cout << "=== Day 1 Runtime Test ===" << endl;

    // ساخت پروژه تستی
    Project project;

    // بلاک اول
    Block b1;
    b1.id = 1;
    b1.type = "move";
    b1.nextBlockId = 2;
    b1.x = 0;
    b1.y = 0;

    // بلاک دوم
    Block b2;
    b2.id = 2;
    b2.type = "turn";
    b2.nextBlockId = -1;
    b2.x = 0;
    b2.y = 0;

    project.blocks.push_back(b1);
    project.blocks.push_back(b2);

    // ساخت runtime
    Runtime rt;
    runtime_init(&rt, &project);

    runtime_start(&rt);

    while (runtime_isRunning(&rt)) {
        runtime_tick(&rt);
    }

    cout << "=== Execution Finished ===" << endl;

    // تست Save
    if (saveProject(project, "test_project.txt")) {
        cout << "Project saved successfully." << endl;
    } else {
        cout << "Save failed." << endl;
    }
    Project loaded;

    if (loadProject(loaded, "test_project.txt")) {
        cout << "Load successful." << endl;
        cout << "Loaded blocks count: " << loaded.blocks.size() << endl;
    } else {
        cout << "Load failed." << endl;
    }
    return 0;
}
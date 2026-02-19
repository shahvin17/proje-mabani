/*#include <iostream>
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
*/

/*#include <iostream>
#include "core_types.h"
#include "runtime.h"

using namespace std;

int main() {

    cout << "=== Scratch Engine Day 4 Test ===" << endl;

    Project project;

    /*
        Block layout:

        1: move
        2: repeat (2 times, child=3)
        3: move (inside repeat)
        4: if (true, child=5)
        5: turn
        6: turn (after repeat)

        chain:
        1 -> 2 -> 6
        3 -> 4
    

    Block b1;
    b1.id = 1;
    b1.type = "move";
    b1.nextBlockId = 2;

    Block b2;
    b2.id = 2;
    b2.type = "repeat";
    b2.inputs = {2, 3};   // repeat 2 times, child id=3
    b2.nextBlockId = 6;

    Block b3;
    b3.id = 3;
    b3.type = "move";
    b3.nextBlockId = 4;

    Block b4;
    b4.id = 4;
    b4.type = "if";
    b4.inputs = {1, 5};   // condition true, child=5
    b4.nextBlockId = -1;

    Block b5;
    b5.id = 5;
    b5.type = "turn";
    b5.nextBlockId = -1;

    Block b6;
    b6.id = 6;
    b6.type = "turn";
    b6.nextBlockId = -1;

    project.blocks.push_back(b1);
    project.blocks.push_back(b2);
    project.blocks.push_back(b3);
    project.blocks.push_back(b4);
    project.blocks.push_back(b5);
    project.blocks.push_back(b6);

    Runtime rt;
    runtime_init(&rt, &project);
    runtime_start(&rt);

    while (runtime_isRunning(&rt)) {
        runtime_tick(&rt);
    }

    cout << "=== Execution Finished ===" << endl;

    return 0;
}
*/



#include <SDL.h>
#include <iostream>
#include "render.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL init failed\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Scratch - Day 1 (A)",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window creation failed\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        std::cerr << "Renderer creation failed\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        renderTestRect(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

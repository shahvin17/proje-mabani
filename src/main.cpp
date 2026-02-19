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

    while (rt.state == RUNTIME_RUNNING) {
        runtime_executeCurrent(&rt);
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
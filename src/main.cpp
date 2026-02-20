#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <map>

#include "core_types.h"
#include "render.h"
#include "runtime.h"

// تعریف یک پالت رنگ برای بلوک‌ها (اختیاری ولی برای خوانایی بهتر)
map<string, SDL_Color> block_colors = {
    {"when_start", {255, 170, 0, 255}},   // زرد/نارنجی
    {"move", {70, 140, 255, 255}},        // آبی
    {"turn", {70, 140, 255, 255}},        // آبی
    {"repeat", {255, 120, 50, 255}},     // نارنجی
    {"say", {160, 100, 255, 255}},        // بنفش
    {"default", {150, 150, 150, 255}}      // خاکستری
};

SDL_Color getColorForBlock(const string& type) {
    if (block_colors.count(type)) {
        return block_colors[type];
    }
    return block_colors["default"];
}

int main(int argc, char* argv[]) {
    // 1. آماده‌سازی
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Scratch Project - UI Core", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Project project;
    project.blocks.push_back({1, "when_start", {}, 2, 50, 50});
    project.blocks.push_back({2, "move", {10}, 3, 50, 120});
    project.blocks.push_back({3, "repeat", {4, 4}, 5, 50, 190});
    project.blocks.push_back({4, "turn", {15}, -1, 150, 220});
    project.blocks.push_back({5, "say", {}, -1, 50, 260});

    Runtime rt;
    runtime_init(&rt, &project);

    // 2. متغیرهای حلقه
    bool running = true;
    SDL_Event event;
    Block* dragging_block = nullptr;
    int offset_x = 0;
    int offset_y = 0;

    // 3. حلقه اصلی
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            switch (event.type) {
                case SDL_MOUSEBUTTONDOWN:
                    for (int i = project.blocks.size() - 1; i >= 0; --i) {
                        Block& b = project.blocks[i];
                        SDL_Rect r = {(int)b.x, (int)b.y, (int)b.width, (int)b.height};
                        if (isInside(r, event.button.x, event.button.y)) {
                            dragging_block = &b;
                            offset_x = event.button.x - b.x;
                            offset_y = event.button.y - b.y;
                            break;
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    dragging_block = nullptr;
                    break;

                case SDL_MOUSEMOTION:
                    if (dragging_block) {
                        dragging_block->x = event.motion.x - offset_x;
                        dragging_block->y = event.motion.y - offset_y;
                    }
                    break;
                
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_r) {
                        cout << "[INFO] 'R' key pressed. Starting runtime..." << endl;
                        for(const auto& block : project.blocks) {
                            if (block.type == "when_start") {
                                rt.currentBlockId = block.id;
                                break;
                            }
                        }
                        runtime_start(&rt);
                    }
                    break;
            }
        }

        if (runtime_isRunning(&rt)) {
            runtime_tick(&rt);
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (const auto& block : project.blocks) {
            SDL_Rect r = {(int)block.x, (int)block.y, (int)block.width, (int)block.height};
            SDL_Color color = getColorForBlock(block.type);
            renderRect(renderer, r, color);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // 4. آزادسازی
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}


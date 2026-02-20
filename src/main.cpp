#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

// --- تغییر اصلی اینجاست ---
// هدرهای لازم برای استفاده از std::map و std::string اضافه شدند
#include <map>
#include <string>

// هدرهای پروژه
#include "core_types.h"
#include "render.h"
#include "runtime.h"

// --- بخش کمکی برای رنگ‌بندی بلوک‌ها ---
// تعریف یک پالت رنگ برای انواع مختلف بلوک‌ها
std::map<std::string, SDL_Color> block_colors = {
    {"when_start", {255, 170, 0, 255}},   // زرد/نارنجی برای رویدادها
    {"move", {70, 140, 255, 255}},        // آبی برای حرکت
    {"turn", {70, 140, 255, 255}},        // آبی برای حرکت
    {"repeat", {255, 120, 50, 255}},     // نارنجی برای کنترل
    {"say", {160, 100, 255, 255}},        // بنفش برای ظاهر
    {"default", {150, 150, 150, 255}}      // خاکستری برای بقیه
};

// تابعی برای گرفتن رنگ مناسب بر اساس نوع بلوک
SDL_Color getColorForBlock(const std::string& type) {
    if (block_colors.count(type)) {
        return block_colors[type];
    }
    return block_colors["default"];
}

// تابعی برای پیدا کردن یک بلوک بر اساس آی‌دی آن
Block* findBlockById(std::vector<Block>& blocks, int id) {
    for (auto& b : blocks) {
        if (b.id == id) {
            return &b;
        }
    }
    return nullptr;
}


int main(int argc, char* argv[]) {
    // =========================
    // 1. آماده‌سازی SDL و پروژه
    // =========================
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Scratch Project - Snap & Run", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // ساخت یک پروژه تستی با چند بلوک
    Project project;
    project.blocks.push_back({1, "when_start", {}, -1, 50, 50});
    project.blocks.push_back({2, "move", {10}, -1, 300, 100});
    project.blocks.push_back({3, "repeat", {4, 4}, -1, 550, 150});
    project.blocks.push_back({4, "turn", {15}, -1, 300, 250});
    project.blocks.push_back({5, "say", {}, -1, 50, 350});

    // آماده‌سازی موتور اجرا
    Runtime rt;
    runtime_init(&rt, &project);

    // =========================
    // 2. متغیرهای حلقه اصلی
    // =========================
    bool running = true;
    SDL_Event event;
    Block* dragging_block = nullptr;
    int offset_x = 0;
    int offset_y = 0;

    // =========================
    // 3. حلقه اصلی برنامه
    // =========================
    while (running) {
        // --- بخش مدیریت رویدادها ---
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

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
                    if (dragging_block) {
                        bool snapped = false;

                        for (auto& potential_prev_block : project.blocks) {
                            if (potential_prev_block.nextBlockId == dragging_block->id) {
                                potential_prev_block.nextBlockId = -1;
                                break;
                            }
                        }

                        for (auto& target_block : project.blocks) {
                            if (dragging_block->id == target_block.id) continue;

                            SDL_Rect snap_zone = {(int)target_block.x, (int)(target_block.y + target_block.height), (int)target_block.width, 20};
                            SDL_Point top_of_dragging_block = {(int)dragging_block->x + (int)dragging_block->width / 2, (int)dragging_block->y};

                            if (SDL_PointInRect(&top_of_dragging_block, &snap_zone)) {
                                dragging_block->x = target_block.x;
                                dragging_block->y = target_block.y + target_block.height + 2;

                                int old_next_id = target_block.nextBlockId;
                                target_block.nextBlockId = dragging_block->id;
                                dragging_block->nextBlockId = old_next_id;

                                snapped = true;
                                std::cout << "[SNAP] Block " << dragging_block->id << " snapped to Block " << target_block.id << std::endl;
                                break;
                            }
                        }
                    }
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
                        std::cout << "[INFO] 'R' key pressed. Starting runtime..." << std::endl;
                        for(const auto& block : project.blocks) {
                            if (block.type == "when_start") {
                                rt.currentBlockId = block.id;
                                runtime_start(&rt);
                                break;
                            }
                        }
                    }
                    break;
            }
        }

        // --- بخش اجرای منطق ---
        if (runtime_isRunning(&rt)) {
            runtime_tick(&rt);
        }

        // --- بخش رندر ---
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (const auto& block : project.blocks) {
            SDL_Rect r = {(int)block.x, (int)block.y, (int)block.width, (int)block.height};
            SDL_Color color = getColorForBlock(block.type);
            renderRect(renderer, r, color);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        for (auto& block : project.blocks) {
            if (block.nextBlockId != -1) {
                Block* next_block = findBlockById(project.blocks, block.nextBlockId);
                if (next_block) {
                    int x1 = block.x + block.width / 2;
                    int y1 = block.y + block.height;
                    int x2 = next_block->x + next_block->width / 2;
                    int y2 = next_block->y;
                    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                }
            }
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    // =========================
    // 4. آزادسازی منابع
    // =========================
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

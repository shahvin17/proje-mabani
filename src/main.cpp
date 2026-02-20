#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>

// هدرهای پروژه
#include "core_types.h"
#include "render.h"
#include "runtime.h"

// --- بخش کمکی برای رنگ‌بندی بلوک‌ها ---
std::map<std::string, SDL_Color> block_colors = {
    {"when_start", {255, 170, 0, 255}},
    {"move", {70, 140, 255, 255}},
    {"turn", {70, 140, 255, 255}},
    {"repeat", {255, 120, 50, 255}},
    {"say", {160, 100, 255, 255}},
    {"default", {150, 150, 150, 255}}
};

SDL_Color getColorForBlock(const std::string& type) {
    if (block_colors.count(type)) return block_colors[type];
    return block_colors["default"];
}

Block* findBlockById(std::vector<Block>& blocks, int id) {
    for (auto& b : blocks) {
        if (b.id == id) return &b;
    }
    return nullptr;
}


int main(int argc, char* argv[]) {
    // =========================
    // 1. آماده‌سازی
    // =========================
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Scratch Clone - Palette Layout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // --- منطق جدید: تعریف نواحی و قالب‌ها ---
    SDL_Rect palette_area = {0, 0, 250, 720};
    SDL_Rect script_area = {250, 0, 1280 - 250, 720};

    std::vector<Block> template_blocks;
    template_blocks.push_back({-1, "when_start", {}, -1, 30, 30});
    template_blocks.push_back({-2, "move", {10}, -1, 30, 100});
    template_blocks.push_back({-3, "turn", {15}, -1, 30, 170});
    template_blocks.push_back({-4, "repeat", {4, 4}, -1, 30, 240});
    template_blocks.push_back({-5, "say", {}, -1, 30, 310});

    Project project; // ناحیه اسکریپت در ابتدا خالی است
    Runtime rt;
    runtime_init(&rt, &project);
    int next_block_id = 1;

    // =========================
    // 2. متغیرهای حلقه
    // =========================
    bool running = true;
    SDL_Event event;
    Block* dragging_block = nullptr;
    int offset_x = 0;
    int offset_y = 0;

    // =========================
    // 3. حلقه اصلی
    // =========================
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            switch (event.type) {
                case SDL_MOUSEBUTTONDOWN: { // Scope for variable definition
                    SDL_Point mouse_pos = {event.button.x, event.button.y};

                    // آیا کلیک در پالت است؟
                    if (SDL_PointInRect(&mouse_pos, &palette_area)) {
                        for (const auto& template_block : template_blocks) {
                            SDL_Rect r = {(int)template_block.x, (int)template_block.y, (int)template_block.width, (int)template_block.height};
                            if (isInside(r, mouse_pos.x, mouse_pos.y)) {
                                project.blocks.push_back(template_block); // کپی کردن قالب
                                Block& new_block = project.blocks.back();

                                new_block.id = next_block_id++;
                                new_block.x = mouse_pos.x - (new_block.width / 2); // از وسط بگیر
                                new_block.y = mouse_pos.y - (new_block.height / 2);
                                
                                dragging_block = &new_block;
                                offset_x = new_block.width / 2;
                                offset_y = new_block.height / 2;
                                break;
                            }
                        }
                    } 
                    // اگر نه، آیا کلیک در ناحیه اسکریپت است؟
                    else if (SDL_PointInRect(&mouse_pos, &script_area)) {
                        for (int i = project.blocks.size() - 1; i >= 0; --i) {
                            Block& b = project.blocks[i];
                            SDL_Rect r = {(int)b.x, (int)b.y, (int)b.width, (int)b.height};
                            if (isInside(r, mouse_pos.x, mouse_pos.y)) {
                                dragging_block = &b;
                                offset_x = mouse_pos.x - b.x;
                                offset_y = mouse_pos.y - b.y;
                                break;
                            }
                        }
                    }
                    break;
                }

                case SDL_MOUSEBUTTONUP:
                    if (dragging_block) {
                        bool snapped = false;
                        for (auto& pp_block : project.blocks) {
                            if (pp_block.nextBlockId == dragging_block->id) {
                                pp_block.nextBlockId = -1;
                                break;
                            }
                        }
                        for (auto& target_block : project.blocks) {
                            if (dragging_block->id == target_block.id) continue;
                            SDL_Rect snap_zone = {(int)target_block.x, (int)(target_block.y + target_block.height), (int)target_block.width, 20};
                            SDL_Point drag_point = {(int)dragging_block->x + (int)dragging_block->width / 2, (int)dragging_block->y};
                            if (SDL_PointInRect(&drag_point, &snap_zone)) {
                                dragging_block->x = target_block.x;
                                dragging_block->y = target_block.y + target_block.height + 2;
                                int old_next = target_block.nextBlockId;
                                target_block.nextBlockId = dragging_block->id;
                                dragging_block->nextBlockId = old_next;
                                snapped = true;
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

        if (runtime_isRunning(&rt)) runtime_tick(&rt);

        // --- بخش رندر ---
        SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255);
        SDL_RenderFillRect(renderer, &palette_area);
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderFillRect(renderer, &script_area);

        for (const auto& block : template_blocks) {
            SDL_Rect r = {(int)block.x, (int)block.y, (int)block.width, (int)block.height};
            renderRect(renderer, r, getColorForBlock(block.type));
        }

        for (const auto& block : project.blocks) {
            SDL_Rect r = {(int)block.x, (int)block.y, (int)block.width, (int)block.height};
            renderRect(renderer, r, getColorForBlock(block.type));
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        for (auto& block : project.blocks) {
            if (block.nextBlockId != -1) {
                Block* next_block = findBlockById(project.blocks, block.nextBlockId);
                if (next_block) {
                    SDL_RenderDrawLine(renderer, block.x + block.width / 2, block.y + block.height, next_block->x + next_block->width / 2, next_block->y);
                }
            }
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

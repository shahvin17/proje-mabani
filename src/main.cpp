#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "core_types.h"
#include "render.h"
#include "runtime.h"

// ... (بخش رنگ‌ها و توابع کمکی بدون تغییر) ...
std::map<std::string, SDL_Color> block_colors = {
    {"when_start", {255, 170, 0, 255}},
    {"move", {70, 140, 255, 255}},
    {"turn", {70, 140, 255, 255}},
    {"repeat", {255, 120, 50, 255}},
    {"say", {160, 100, 255, 255}},
    {"default", {150, 150, 150, 255}}
};
SDL_Color getColorForBlock(const std::string& type) { if (block_colors.count(type)) return block_colors[type]; return block_colors["default"]; }
Block* findBlockById(std::vector<Block>& blocks, int id) { for (auto& b : blocks) if (b.id == id) return &b; return nullptr; }


int main(int argc, char* argv[]) {
    // 1. آماده‌سازی
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Scratch Clone - Final Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Rect palette_area = {0, 0, 250, 720};
    SDL_Rect script_area = {250, 0, 1280 - 250, 720};

    // --- تغییر اصلی: بازنویسی کامل داده‌های تستی ---
    
    // بلوک‌های قالب برای پالت (بدون تغییر)
    std::vector<Block> template_blocks;
    template_blocks.push_back({-1, "when_start", {}, -1, 30, 30});
    template_blocks.push_back({-2, "move", {10}, -1, 30, 100});
    template_blocks.push_back({-3, "repeat", {4, 4}, -1, 30, 170}); // مقدار childId (دومین 4) در اینجا مهم نیست
    template_blocks.push_back({-4, "turn", {15}, -1, 30, 240});
    template_blocks.push_back({-5, "say", {}, -1, 30, 310});

    // پروژه را با یک اسکریپت از پیش ساخته شده برای تست شروع می‌کنیم
    Project project;
    // زنجیره اصلی: when_start -> move -> repeat -> say
    project.blocks.push_back({1, "when_start", {}, 2, 300, 50});
    project.blocks.push_back({2, "move", {10}, 3, 300, 120});
    // بلوک repeat: ۴ بار تکرار کن، فرزند اولش بلوک با id=4 است
    project.blocks.push_back({3, "repeat", {4, 4}, 5, 300, 190});
    project.blocks.push_back({5, "say", {}, -1, 300, 320});
    
    // فرزند حلقه repeat: فقط یک بلوک turn
    project.blocks.push_back({4, "turn", {15}, -1, 450, 220}); // این بلوک داخل حلقه اجرا می‌شود

    Runtime rt;
    runtime_init(&rt, &project);
    int next_block_id = 6; // چون تا id=5 استفاده کرده‌ایم

    // ... (بقیه متغیرهای حلقه بدون تغییر) ...
    bool running = true;
    SDL_Event event;
    Block* dragging_block = nullptr;
    int offset_x = 0, offset_y = 0;

    // ... (کل حلقه while و منطق Drag/Drop/Snap/Render بدون تغییر باقی می‌ماند) ...
    // ... (برای جلوگیری از تکرار، کد کامل حلقه در اینجا حذف شده، شما نیازی به تغییر آن ندارید) ...
    // ... (همان کد کامل و صحیح قبلی را استفاده کنید) ...
    
    // فقط برای اطمینان، کد کامل حلقه while را هم اینجا قرار می‌دهم:
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            switch (event.type) {
                case SDL_MOUSEBUTTONDOWN: {
                    SDL_Point mouse_pos = {event.button.x, event.button.y};
                    if (SDL_PointInRect(&mouse_pos, &palette_area)) {
                        for (const auto& tb : template_blocks) {
                            SDL_Rect r = {(int)tb.x, (int)tb.y, (int)tb.width, (int)tb.height};
                            if (isInside(r, mouse_pos.x, mouse_pos.y)) {
                                project.blocks.push_back(tb);
                                Block& new_block = project.blocks.back();
                                new_block.id = next_block_id++;
                                new_block.x = mouse_pos.x - (new_block.width / 2);
                                new_block.y = mouse_pos.y - (new_block.height / 2);
                                dragging_block = &new_block;
                                offset_x = new_block.width / 2;
                                offset_y = new_block.height / 2;
                                break;
                            }
                        }
                    } else if (SDL_PointInRect(&mouse_pos, &script_area)) {
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
                        for (auto& pb : project.blocks) if (pb.nextBlockId == dragging_block->id) { pb.nextBlockId = -1; break; }
                        for (auto& tb : project.blocks) {
                            if (dragging_block->id == tb.id) continue;
                            SDL_Rect sz = {(int)tb.x, (int)(tb.y + tb.height), (int)tb.width, 20};
                            SDL_Point dp = {(int)dragging_block->x + (int)dragging_block->width / 2, (int)dragging_block->y};
                            if (SDL_PointInRect(&dp, &sz)) {
                                dragging_block->x = tb.x;
                                dragging_block->y = tb.y + tb.height + 2;
                                int old_next = tb.nextBlockId;
                                tb.nextBlockId = dragging_block->id;
                                dragging_block->nextBlockId = old_next;
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
                        runtime_init(&rt, &project); // ریست کردن runtime قبل از هر اجرا
                        for (const auto& b : project.blocks) if (b.type == "when_start") { rt.currentBlockId = b.id; runtime_start(&rt); break; }
                    }
                    break;
            }
        }
        if (runtime_isRunning(&rt)) runtime_tick(&rt);
        SDL_SetRenderDrawColor(renderer, 45, 45, 45, 255); SDL_RenderFillRect(renderer, &palette_area);
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); SDL_RenderFillRect(renderer, &script_area);
        for (const auto& b : template_blocks) { SDL_Rect r = {(int)b.x, (int)b.y, (int)b.width, (int)b.height}; renderRect(renderer, r, getColorForBlock(b.type)); }
        for (const auto& b : project.blocks) { SDL_Rect r = {(int)b.x, (int)b.y, (int)b.width, (int)b.height}; renderRect(renderer, r, getColorForBlock(b.type)); }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        for (auto& b : project.blocks) {
            if (b.nextBlockId != -1) {
                Block* nb = findBlockById(project.blocks, b.nextBlockId);
                if (nb) SDL_RenderDrawLine(renderer, b.x + b.width / 2, b.y + b.height, nb->x + nb->width / 2, nb->y);
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

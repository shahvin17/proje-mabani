// main.cpp — رابط کاربری اصلی پروژه Scratch C++
// تغییرات نسبت به نسخه قبل:
//  1. Toolbar بالا با دکمه‌های Run/Stop/Step
//  2. Sidebar چپ با دسته‌بندی رنگی (Motion/Events/Control/...)
//  3. Stage راست برای نمایش sprite
//  4. Script area وسط برای drag & drop بلوک‌ها
//  5. پشتیبانی از SDL_ttf برای نمایش متن روی بلوک‌ها
//  6. منطق drag به ui_drag.cpp منتقل شده

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

#include "core_types.h"
#include "runtime.h"
#include "render.h"
#include "persist.h"
#include "logger.h"

// ─── ثابت‌های ابعاد UI ───────────────────────────────────────────────────────
static const int WINDOW_W      = 1280;
static const int WINDOW_H      = 720;
static const int TOOLBAR_H     = 50;
static const int SIDEBAR_W     = 220;  // پانل دسته‌بندی + پالت
static const int CATEGORY_W    = 60;   // ستون آیکون دسته‌بندی
static const int PALETTE_W     = SIDEBAR_W - CATEGORY_W;
static const int STAGE_W       = 320;
static const int STAGE_H       = 240;
static const int BLOCK_H       = 44;
static const int BLOCK_W       = 150;

// ─── رنگ‌های دسته‌بندی (مطابق Scratch اصلی) ─────────────────────────────────
struct CategoryInfo {
    std::string name;
    SDL_Color   color;
};

static const std::vector<CategoryInfo> CATEGORIES = {
    {"Motion",    {74,  144, 226, 255}},
    {"Looks",     {153, 102, 204, 255}},
    {"Sound",     {207,  99, 207, 255}},
    {"Events",    {255, 171,  25, 255}},
    {"Control",   {255, 171,  25, 255}},  // نارنجی‌تر
    {"Sensing",   {92,  196, 220, 255}},
    {"Operators", {89,  192,  89, 255}},
    {"Variables", {255, 140,  26, 255}},
};

// ─── اطلاعات یک بلوک قالب در پالت ───────────────────────────────────────────
struct PaletteEntry {
    std::string type;
    std::string label;        // متنی که روی بلوک نمایش داده می‌شود
    std::vector<int> default_inputs;
    std::string category;
};

static const std::vector<PaletteEntry> PALETTE_ENTRIES = {
    // Motion
    {"move",            "move 10 steps",          {10},    "Motion"},
    {"turn_right",      "turn right 15°",         {15},    "Motion"},
    {"turn_left",       "turn left 15°",          {15},    "Motion"},
    {"goto_xy",         "go to x:0 y:0",          {0,0},   "Motion"},
    {"set_x",           "set x to 0",             {0},     "Motion"},
    {"set_y",           "set y to 0",             {0},     "Motion"},
    {"change_x",        "change x by 10",         {10},    "Motion"},
    {"change_y",        "change y by 10",         {10},    "Motion"},
    {"bounce",          "if on edge, bounce",     {},      "Motion"},
    // Looks
    {"say",             "say Hello!",             {},      "Looks"},
    {"say_for",         "say Hello! 2 secs",      {2},     "Looks"},
    {"show",            "show",                   {},      "Looks"},
    {"hide",            "hide",                   {},      "Looks"},
    {"set_size",        "set size to 100%",       {100},   "Looks"},
    {"change_size",     "change size by 10",      {10},    "Looks"},
    // Sound
    {"play_sound",      "play sound",             {},      "Sound"},
    {"stop_sounds",     "stop all sounds",        {},      "Sound"},
    // Events
    {"when_start",      "when 🏁 clicked",        {},      "Events"},
    {"when_key",        "when space pressed",     {},      "Events"},
    {"when_clicked",    "when sprite clicked",    {},      "Events"},
    // Control
    {"wait",            "wait 1 secs",            {1},     "Control"},
    {"repeat",          "repeat 10",              {10},    "Control"},
    {"forever",         "forever",                {},      "Control"},
    {"if_block",        "if <> then",             {},      "Control"},
    {"if_else",         "if <> else",             {},      "Control"},
    {"stop_all",        "stop all",               {},      "Control"},
    // Sensing
    {"touching_mouse",  "touching mouse?",        {},      "Sensing"},
    {"key_pressed",     "key space pressed?",     {},      "Sensing"},
    {"mouse_down",      "mouse down?",            {},      "Sensing"},
    {"mouse_x",         "mouse x",                {},      "Sensing"},
    {"mouse_y",         "mouse y",                {},      "Sensing"},
    // Operators
    {"op_add",          "( ) + ( )",              {},      "Operators"},
    {"op_sub",          "( ) - ( )",              {},      "Operators"},
    {"op_mul",          "( ) * ( )",              {},      "Operators"},
    {"op_div",          "( ) / ( )",              {},      "Operators"},
    {"op_gt",           "( ) > ( )",              {},      "Operators"},
    {"op_lt",           "( ) < ( )",              {},      "Operators"},
    {"op_eq",           "( ) = ( )",              {},      "Operators"},
    // Variables
    {"set_var",         "set var to 0",           {0},     "Variables"},
    {"change_var",      "change var by 1",        {1},     "Variables"},
    {"show_var",        "show variable var",      {},      "Variables"},
};

// ─── رنگ بلوک بر اساس دسته‌بندی ──────────────────────────────────────────────
SDL_Color colorForCategory(const std::string& cat) {
    for (auto& c : CATEGORIES)
        if (c.name == cat) return c.color;
    return {150, 150, 150, 255};
}

SDL_Color colorForBlockType(const std::string& type) {
    for (auto& e : PALETTE_ENTRIES)
        if (e.type == type) return colorForCategory(e.category);
    return {150, 150, 150, 255};
}

std::string labelForBlockType(const std::string& type) {
    for (auto& e : PALETTE_ENTRIES)
        if (e.type == type) return e.label;
    return type;
}

// ─── کمک‌های رندر ────────────────────────────────────────────────────────────
static void fillRect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rc = {x, y, w, h};
    SDL_RenderFillRect(r, &rc);
}

static void drawRect(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect rc = {x, y, w, h};
    SDL_RenderDrawRect(r, &rc);
}

// رندر یک بلوک با گوشه‌های گرد شبیه‌سازی‌شده (۳ لایه)
static void renderBlock(SDL_Renderer* r, int x, int y, int w, int h, SDL_Color col, bool highlighted = false) {
    // سایه
    fillRect(r, x+3, y+3, w, h, {0,0,0,60});
    // بدنه اصلی
    fillRect(r, x, y, w, h, col);
    // نوار بالایی روشن‌تر
    SDL_Color top = {(Uint8)std::min(col.r+40,255),(Uint8)std::min(col.g+40,255),(Uint8)std::min(col.b+40,255),255};
    fillRect(r, x, y, w, 4, top);
    // کادر highlight
    if (highlighted) {
        SDL_SetRenderDrawColor(r, 255,255,0,255);
        SDL_Rect rc = {x,y,w,h};
        SDL_RenderDrawRect(r, &rc);
        SDL_Rect rc2 = {x+1,y+1,w-2,h-2};
        SDL_RenderDrawRect(r, &rc2);
    } else {
        SDL_Color border = {(Uint8)std::max(col.r-50,0),(Uint8)std::max(col.g-50,0),(Uint8)std::max(col.b-50,0),255};
        SDL_SetRenderDrawColor(r, border.r,border.g,border.b,255);
        SDL_Rect rc = {x,y,w,h};
        SDL_RenderDrawRect(r, &rc);
    }
    // زبانه اتصال پایین (نشانه‌ی "next block")
    fillRect(r, x+10, y+h-2, 20, 5, col);
    fillRect(r, x+10, y+h+1, 20, 3, {(Uint8)std::max(col.r-30,0),(Uint8)std::max(col.g-30,0),(Uint8)std::max(col.b-30,0),255});
}

// رندر متن با TTF
static void renderText(SDL_Renderer* r, TTF_Font* font, const std::string& text,
                       int x, int y, SDL_Color col = {255,255,255,255}) {
    if (!font || text.empty()) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(r, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

// ─── ساختار UI State ──────────────────────────────────────────────────────────
struct UIState {
    std::string active_category = "Motion";

    // drag
    Block* dragging_block  = nullptr;
    int    drag_offset_x   = 0;
    int    drag_offset_y   = 0;
    bool   drag_from_palette = false; // آیا از پالت برداشته شده؟

    // snap highlight
    int    snap_target_id  = -1;      // id بلوکی که می‌شود به زیرش snap کرد
};

// ─── پیدا کردن بلوک با id ─────────────────────────────────────────────────────
static Block* findBlock(Project& p, int id) {
    for (auto& b : p.blocks) if (b.id == id) return &b;
    return nullptr;
}

// ─── snap logic: آیا بلوک dragging نزدیک یک بلوک دیگر است؟ ─────────────────
static int findSnapTarget(const Block& dragging, const Project& p) {
    for (const auto& b : p.blocks) {
        if (b.id == dragging.id) continue;
        // ناحیه snap: پایین بلوک b
        int snap_x = (int)b.x;
        int snap_y = (int)(b.y + b.height);
        int dx = (int)dragging.x - snap_x;
        int dy = (int)dragging.y - snap_y;
        if (std::abs(dx) < 40 && std::abs(dy) < 30)
            return b.id;
    }
    return -1;
}

// ─── تابع اصلی ───────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {

    // ── راه‌اندازی SDL ──
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Scratch C++ [Sharif Project]",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // ── بارگذاری فونت ──
    // مسیر فونت از CMakeLists.txt به صورت ماکرو تعریف شده (FONT_PATH_NORMAL / FONT_PATH_BOLD)
    // اگر فونت پیدا نشد، متن رندر نمی‌شود ولی برنامه crash نمی‌کند
#ifndef FONT_PATH_NORMAL
#define FONT_PATH_NORMAL "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#endif
#ifndef FONT_PATH_BOLD
#define FONT_PATH_BOLD   "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
#endif

    TTF_Font* font_normal = TTF_OpenFont(FONT_PATH_NORMAL, 13);
    TTF_Font* font_small  = TTF_OpenFont(FONT_PATH_NORMAL, 11);
    TTF_Font* font_bold   = TTF_OpenFont(FONT_PATH_BOLD,   14);

    if (!font_normal) {
        std::cerr << "[Font] Warning: Could not load font: " << TTF_GetError() << std::endl;
        std::cerr << "[Font] UI will render without text. Install fonts or update FONT_PATH in CMakeLists." << std::endl;
    }

    // ── پروژه و Runtime ──
    Project project;
    Runtime rt;
    runtime_init(&rt, &project);

    int next_block_id = 100;

    // ── تابع کمکی: ساخت پروژه پیش‌فرض (sprite + when_start) ──
    auto setup_default_project = [&]() {
        project.sprites.clear();
        // Sprite1 پیش‌فرض
        Sprite spr;
        spr.id           = 1;
        spr.name         = "Sprite1";
        spr.x            = 0.0f;
        spr.y            = 0.0f;
        spr.direction    = 90.0f;
        spr.visible      = true;
        spr.width        = 48.0f;
        spr.height       = 48.0f;
        spr.size_percent = 100.0f;
        project.sprites.push_back(spr);
        // بلوک when_start پیش‌فرض
        Block b;
        b.id          = next_block_id++;
        b.type        = "when_start";
        b.x           = SIDEBAR_W + 40;
        b.y           = TOOLBAR_H + 40;
        b.nextBlockId = -1;
        b.width       = BLOCK_W;
        b.height      = BLOCK_H;
        project.blocks.push_back(b);
    };

    setup_default_project();

    UIState ui;
    bool running = true;
    SDL_Event event;

    // ── مناطق UI ──
    //  [TOOLBAR]         y=0..TOOLBAR_H
    //  [CAT | PAL | SCRIPT AREA | STAGE]   y=TOOLBAR_H..WINDOW_H
    SDL_Rect area_toolbar = {0, 0, WINDOW_W, TOOLBAR_H};
    SDL_Rect area_category= {0, TOOLBAR_H, CATEGORY_W, WINDOW_H - TOOLBAR_H};
    SDL_Rect area_palette = {CATEGORY_W, TOOLBAR_H, PALETTE_W, WINDOW_H - TOOLBAR_H};
    SDL_Rect area_script  = {SIDEBAR_W, TOOLBAR_H, WINDOW_W - SIDEBAR_W - STAGE_W, WINDOW_H - TOOLBAR_H};
    SDL_Rect area_stage   = {WINDOW_W - STAGE_W, TOOLBAR_H, STAGE_W, WINDOW_H - TOOLBAR_H};

    // دکمه‌های toolbar
    SDL_Rect btn_run  = {WINDOW_W/2 - 60, 8, 50, 34};
    SDL_Rect btn_stop = {WINDOW_W/2 - 5,  8, 50, 34};
    SDL_Rect btn_step = {WINDOW_W/2 + 50, 8, 50, 34};
    SDL_Rect btn_save = {20, 10, 60, 30};
    SDL_Rect btn_load = {90, 10, 60, 30};
    SDL_Rect btn_new  = {WINDOW_W - 80, 10, 60, 30};

    // ── حلقه اصلی ──────────────────────────────────────────────────────────
    while (running) {

        // ── پردازش رویدادها ──────────────────────────────────────────────
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            // ── Mouse Down ──
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;
                SDL_Point mp = {mx, my};

                // --- toolbar دکمه‌ها ---
                if (SDL_PointInRect(&mp, &btn_run)) {
                    runtime_init(&rt, &project);
                    for (auto& b : project.blocks)
                        if (b.type == "when_start") { rt.currentBlockId = b.id; break; }
                    runtime_start(&rt);
                    logInfo("Runtime started by user.");
                }
                else if (SDL_PointInRect(&mp, &btn_stop)) {
                    runtime_stop(&rt);
                    logInfo("Runtime stopped by user.");
                }
                else if (SDL_PointInRect(&mp, &btn_save)) {
                    saveProjectAndMark(project, "project.fop");
                    logInfo("Project saved.");
                }
                else if (SDL_PointInRect(&mp, &btn_load)) {
                    if (loadProject(project, "project.fop")) {
                        // اگر فایل قدیمی sprite نداشت، یه sprite پیش‌فرض اضافه کن
                        if (project.sprites.empty()) {
                            Sprite spr; spr.id=1; spr.name="Sprite1";
                            spr.x=0; spr.y=0; spr.direction=90;
                            spr.visible=true; spr.width=48; spr.height=48;
                            spr.size_percent=100;
                            project.sprites.push_back(spr);
                        }
                        runtime_init(&rt, &project);
                        logInfo("Project loaded.");
                    }
                }
                else if (SDL_PointInRect(&mp, &btn_new)) {
                    newProject(project);
                    next_block_id = 100;
                    setup_default_project();
                    runtime_init(&rt, &project);
                    logInfo("New project created.");
                }

                // --- کلیک روی دسته‌بندی ---
                else if (SDL_PointInRect(&mp, &area_category)) {
                    int rel_y = my - area_category.y;
                    int cat_h = area_category.h / (int)CATEGORIES.size();
                    int idx = rel_y / cat_h;
                    if (idx >= 0 && idx < (int)CATEGORIES.size())
                        ui.active_category = CATEGORIES[idx].name;
                }

                // --- کلیک روی پالت: ساخت بلوک جدید ---
                else if (SDL_PointInRect(&mp, &area_palette)) {
                    int rel_y = my - area_palette.y - 10;
                    int idx = rel_y / (BLOCK_H + 8);

                    // فیلتر بر اساس دسته فعال
                    std::vector<const PaletteEntry*> visible;
                    for (auto& e : PALETTE_ENTRIES)
                        if (e.category == ui.active_category) visible.push_back(&e);

                    if (idx >= 0 && idx < (int)visible.size()) {
                        const PaletteEntry* pe = visible[idx];
                        Block nb;
                        nb.id      = next_block_id++;
                        nb.type    = pe->type;
                        nb.inputs  = pe->default_inputs;
                        nb.nextBlockId = -1;
                        nb.x       = (float)(mx - BLOCK_W/2);
                        nb.y       = (float)(my - BLOCK_H/2);
                        nb.width   = BLOCK_W;
                        nb.height  = BLOCK_H;
                        project.blocks.push_back(nb);
                        ui.dragging_block = &project.blocks.back();
                        ui.drag_offset_x  = BLOCK_W/2;
                        ui.drag_offset_y  = BLOCK_H/2;
                        ui.drag_from_palette = true;
                    }
                }

                // --- کلیک روی script area: drag بلوک موجود ---
                else if (SDL_PointInRect(&mp, &area_script)) {
                    // جستجو از آخر (بلوک‌های رویی اول)
                    for (int i = (int)project.blocks.size()-1; i >= 0; --i) {
                        Block& b = project.blocks[i];
                        SDL_Rect br = {(int)b.x, (int)b.y, (int)b.width, (int)b.height};
                        if (SDL_PointInRect(&mp, &br)) {
                            // قطع اتصال از والد
                            for (auto& pb : project.blocks)
                                if (pb.nextBlockId == b.id) pb.nextBlockId = -1;
                            ui.dragging_block  = &b;
                            ui.drag_offset_x   = mx - (int)b.x;
                            ui.drag_offset_y   = my - (int)b.y;
                            ui.drag_from_palette = false;
                            break;
                        }
                    }
                }
            }

            // ── Mouse Motion ──
            if (event.type == SDL_MOUSEMOTION && ui.dragging_block) {
                ui.dragging_block->x = (float)(event.motion.x - ui.drag_offset_x);
                ui.dragging_block->y = (float)(event.motion.y - ui.drag_offset_y);
                ui.snap_target_id = findSnapTarget(*ui.dragging_block, project);
            }

            // ── Mouse Up ──
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                if (ui.dragging_block) {
                    int mx = event.button.x;
                    int my = event.button.y;

                    // اگر بلوک به ناحیه پالت برده شد → حذف
                    SDL_Point mp = {mx, my};
                    if (SDL_PointInRect(&mp, &area_palette) || SDL_PointInRect(&mp, &area_category)) {
                        int del_id = ui.dragging_block->id;
                        project.blocks.erase(
                            std::remove_if(project.blocks.begin(), project.blocks.end(),
                                [del_id](const Block& b){ return b.id == del_id; }),
                            project.blocks.end()
                        );
                    }
                    // snap به بلوک هدف
                    else if (ui.snap_target_id != -1) {
                        Block* target = findBlock(project, ui.snap_target_id);
                        if (target) {
                            // بلوک dragging را دقیقاً زیر target قرار بده
                            ui.dragging_block->x = target->x;
                            ui.dragging_block->y = target->y + target->height + 2;
                            // اتصال
                            int old_next = target->nextBlockId;
                            target->nextBlockId = ui.dragging_block->id;
                            ui.dragging_block->nextBlockId = old_next;
                        }
                    }
                    // اگر در script area رها شد، همانجا بماند
                }
                ui.dragging_block    = nullptr;
                ui.snap_target_id    = -1;
                ui.drag_from_palette = false;
            }

            // ── Keyboard ──
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_F5: // Run
                        runtime_init(&rt, &project);
                        for (auto& b : project.blocks)
                            if (b.type == "when_start") { rt.currentBlockId = b.id; break; }
                        runtime_start(&rt);
                        break;
                    case SDLK_F6: // Stop
                        runtime_stop(&rt);
                        break;
                    case SDLK_s:  // Save (Ctrl+S ساده‌شده)
                        saveProjectAndMark(project, "project.fop");
                        break;
                    case SDLK_DELETE: {
                        // حذف بلوک انتخاب‌شده (بلوکی که ماوس روش بود)
                        // در این نسخه فقط برای نشان دادن قابلیت
                        break;
                    }
                }
            }
        }

        // ── اجرای runtime ──
        if (runtime_isRunning(&rt))
            runtime_tick(&rt);

        // ─────────────────────────────────────────────────────────────────
        // ── رندر ─────────────────────────────────────────────────────────
        // ─────────────────────────────────────────────────────────────────

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);

        // ── 1) TOOLBAR ────────────────────────────────────────────────────
        fillRect(renderer, 0, 0, WINDOW_W, TOOLBAR_H, {40, 40, 40, 255});
        // خط جداکننده پایین toolbar
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
        SDL_RenderDrawLine(renderer, 0, TOOLBAR_H-1, WINDOW_W, TOOLBAR_H-1);

        // نام پروژه
        if (font_bold) renderText(renderer, font_bold, "Scratch C++  |  Sharif Project",
            10, 15, {200,200,200,255});

        // دکمه Run (سبز)
        {
            bool running_state = runtime_isRunning(&rt);
            SDL_Color c_run = running_state ? SDL_Color{60,180,60,255} : SDL_Color{50,160,50,255};
            fillRect(renderer, btn_run.x, btn_run.y, btn_run.w, btn_run.h, c_run);
            drawRect(renderer, btn_run.x, btn_run.y, btn_run.w, btn_run.h, {30,120,30,255});
            if (font_normal) renderText(renderer, font_normal, "▶ Run",
                btn_run.x+6, btn_run.y+9, {255,255,255,255});
        }
        // دکمه Stop (قرمز)
        {
            fillRect(renderer, btn_stop.x, btn_stop.y, btn_stop.w, btn_stop.h, {180,50,50,255});
            drawRect(renderer, btn_stop.x, btn_stop.y, btn_stop.w, btn_stop.h, {120,30,30,255});
            if (font_normal) renderText(renderer, font_normal, "■ Stop",
                btn_stop.x+4, btn_stop.y+9, {255,255,255,255});
        }
        // دکمه Step (آبی)
        {
            fillRect(renderer, btn_step.x, btn_step.y, btn_step.w, btn_step.h, {50,100,200,255});
            drawRect(renderer, btn_step.x, btn_step.y, btn_step.w, btn_step.h, {30,70,150,255});
            if (font_normal) renderText(renderer, font_normal, "▶| Step",
                btn_step.x+4, btn_step.y+9, {255,255,255,255});
        }
        // Save/Load/New
        {
            fillRect(renderer, btn_save.x, btn_save.y, btn_save.w, btn_save.h, {60,120,60,255});
            if (font_small) renderText(renderer, font_small, "💾 Save",
                btn_save.x+6, btn_save.y+8, {255,255,255,255});

            fillRect(renderer, btn_load.x, btn_load.y, btn_load.w, btn_load.h, {60,90,160,255});
            if (font_small) renderText(renderer, font_small, "📂 Load",
                btn_load.x+6, btn_load.y+8, {255,255,255,255});

            fillRect(renderer, btn_new.x, btn_new.y, btn_new.w, btn_new.h, {100,60,60,255});
            if (font_small) renderText(renderer, font_small, "🗋 New",
                btn_new.x+8, btn_new.y+8, {255,255,255,255});
        }

        // ── 2) CATEGORY SIDEBAR ───────────────────────────────────────────
        fillRect(renderer, area_category.x, area_category.y,
                 area_category.w, area_category.h, {30, 30, 30, 255});

        int cat_h = area_category.h / (int)CATEGORIES.size();
        for (int i = 0; i < (int)CATEGORIES.size(); ++i) {
            const auto& cat = CATEGORIES[i];
            int cy = area_category.y + i * cat_h;
            bool active = (cat.name == ui.active_category);

            //배경
            SDL_Color bg = active ? cat.color : SDL_Color{45,45,45,255};
            fillRect(renderer, area_category.x, cy, area_category.w, cat_h-1, bg);

            // متن (اول حرف)
            if (font_small) {
                std::string short_name = cat.name.substr(0, 2);
                renderText(renderer, font_small, short_name,
                    area_category.x + 4, cy + cat_h/2 - 7,
                    active ? SDL_Color{255,255,255,255} : cat.color);
            }
            // نقطه رنگی
            fillRect(renderer, area_category.x + CATEGORY_W-8, cy+cat_h/2-4, 6, 8, cat.color);
        }

        // ── 3) PALETTE ────────────────────────────────────────────────────
        fillRect(renderer, area_palette.x, area_palette.y,
                 area_palette.w, area_palette.h, {38, 38, 38, 255});

        // عنوان دسته
        if (font_bold) {
            SDL_Color cat_col = colorForCategory(ui.active_category);
            fillRect(renderer, area_palette.x, area_palette.y, area_palette.w, 28, {30,30,30,255});
            renderText(renderer, font_bold, ui.active_category,
                area_palette.x+8, area_palette.y+7, cat_col);
        }

        {
            std::vector<const PaletteEntry*> visible;
            for (auto& e : PALETTE_ENTRIES)
                if (e.category == ui.active_category) visible.push_back(&e);

            int py = area_palette.y + 36;
            for (auto* pe : visible) {
                SDL_Color col = colorForCategory(pe->category);
                renderBlock(renderer, area_palette.x+6, py, PALETTE_W-12, BLOCK_H-4, col);
                if (font_small)
                    renderText(renderer, font_small, pe->label,
                        area_palette.x+14, py + BLOCK_H/2 - 8, {255,255,255,255});
                py += BLOCK_H + 4;
                if (py > area_palette.y + area_palette.h - BLOCK_H) break;
            }
        }

        // ── 4) SCRIPT AREA ────────────────────────────────────────────────
        // پس‌زمینه grid
        fillRect(renderer, area_script.x, area_script.y,
                 area_script.w, area_script.h, {22, 22, 28, 255});
        // نقطه‌های grid
        SDL_SetRenderDrawColor(renderer, 40, 40, 50, 255);
        for (int gx = area_script.x; gx < area_script.x + area_script.w; gx += 24)
            for (int gy = area_script.y; gy < area_script.y + area_script.h; gy += 24)
                SDL_RenderDrawPoint(renderer, gx, gy);

        // خطوط اتصال بین بلوک‌ها
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 180);
        for (auto& b : project.blocks) {
            if (b.nextBlockId != -1) {
                Block* nb = findBlock(project, b.nextBlockId);
                if (nb) {
                    int x1 = (int)b.x  + (int)b.width/2;
                    int y1 = (int)b.y  + (int)b.height;
                    int x2 = (int)nb->x + (int)nb->width/2;
                    int y2 = (int)nb->y;
                    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
                }
            }
        }

        // رندر بلوک‌های script (به جز dragging که آخر رندر می‌شود)
        for (auto& b : project.blocks) {
            if (ui.dragging_block && b.id == ui.dragging_block->id) continue;

            bool highlighted = (b.id == rt.lastExecutedBlockId);
            bool snap_target = (b.id == ui.snap_target_id);

            SDL_Color col = colorForBlockType(b.type);
            // رنگ روشن‌تر برای snap target
            if (snap_target) {
                col.r = std::min(col.r+50, 255);
                col.g = std::min(col.g+50, 255);
                col.b = std::min(col.b+50, 255);
            }

            renderBlock(renderer, (int)b.x, (int)b.y, (int)b.width, (int)b.height,
                        col, highlighted);

            if (font_small) {
                std::string lbl = labelForBlockType(b.type);
                renderText(renderer, font_small, lbl,
                    (int)b.x+10, (int)b.y + BLOCK_H/2 - 8, {255,255,255,255});
            }
        }

        // رندر بلوک در حال drag (روی همه چیز)
        if (ui.dragging_block) {
            Block& d = *ui.dragging_block;
            SDL_Color col = colorForBlockType(d.type);
            // کمی شفاف‌تر
            col.a = 200;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            renderBlock(renderer, (int)d.x, (int)d.y, (int)d.width, (int)d.height, col);
            if (font_small) {
                std::string lbl = labelForBlockType(d.type);
                renderText(renderer, font_small, lbl,
                    (int)d.x+10, (int)d.y + BLOCK_H/2 - 8, {255,255,255,255});
            }
        }

        // ── 5) STAGE ──────────────────────────────────────────────────────
        fillRect(renderer, area_stage.x, area_stage.y,
                 area_stage.w, area_stage.h, {50, 50, 55, 255});

        // Stage (صفحه نمایش sprite)
        int stage_x = area_stage.x + 10;
        int stage_y = area_stage.y + 10;
        fillRect(renderer, stage_x, stage_y, STAGE_W-20, STAGE_H, {255,255,255,255});
        drawRect(renderer, stage_x, stage_y, STAGE_W-20, STAGE_H, {100,100,100,255});

        // رندر spriteها
        // مختصات Scratch: x=0,y=0 مرکز stage. x از -240 تا +240، y از -180 تا +180
        // مختصات Stage روی صفحه: stage_x..stage_x+STAGE_W-20
        int stage_w = STAGE_W - 20;
        int stage_h = STAGE_H;
        for (auto& spr : project.sprites) {
            if (!spr.visible) continue;
            // تبدیل مختصات Scratch به pixel
            int sx = stage_x + stage_w/2 + (int)(spr.x * stage_w / 480.0f);
            int sy = stage_y + stage_h/2 - (int)(spr.y * stage_h / 360.0f);
            // اندازه sprite با size_percent
            int sw = (int)(spr.width  * spr.size_percent / 100.0f);
            int sh = (int)(spr.height * spr.size_percent / 100.0f);
            sw = std::max(sw, 8); sh = std::max(sh, 8);
            // clamp داخل stage
            sx = std::max(stage_x + sw/2, std::min(sx, stage_x + stage_w - sw/2));
            sy = std::max(stage_y + sh/2, std::min(sy, stage_y + stage_h - sh/2));
            // رندر sprite (مستطیل نارنجی)
            fillRect(renderer, sx-sw/2, sy-sh/2, sw, sh, {255, 165, 0, 220});
            drawRect(renderer, sx-sw/2, sy-sh/2, sw, sh, {200, 100, 0, 255});
            // نام sprite زیرش
            if (font_small) renderText(renderer, font_small, spr.name,
                sx - sw/2, sy + sh/2 + 2, {40, 40, 40, 255});
        }

        // Sprite info پایین stage (live x,y,direction)
        {
            int info_y = area_stage.y + STAGE_H + 20;
            if (font_small) {
                std::string spr_name = project.sprites.empty() ? "Sprite1" : project.sprites[0].name;
                float x_val  = project.sprites.empty() ? 0.0f : project.sprites[0].x;
                float y_val  = project.sprites.empty() ? 0.0f : project.sprites[0].y;
                float d_val  = project.sprites.empty() ? 90.0f: project.sprites[0].direction;

                renderText(renderer, font_small, "Sprite: " + spr_name,
                    area_stage.x+10, info_y, {200,200,200,255});
                std::string xy  = "x:" + std::to_string((int)x_val) + "  y:" + std::to_string((int)y_val);
                std::string dir = "dir:" + std::to_string((int)d_val);
                renderText(renderer, font_small, xy,  area_stage.x+10, info_y+18, {160,220,160,255});
                renderText(renderer, font_small, dir, area_stage.x+10, info_y+34, {160,160,220,255});
            }
        }

        // وضعیت runtime
        {
            std::string status;
            if (runtime_isRunning(&rt))       status = "● Running";
            else if (runtime_isPaused(&rt))   status = "⏸ Paused";
            else                              status = "■ Stopped";

            SDL_Color sc = runtime_isRunning(&rt) ? SDL_Color{80,220,80,255}
                         : runtime_isPaused(&rt)  ? SDL_Color{220,220,80,255}
                                                   : SDL_Color{180,80,80,255};

            if (font_small) renderText(renderer, font_small, status,
                area_stage.x+10, area_stage.y + STAGE_H + 60, sc);
        }

        // راهنمای کلیدها
        if (font_small) {
            renderText(renderer, font_small, "F5: Run  |  F6: Stop  |  S: Save",
                area_stage.x+8, WINDOW_H - 30, {100,100,100,255});
        }

        // ── پایان رندر ──
        SDL_RenderPresent(renderer);

    } // end while(running)

    // ── پاک‌سازی ──
    if (font_normal) TTF_CloseFont(font_normal);
    if (font_small)  TTF_CloseFont(font_small);
    if (font_bold)   TTF_CloseFont(font_bold);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
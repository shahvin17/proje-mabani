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
#include <SDL2/SDL_image.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "core_types.h"
#include "runtime.h"
#include "render.h"
#include "persist.h"
#include "logger.h"
#include <cctype>   // برای std::isdigit

// ─── ثابت‌های ابعاد UI ───────────────────────────────────────────────────────
// ابعاد پنجره — در main() بعد از fullscreen آپدیت می‌شوند
static int WINDOW_W      = 1280;
static int WINDOW_H      = 720;
static const int TOOLBAR_H     = 50;
static const int SIDEBAR_W     = 220;  // پانل دسته‌بندی + پالت
static const int CATEGORY_W    = 60;   // ستون آیکون دسته‌بندی
static const int PALETTE_W     = SIDEBAR_W - CATEGORY_W;
static const int STAGE_W         = 320;
static const int STAGE_H         = 240;
static const int BLOCK_H         = 44;
static const int BLOCK_W         = 150;
static const int SPRITE_PANEL_H  = 110;  // ارتفاع پانل sprite در زیر stage
static const int SPRITE_THUMB_W  = 70;
static const int SPRITE_THUMB_H  = 70;

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
    // Operators — 13 اجباری + 8 جبرانی
    {"op_add",          "( ) + ( )",              {0,0},   "Operators"},
    {"op_sub",          "( ) - ( )",              {0,0},   "Operators"},
    {"op_mul",          "( ) * ( )",              {0,0},   "Operators"},
    {"op_div",          "( ) / ( )",              {0,0},   "Operators"},
    {"op_mod",          "( ) mod ( )",            {0,0},   "Operators"},
    {"op_gt",           "( ) > ( )",              {0,0},   "Operators"},
    {"op_lt",           "( ) < ( )",              {0,0},   "Operators"},
    {"op_eq",           "( ) = ( )",              {0,0},   "Operators"},
    {"op_and",          "( ) and ( )",            {0,0},   "Operators"},
    {"op_or",           "( ) or ( )",             {0,0},   "Operators"},
    {"op_not",          "not ( )",                {0},     "Operators"},
    {"op_join",         "join ( ) ( )",           {0,0},   "Operators"},
    {"op_abs",          "abs ( )",                {0},     "Operators"},
    {"op_floor",        "floor ( )",              {0},     "Operators"},
    {"op_ceil",         "ceiling ( )",            {0},     "Operators"},
    {"op_sqrt",         "sqrt ( )",               {0},     "Operators"},
    {"op_sin",          "sin ( )",                {0},     "Operators"},
    {"op_cos",          "cos ( )",                {0},     "Operators"},
    {"op_round",        "round ( )",              {0},     "Operators"},
    {"op_xor",          "( ) xor ( )",            {0,0},   "Operators"},
    {"op_random",       "pick random ( ) to ( )", {1,10},  "Operators"},
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

// label کامل (با عدد) — برای پالت
std::string labelForBlockType(const std::string& type) {
    for (auto& e : PALETTE_ENTRIES)
        if (e.type == type) return e.label;
    return type;
}

// label بدون عدد — برای بلوک‌های script که input field دارن
std::string labelNoNumbers(const std::string& type) {
    static const std::map<std::string,std::string> clean = {
        {"move",         "move"},
        {"turn_right",   "turn right"},
        {"turn_left",    "turn left"},
        {"goto_xy",      "go to x:  y:"},
        {"set_x",        "set x to"},
        {"set_y",        "set y to"},
        {"change_x",     "change x by"},
        {"change_y",     "change y by"},
        {"bounce",       "if on edge, bounce"},
        {"say",          "say Hello!"},
        {"say_for",      "say Hello! for"},
        {"show",         "show"},
        {"hide",         "hide"},
        {"set_size",     "set size to"},
        {"change_size",  "change size by"},
        {"wait",         "wait"},
        {"repeat",       "repeat"},
        {"when_start",   "when ð© clicked"},
        {"when_key",     "when space pressed"},
        {"when_clicked", "when sprite clicked"},
        {"forever",      "forever"},
        {"if_block",     "if <> then"},
        {"if_else",      "if <> else"},
        {"stop_all",     "stop all"},
        {"set_var",      "set var to"},
        {"change_var",   "change var by"},
        {"show_var",     "show variable"},
        // Operators
        {"op_add",       "( ) + ( )"},
        {"op_sub",       "( ) - ( )"},
        {"op_mul",       "( ) * ( )"},
        {"op_div",       "( ) / ( )"},
        {"op_mod",       "( ) mod ( )"},
        {"op_gt",        "( ) > ( )"},
        {"op_lt",        "( ) < ( )"},
        {"op_eq",        "( ) = ( )"},
        {"op_and",       "( ) and ( )"},
        {"op_or",        "( ) or ( )"},
        {"op_not",       "not ( )"},
        {"op_join",      "join ( ) ( )"},
        {"op_abs",       "abs ( )"},
        {"op_floor",     "floor ( )"},
        {"op_ceil",      "ceiling ( )"},
        {"op_sqrt",      "sqrt ( )"},
        {"op_sin",       "sin ( )"},
        {"op_cos",       "cos ( )"},
        {"op_round",     "round ( )"},
        {"op_xor",       "( ) xor ( )"},
        {"op_random",    "random ( ) to ( )"},
    };
    auto it = clean.find(type);
    if (it != clean.end()) return it->second;
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

// ─── Texture Cache ───────────────────────────────────────────────────────────
// برای جلوگیری از لود مکرر تصاویر، هر مسیر یک بار لود و کش می‌شود
static std::unordered_map<std::string, SDL_Texture*> g_texture_cache;
static SDL_Renderer* g_renderer_ref = nullptr;
std::string g_assets_path = "./assets/images/";  // آپدیت می‌شه در main()

SDL_Texture* loadTexture(const std::string& path) {
    if (path.empty()) return nullptr;
    auto it = g_texture_cache.find(path);
    if (it != g_texture_cache.end()) return it->second;

    SDL_Texture* tex = IMG_LoadTexture(g_renderer_ref, path.c_str());
    if (!tex) {
        std::cerr << "[IMG] Failed to load: " << path << " — " << IMG_GetError() << std::endl;
        g_texture_cache[path] = nullptr;
        return nullptr;
    }
    g_texture_cache[path] = tex;
    std::cout << "[IMG] Loaded: " << path << std::endl;
    return tex;
}

void freeTextureCache() {
    for (auto& kv : g_texture_cache)
        if (kv.second) SDL_DestroyTexture(kv.second);
    g_texture_cache.clear();
}

// رندر تکسچر با حفظ نسبت ابعاد
static void renderTextureFit(SDL_Renderer* r, SDL_Texture* tex, int x, int y, int w, int h) {
    if (!tex) return;
    int tw, th;
    SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
    // حفظ نسبت ابعاد
    float scale = std::min((float)w/tw, (float)h/th);
    int dw = (int)(tw * scale);
    int dh = (int)(th * scale);
    int dx = x + (w - dw) / 2;
    int dy = y + (h - dh) / 2;
    SDL_Rect dst = {dx, dy, dw, dh};
    SDL_RenderCopy(r, tex, nullptr, &dst);
}

// ─── ساختار UI State ──────────────────────────────────────────────────────────
struct UIState {
    std::string active_category = "Motion";

    // drag
    Block* dragging_block  = nullptr;
    int    drag_offset_x   = 0;
    int    drag_offset_y   = 0;
    bool   drag_from_palette = false;

    // snap highlight
    int    snap_target_id  = -1;

    // ورودی متنی برای بلوک‌ها
    int    editing_block_id  = -1;
    int    editing_input_idx = 0;
    std::string input_buffer = "";

    // sprite panel
    int    active_sprite_id  = 1;    // id sprite انتخاب‌شده فعلی
    bool   show_sprite_panel = true;
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

    // SDL_image برای بارگذاری PNG
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "[IMG] Warning: " << IMG_GetError() << " — images will not load." << std::endl;
    }

    // مسیر پایه assets نسبت به executable
    // SDL_GetBasePath برمیگردونه مثلاً /home/user/project/build/
    {
        char* base = SDL_GetBasePath();
        std::string base_str = base ? std::string(base) : "./";
        SDL_free(base);
        // اگر assets/images در کنار executable نبود، یه سطح بالاتر چک کن
        // چون ممکنه از build/ اجرا بشه ولی assets در ریشه پروژه باشه
        auto file_exists = [](const std::string& p) -> bool {
            FILE* f = fopen(p.c_str(), "rb");
            if (f) { fclose(f); return true; }
            return false;
        };

        // پیدا کردن مسیر درست assets
        std::vector<std::string> candidates = {
            "./assets/images/",
            base_str + "assets/images/",
            base_str + "../assets/images/",
            base_str + "../../assets/images/",
        };
        g_assets_path = "./assets/images/";  // default
        for (auto& c : candidates) {
            if (file_exists(c + "background.png") ||
                file_exists(c + "sprite1.png")) {
                g_assets_path = c;
                std::cout << "[Assets] Found at: " << c << std::endl;
                break;
            }
        }
        if (g_assets_path == "./assets/images/")
            std::cout << "[Assets] Warning: images not found, using default path." << std::endl;
    }

    // fullscreen: ابعاد واقعی صفحه رو بگیر
    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);
    // اگر صفحه کوچیکه از WINDOW_W، fullscreen desktop بزن
    // وگرنه همان ابعاد ثابت
    bool use_fullscreen = (dm.w > 0 && dm.h > 0);
    int actual_w = use_fullscreen ? dm.w : WINDOW_W;
    int actual_h = use_fullscreen ? dm.h : WINDOW_H;

    SDL_Window* window = SDL_CreateWindow(
        "Scratch C++ [Sharif Project]",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        actual_w, actual_h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    // fullscreen borderless
    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    // ابعاد واقعی بعد از fullscreen
    SDL_GetWindowSize(window, &actual_w, &actual_h);
    // آپدیت ابعاد سراسری
    WINDOW_W = actual_w;
    WINDOW_H = actual_h;
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    g_renderer_ref = renderer;  // برای texture cache

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

    // ── لود تصاویر ──
    // background
    SDL_Texture* bg_texture = loadTexture(g_assets_path + "background.png");

    // تخصیص costume پیش‌فرض به sprite ها بر اساس ترتیب
    {
        std::vector<std::string> default_costumes = {
            g_assets_path + "sprite1.png",
            g_assets_path + "sprite2.png",
            g_assets_path + "sprite3.png",
        };
        for (int i = 0; i < (int)project.sprites.size(); ++i) {
            if (project.sprites[i].costume_path.empty()) {
                int ci = i % 3;
                project.sprites[i].costume_path = default_costumes[ci];
            }
        }
    }

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
    SDL_Rect area_stage   = {WINDOW_W - STAGE_W, TOOLBAR_H, STAGE_W, WINDOW_H - TOOLBAR_H - SPRITE_PANEL_H};

    // دکمه‌های toolbar
    // وسط: Run / Stop / Step
    SDL_Rect btn_run  = {WINDOW_W/2 - 60, 8, 52, 34};
    SDL_Rect btn_stop = {WINDOW_W/2 - 4,  8, 52, 34};
    SDL_Rect btn_step = {WINDOW_W/2 + 52, 8, 52, 34};
    // راست: New / Save / Load
    SDL_Rect btn_new  = {WINDOW_W - 195, 8, 55, 34};
    SDL_Rect btn_save = {WINDOW_W - 135, 8, 60, 34};
    SDL_Rect btn_load = {WINDOW_W - 70,  8, 60, 34};

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

                // محاسبه ناحیه sprite panel
                SDL_Rect add_btn_rect  = {WINDOW_W - 44, WINDOW_H - SPRITE_PANEL_H + 8, 34, 30};
                SDL_Rect sprite_panel  = {WINDOW_W - STAGE_W, WINDOW_H - SPRITE_PANEL_H, STAGE_W, SPRITE_PANEL_H};
                int panel_y_s = WINDOW_H - SPRITE_PANEL_H;
                SDL_Rect vis_btn = {WINDOW_W - 90, panel_y_s + SPRITE_THUMB_H + 10, 38, 20};
                SDL_Rect del_btn = {WINDOW_W - 46, panel_y_s + SPRITE_THUMB_H + 10, 38, 20};

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
                    ui.active_sprite_id = project.sprites.empty() ? -1 : project.sprites[0].id;
                    logInfo("New project created.");
                }
                // --- دکمه + اضافه کردن sprite ---
                else if (SDL_PointInRect(&mp, &add_btn_rect)) {
                    Sprite spr;
                    spr.id           = (int)project.sprites.size() + 1;
                    spr.name         = "Sprite" + std::to_string(spr.id);
                    spr.x = 0; spr.y = 0; spr.direction = 90;
                    spr.visible = true; spr.width = 48; spr.height = 48;
                    spr.size_percent = 100;
                    // تخصیص costume بر اساس شماره sprite
                    {
                        std::vector<std::string> costumes = {
                            g_assets_path + "sprite1.png",
                            g_assets_path + "sprite2.png",
                            g_assets_path + "sprite3.png",
                        };
                        spr.costume_path = costumes[(spr.id - 1) % 3];
                    }
                    project.sprites.push_back(spr);
                    ui.active_sprite_id = spr.id;
                    logInfo("Added sprite: " + spr.name);
                }
                // --- دکمه Show/Hide sprite ---
                else if (SDL_PointInRect(&mp, &vis_btn)) {
                    for (auto& s : project.sprites)
                        if (s.id == ui.active_sprite_id) { s.visible = !s.visible; break; }
                }
                // --- دکمه Delete sprite ---
                else if (SDL_PointInRect(&mp, &del_btn)) {
                    if (project.sprites.size() > 1) {
                        project.sprites.erase(
                            std::remove_if(project.sprites.begin(), project.sprites.end(),
                                [&ui](const Sprite& s){ return s.id == ui.active_sprite_id; }),
                            project.sprites.end());
                        ui.active_sprite_id = project.sprites[0].id;
                        logInfo("Sprite deleted.");
                    }
                }
                // --- کلیک روی thumbnail sprite panel ---
                else if (SDL_PointInRect(&mp, &sprite_panel)) {
                    int sp_start_x = WINDOW_W - STAGE_W + 8;
                    for (int si = 0; si < (int)project.sprites.size(); ++si) {
                        SDL_Rect thumb = {sp_start_x + si*(SPRITE_THUMB_W+8),
                                          panel_y_s + 8,
                                          SPRITE_THUMB_W, SPRITE_THUMB_H};
                        if (SDL_PointInRect(&mp, &thumb)) {
                            ui.active_sprite_id = project.sprites[si].id;
                            break;
                        }
                    }
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

                // --- کلیک روی script area: drag یا ویرایش input ---
                else if (SDL_PointInRect(&mp, &area_script)) {
                    bool clicked_input = false;
                    // اول چک کن آیا روی input field یه بلوک کلیک شده
                    for (int i = (int)project.blocks.size()-1; i >= 0; --i) {
                        Block& b = project.blocks[i];
                        // input fields در سمت راست بلوک قرار دارند
                        // هر input: یک باکس 36x22 در y مرکزی بلوک
                        int input_count = (int)b.inputs.size();
                        for (int k = 0; k < input_count && k < 3; ++k) {
                            int field_x = (int)b.x + (int)b.width - (input_count - k) * 42 - 4;
                            int field_y = (int)b.y + ((int)b.height - 22) / 2;
                            SDL_Rect field_r = {field_x, field_y, 38, 22};
                            if (SDL_PointInRect(&mp, &field_r)) {
                                // شروع ویرایش این input
                                ui.editing_block_id  = b.id;
                                ui.editing_input_idx = k;
                                ui.input_buffer      = std::to_string(b.inputs[k]);
                                SDL_StartTextInput();
                                clicked_input = true;
                                break;
                            }
                        }
                        if (clicked_input) break;
                    }
                    if (!clicked_input) {
                        // اگر روی input نزدیم، drag را شروع کن
                        // ابتدا اگر داریم ویرایش می‌کردیم، اعمال کن
                        if (ui.editing_block_id != -1) {
                            Block* eb = findBlock(project, ui.editing_block_id);
                            if (eb && ui.editing_input_idx < (int)eb->inputs.size()) {
                                try { eb->inputs[ui.editing_input_idx] = std::stoi(ui.input_buffer); }
                                catch (...) {}
                            }
                            ui.editing_block_id = -1;
                            SDL_StopTextInput();
                        }
                        for (int i = (int)project.blocks.size()-1; i >= 0; --i) {
                            Block& b = project.blocks[i];
                            SDL_Rect br = {(int)b.x, (int)b.y, (int)b.width, (int)b.height};
                            if (SDL_PointInRect(&mp, &br)) {
                                for (auto& pb : project.blocks)
                                    if (pb.nextBlockId == b.id) pb.nextBlockId = -1;
                                ui.dragging_block    = &b;
                                ui.drag_offset_x     = mx - (int)b.x;
                                ui.drag_offset_y     = my - (int)b.y;
                                ui.drag_from_palette = false;
                                break;
                            }
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
                // اگر در حال ویرایش input هستیم
                if (ui.editing_block_id != -1) {
                    if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER) {
                        // اعمال مقدار و بستن
                        Block* eb = findBlock(project, ui.editing_block_id);
                        if (eb && ui.editing_input_idx < (int)eb->inputs.size()) {
                            try { eb->inputs[ui.editing_input_idx] = std::stoi(ui.input_buffer); }
                            catch (...) { /* ورودی نامعتبر، نادیده بگیر */ }
                        }
                        ui.editing_block_id = -1;
                        SDL_StopTextInput();
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        // کنسل ویرایش
                        ui.editing_block_id = -1;
                        SDL_StopTextInput();
                    }
                    else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                        if (!ui.input_buffer.empty())
                            ui.input_buffer.pop_back();
                    }
                    else if (event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_KP_MINUS) {
                        // اجازه وارد کردن عدد منفی
                        if (ui.input_buffer.empty()) ui.input_buffer = "-";
                    }
                } else {
                    // کلیدهای عادی (بدون حالت ویرایش)
                    switch (event.key.keysym.sym) {
                        case SDLK_F5:
                            runtime_init(&rt, &project);
                            for (auto& b : project.blocks)
                                if (b.type == "when_start") { rt.currentBlockId = b.id; break; }
                            runtime_start(&rt);
                            break;
                        case SDLK_F6:
                            runtime_stop(&rt);
                            break;
                        case SDLK_s:
                            saveProjectAndMark(project, "project.fop");
                            break;
                    }
                }
            }
            // ── Text Input (حروف و اعداد وارد شده) ──
            if (event.type == SDL_TEXTINPUT && ui.editing_block_id != -1) {
                // فقط ارقام و نقطه اعشار
                std::string incoming = event.text.text;
                for (char c : incoming) {
                    if (std::isdigit(c) || (c == '.' && ui.input_buffer.find('.') == std::string::npos))
                        ui.input_buffer += c;
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
        // New / Save / Load (سمت راست toolbar)
        {
            fillRect(renderer, btn_new.x,  btn_new.y,  btn_new.w,  btn_new.h,  {80,55,55,255});
            drawRect(renderer, btn_new.x,  btn_new.y,  btn_new.w,  btn_new.h,  {60,35,35,255});
            if (font_small) renderText(renderer, font_small, "New",
                btn_new.x+16, btn_new.y+10, {220,220,220,255});

            fillRect(renderer, btn_save.x, btn_save.y, btn_save.w, btn_save.h, {50,110,50,255});
            drawRect(renderer, btn_save.x, btn_save.y, btn_save.w, btn_save.h, {30,80,30,255});
            if (font_small) renderText(renderer, font_small, "Save",
                btn_save.x+14, btn_save.y+10, {220,255,220,255});

            fillRect(renderer, btn_load.x, btn_load.y, btn_load.w, btn_load.h, {50,80,150,255});
            drawRect(renderer, btn_load.x, btn_load.y, btn_load.w, btn_load.h, {30,55,110,255});
            if (font_small) renderText(renderer, font_small, "Load",
                btn_load.x+14, btn_load.y+10, {220,220,255,255});
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
            if (snap_target) {
                col.r = std::min(col.r+50, 255);
                col.g = std::min(col.g+50, 255);
                col.b = std::min(col.b+50, 255);
            }

            renderBlock(renderer, (int)b.x, (int)b.y, (int)b.width, (int)b.height,
                        col, highlighted);

            // رندر label بدون عدد (اعداد در input field نشان داده می‌شوند)
            if (font_small) {
                std::string lbl = b.inputs.empty()
                    ? labelForBlockType(b.type)
                    : labelNoNumbers(b.type);
                renderText(renderer, font_small, lbl,
                    (int)b.x+8, (int)b.y + (int)b.height/2 - 7, {255,255,255,255});
            }

            // رندر input fields
            {
                int input_count = (int)b.inputs.size();
                for (int k = 0; k < input_count && k < 3; ++k) {
                    int field_x = (int)b.x + (int)b.width - (input_count - k) * 42 - 4;
                    int field_y = (int)b.y + ((int)b.height - 22) / 2;

                    bool is_editing = (ui.editing_block_id == b.id && ui.editing_input_idx == k);

                    // پس‌زمینه field
                    SDL_Color field_bg = is_editing ? SDL_Color{255,255,200,255}
                                                    : SDL_Color{255,255,255,180};
                    fillRect(renderer, field_x, field_y, 38, 22, field_bg);
                    // کادر
                    SDL_Color border_col = is_editing ? SDL_Color{255,120,0,255}
                                                      : SDL_Color{100,100,100,180};
                    drawRect(renderer, field_x, field_y, 38, 22, border_col);

                    // مقدار
                    std::string val_str = is_editing ? ui.input_buffer
                                                     : std::to_string(b.inputs[k]);
                    if (font_small)
                        renderText(renderer, font_small, val_str,
                            field_x+3, field_y+4, {20,20,20,255});

                    // cursor چشمک‌زن در حالت editing
                    if (is_editing) {
                        static Uint32 cursor_timer = 0;
                        cursor_timer++;
                        if ((cursor_timer / 30) % 2 == 0) {
                            int cur_x = field_x + 4 + (int)val_str.size() * 7;
                            fillRect(renderer, cur_x, field_y+3, 2, 16, {0,0,0,255});
                        }
                    }
                }
            }
        }

        // رندر بلوک در حال drag (روی همه چیز)
        if (ui.dragging_block) {
            Block& d = *ui.dragging_block;
            SDL_Color col = colorForBlockType(d.type);
            // کمی شفاف‌تر
            col.a = 200;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    g_renderer_ref = renderer;  // برای texture cache
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
        int stage_draw_w = STAGE_W - 20;
        // stage با نسبت ۴:۳
        int stage_draw_h = stage_draw_w * 3 / 4;
        // اگر background تصویر داره نمایش بده، وگرنه سفید
        if (bg_texture) {
            SDL_Rect bg_dst = {stage_x, stage_y, stage_draw_w, stage_draw_h};
            SDL_RenderCopy(renderer, bg_texture, nullptr, &bg_dst);
        } else {
            fillRect(renderer, stage_x, stage_y, stage_draw_w, stage_draw_h, {255,255,255,255});
        }
        drawRect(renderer, stage_x, stage_y, stage_draw_w, stage_draw_h, {100,100,100,255});

        // رندر spriteها
        // مختصات Scratch: x=0,y=0 مرکز stage. x از -240 تا +240، y از -180 تا +180
        // مختصات Stage روی صفحه: stage_x..stage_x+STAGE_W-20
        for (auto& spr : project.sprites) {
            if (!spr.visible) continue;
            int sx = stage_x + stage_draw_w/2 + (int)(spr.x * stage_draw_w / 480.0f);
            int sy = stage_y + stage_draw_h/2 - (int)(spr.y * stage_draw_h / 360.0f);
            // اندازه sprite با size_percent
            int sw = (int)(spr.width  * spr.size_percent / 100.0f);
            int sh = (int)(spr.height * spr.size_percent / 100.0f);
            sw = std::max(sw, 8); sh = std::max(sh, 8);
            // clamp داخل stage
            sx = std::max(stage_x + sw/2, std::min(sx, stage_x + stage_draw_w - sw/2));
            sy = std::max(stage_y + sh/2, std::min(sy, stage_y + stage_draw_h - sh/2));
            // رندر sprite با تصویر یا fallback نارنجی
            SDL_Texture* spr_tex = spr.costume_path.empty()
                ? nullptr : loadTexture(spr.costume_path);
            if (spr_tex) {
                SDL_Rect spr_dst = {sx - sw/2, sy - sh/2, sw, sh};
                SDL_RenderCopy(renderer, spr_tex, nullptr, &spr_dst);
            } else {
                fillRect(renderer, sx-sw/2, sy-sh/2, sw, sh, {255, 165, 0, 220});
                drawRect(renderer, sx-sw/2, sy-sh/2, sw, sh, {200, 100, 0, 255});
            }
            // نام sprite
            if (font_small) renderText(renderer, font_small, spr.name,
                sx - sw/2, sy + sh/2 + 2, {40, 40, 40, 255});
        }

        // Sprite info پایین stage (live x,y,direction)
        // info_y را از stage_y + stage_draw_h محاسبه می‌کنیم تا overlap نشه
        {
            int info_y = stage_y + stage_draw_h + 8;
            int line_h = 16;  // فاصله بین خطوط

            // پس‌زمینه ناحیه info
            fillRect(renderer, area_stage.x, info_y - 4,
                area_stage.w, line_h * 5 + 8, {45,45,50,255});

            if (font_small) {
                // sprite فعال رو نشون بده نه اولی
                Sprite* active_info = nullptr;
                for (auto& s : project.sprites)
                    if (s.id == ui.active_sprite_id) { active_info = &s; break; }
                if (!active_info && !project.sprites.empty())
                    active_info = &project.sprites[0];

                std::string spr_name = active_info ? active_info->name : "Sprite1";
                float x_val = active_info ? active_info->x : 0.0f;
                float y_val = active_info ? active_info->y : 0.0f;
                float d_val = active_info ? active_info->direction : 90.0f;

                renderText(renderer, font_small, "Sprite: " + spr_name,
                    area_stage.x+8, info_y, {200,200,200,255});
                renderText(renderer, font_small,
                    "x:" + std::to_string((int)x_val) + "  y:" + std::to_string((int)y_val),
                    area_stage.x+8, info_y + line_h, {160,220,160,255});
                renderText(renderer, font_small,
                    "dir:" + std::to_string((int)d_val),
                    area_stage.x+8, info_y + line_h*2, {160,160,220,255});

                // runtime status
                std::string status;
                SDL_Color sc;
                if (runtime_isRunning(&rt)) {
                    status = "● Running"; sc = {80,220,80,255};
                } else if (runtime_isPaused(&rt)) {
                    status = "⏸ Paused";  sc = {220,220,80,255};
                } else {
                    status = "■ Stopped"; sc = {180,80,80,255};
                }
                renderText(renderer, font_small, status,
                    area_stage.x+8, info_y + line_h*3, sc);
            }
        }

        // ── Sprite Panel زیر stage ──────────────────────────────────────────
        {
            int panel_y = WINDOW_H - SPRITE_PANEL_H;
            int panel_x = WINDOW_W - STAGE_W;

            // پس‌زمینه پانل
            fillRect(renderer, panel_x, panel_y, STAGE_W, SPRITE_PANEL_H, {35,35,40,255});
            SDL_SetRenderDrawColor(renderer, 60,60,70,255);
            SDL_RenderDrawLine(renderer, panel_x, panel_y, WINDOW_W, panel_y);

            // دکمه + برای اضافه کردن sprite
            SDL_Rect add_btn = {WINDOW_W - 44, panel_y + 8, 34, 30};
            fillRect(renderer, add_btn.x, add_btn.y, add_btn.w, add_btn.h, {60,160,60,255});
            drawRect(renderer, add_btn.x, add_btn.y, add_btn.w, add_btn.h, {40,120,40,255});
            if (font_bold) renderText(renderer, font_bold, "+",
                add_btn.x+11, add_btn.y+5, {255,255,255,255});

            // thumbnail هر sprite
            int tx = panel_x + 8;
            for (int si = 0; si < (int)project.sprites.size(); ++si) {
                auto& spr = project.sprites[si];
                bool active = (spr.id == ui.active_sprite_id);

                SDL_Rect thumb = {tx, panel_y + 8, SPRITE_THUMB_W, SPRITE_THUMB_H};

                // کادر انتخاب
                SDL_Color frame_col = active ? SDL_Color{80,150,255,255} : SDL_Color{80,80,90,255};
                fillRect(renderer, thumb.x-2, thumb.y-2, thumb.w+4, thumb.h+4, frame_col);

                // پس‌زمینه thumbnail
                fillRect(renderer, thumb.x, thumb.y, thumb.w, thumb.h, {255,255,255,240});

                // sprite thumbnail با تصویر یا fallback
                SDL_Texture* th_tex = spr.costume_path.empty()
                    ? nullptr : loadTexture(spr.costume_path);
                if (th_tex) {
                    SDL_Rect th_dst = {thumb.x+4, thumb.y+4, thumb.w-8, thumb.h-8};
                    SDL_RenderCopy(renderer, th_tex, nullptr, &th_dst);
                } else {
                    int sp_cx = thumb.x + thumb.w/2;
                    int sp_cy = thumb.y + thumb.h/2 - 6;
                    fillRect(renderer, sp_cx-14, sp_cy-14, 28, 28,
                        active ? SDL_Color{255,140,0,255} : SDL_Color{200,140,80,255});
                }

                // نام sprite زیر thumbnail
                if (font_small) {
                    renderText(renderer, font_small, spr.name,
                        thumb.x + 2, thumb.y + thumb.h + 1,
                        active ? SDL_Color{100,180,255,255} : SDL_Color{160,160,160,255});
                }

                tx += SPRITE_THUMB_W + 8;
                if (tx + SPRITE_THUMB_W > WINDOW_W - 50) break;
            }

            // اطلاعات sprite انتخاب‌شده
            Sprite* active_spr = nullptr;
            for (auto& s : project.sprites)
                if (s.id == ui.active_sprite_id) { active_spr = &s; break; }

            if (active_spr && font_small) {
                int info_x = panel_x + 10;
                int info_y2 = panel_y + SPRITE_THUMB_H + 16;
                std::string info = active_spr->name +
                    "  x:" + std::to_string((int)active_spr->x) +
                    "  y:" + std::to_string((int)active_spr->y) +
                    "  dir:" + std::to_string((int)active_spr->direction);
                renderText(renderer, font_small, info, info_x, info_y2, {180,180,180,255});

                // دکمه‌های show/hide و delete برای sprite فعال
                // Hide/Show
                SDL_Rect vis_btn = {panel_x + STAGE_W - 90, panel_y + SPRITE_THUMB_H + 10, 38, 20};
                SDL_Color vis_col = active_spr->visible ? SDL_Color{50,130,50,255} : SDL_Color{130,80,50,255};
                fillRect(renderer, vis_btn.x, vis_btn.y, vis_btn.w, vis_btn.h, vis_col);
                renderText(renderer, font_small,
                    active_spr->visible ? "Show" : "Hide",
                    vis_btn.x+6, vis_btn.y+4, {255,255,255,255});

                // Delete
                SDL_Rect del_btn = {panel_x + STAGE_W - 46, panel_y + SPRITE_THUMB_H + 10, 38, 20};
                fillRect(renderer, del_btn.x, del_btn.y, del_btn.w, del_btn.h, {150,40,40,255});
                renderText(renderer, font_small, "Del",
                    del_btn.x+8, del_btn.y+4, {255,255,255,255});
            }
        }

        // راهنمای کلیدها
        if (font_small) {
            renderText(renderer, font_small, "F5:Run | F6:Stop | S:Save",
                area_stage.x+8, WINDOW_H - SPRITE_PANEL_H - 20, {80,80,80,255});
        }

        // ── پایان رندر ──
        SDL_RenderPresent(renderer);

    } // end while(running)

    // ── پاک‌سازی ──
    freeTextureCache();
    IMG_Quit();
    if (font_normal) TTF_CloseFont(font_normal);
    if (font_small)  TTF_CloseFont(font_small);
    if (font_bold)   TTF_CloseFont(font_bold);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
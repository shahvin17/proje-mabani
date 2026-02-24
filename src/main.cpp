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
#include "pen.h"
#include "sensing.h"
#include "sound.h"

// ─── ثابت‌های ابعاد UI ───────────────────────────────────────────────────────
// ابعاد پنجره — در main() بعد از fullscreen آپدیت می‌شوند
static int WINDOW_W      = 1280;
static int WINDOW_H      = 720;
static const int TOOLBAR_H     = 50;
static const int SIDEBAR_W     = 270;  // پانل دسته‌بندی + پالت
static const int CATEGORY_W    = 98;   // ستون دسته‌بندی (عریض برای نام کامل)
static const int PALETTE_W     = SIDEBAR_W - CATEGORY_W;
static const int STAGE_W         = 320;
static const int STAGE_H         = 240;
static const int BLOCK_H         = 44;
static const int BLOCK_W         = 170;   // عریض‌تر برای input fields
static const int SPRITE_PANEL_H  = 135;  // ارتفاع پانل sprite در زیر stage
static const int SPRITE_THUMB_W  = 70;
static const int SPRITE_THUMB_H  = 70;

// ─── رنگ‌های دسته‌بندی (مطابق Scratch اصلی) ─────────────────────────────────
struct CategoryInfo {
    std::string name;
    SDL_Color   color;
};

static std::vector<CategoryInfo> CATEGORIES = {  // non-const برای Extensions
    {"Motion",    {74,  144, 226, 255}},
    {"Looks",     {153, 102, 204, 255}},
    {"Sound",     {207,  99, 207, 255}},
    {"Events",    {255, 171,  25, 255}},
    {"Control",   {255, 140,   0, 255}},
    {"Sensing",   {92,  196, 220, 255}},
    {"Operators", {89,  192,  89, 255}},
    {"Variables", {255, 140,  26, 255}},
    {"Pen",       {0,   200, 100, 255}},
};

// Extensions available
struct ExtensionInfo {
    std::string id, name, description;
    SDL_Color color;
    bool enabled;
};
static std::vector<ExtensionInfo> EXTENSIONS = {
    {"pen",     "Pen",         "Draw on stage",         {0,200,100,255},  true},
    {"video",   "Video",       "Video sensing",         {90,90,200,255},  false},
    {"tts",     "Text to Speech","Say blocks aloud",    {200,100,180,255},false},
    {"micro",   "Microphone",  "Sound input",           {200,60,60,255},  false},
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
    {"next_backdrop",   "next backdrop",           {},      "Looks"},
    {"prev_backdrop",   "prev backdrop",           {},      "Looks"},
    {"set_backdrop",    "switch backdrop to",      {0},     "Looks"},
    {"next_costume",    "next costume",            {},      "Looks"},
    {"prev_costume",    "prev costume",            {},      "Looks"},
    {"set_costume",     "switch costume to",       {0},     "Looks"},
    // Sound
    {"play_sound",      "play sound pop",         {0},     "Sound"},
    {"play_sound_wait", "play sound pop until done",{0},   "Sound"},
    {"stop_sounds",     "stop all sounds",        {},      "Sound"},
    {"set_volume",      "set volume to 100%",     {100},   "Sound"},
    {"change_volume",   "change volume by -10",   {-10},   "Sound"},
    // Sensing (Ask)
    {"ask",             "ask What's your name? and wait", {}, "Sensing"},
    // Control (Clone)
    {"create_clone",    "create clone of myself",  {-1},   "Control"},
    {"when_clone_start","when I start as a clone",  {},    "Control"},
    {"delete_clone",    "delete this clone",        {},    "Control"},
    {"stop_this",       "stop this script",         {},    "Control"},
    // Events
    {"when_start",      "when 🏁 clicked",        {},      "Events"},
    {"when_key",        "when space pressed",     {},      "Events"},
    {"when_clicked",    "when sprite clicked",    {},      "Events"},
    // Control
    {"wait",            "wait 1 secs",            {1},     "Control"},
    {"repeat",          "repeat 10",              {10},    "Control"},
    {"forever",         "forever",                {},      "Control"},
    {"if_then",         "if <condition> then",    {},      "Control"},
    {"if_else",         "if <cond> else",          {},      "Control"},
    {"stop_all",        "stop all",               {},      "Control"},
    {"end_repeat",      "end repeat ←",           {},      "Control"},
    // Sensing
    {"touching_mouse",  "touching mouse?",        {},      "Sensing"},
    {"key_pressed",     "key space pressed?",     {},      "Sensing"},
    {"mouse_down",      "mouse down?",            {},      "Sensing"},
    {"mouse_x",         "mouse x",                {},      "Sensing"},
    {"mouse_y",         "mouse y",                {},      "Sensing"},
    {"distance_to_mouse","distance to mouse",     {},      "Sensing"},
    {"reset_timer",     "reset timer",            {},      "Sensing"},
    {"set_drag_mode",   "set drag mode",          {1},     "Sensing"},
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
    // Pen (Add Extension)
    {"pen_erase_all",   "erase all",              {},      "Pen"},
    {"pen_stamp",       "stamp",                  {},      "Pen"},
    {"pen_down",        "pen down",               {},      "Pen"},
    {"pen_up",          "pen up",                 {},      "Pen"},
    {"pen_set_color",   "set pen color",          {0,0,255},"Pen"},
    {"pen_change_color","change pen color by",    {10},    "Pen"},
    {"pen_set_size",    "set pen size to",        {1},     "Pen"},
    {"pen_change_size", "change pen size by",     {1},     "Pen"},
    {"pen_set_hue",     "set pen hue to",         {100},   "Pen"},
    {"pen_change_hue",  "change pen hue by",      {10},    "Pen"},
    {"pen_set_brightness","set pen brightness to",{100},   "Pen"},
    {"pen_change_brightness","change pen brightness by",{10},"Pen"},
    {"pen_set_saturation","set pen saturation to",{100},   "Pen"},
    {"pen_change_saturation","change pen saturation by",{10},"Pen"},
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
        {"next_backdrop", "next backdrop"},
        {"prev_backdrop", "prev backdrop"},
        {"set_backdrop",  "switch backdrop to"},
        {"next_costume",  "next costume"},
        {"prev_costume",  "prev costume"},
        {"set_costume",   "switch costume to"},
        {"wait",         "wait"},
        {"repeat",       "repeat"},
        {"when_start",   "when ð© clicked"},
        {"when_key",     "when space pressed"},
        {"when_clicked", "when sprite clicked"},
        {"forever",      "forever"},
        {"end_repeat",   "end repeat ←"},
        {"if_then",      "if <condition> then"},
        {"if_else",      "if <cond> else"},
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
        // Pen
        {"pen_erase_all",    "erase all"},
        {"pen_stamp",        "stamp"},
        {"pen_down",         "pen down"},
        {"pen_up",           "pen up"},
        {"pen_set_color",    "set pen color"},
        {"pen_change_color", "change pen color by"},
        {"pen_set_size",     "set pen size to"},
        {"pen_change_size",  "change pen size by"},
        {"pen_set_hue",      "set pen hue to"},
        {"pen_change_hue",   "change pen hue by"},
        {"pen_set_brightness","set pen brightness to"},
        {"pen_change_brightness","change pen brightness by"},
        {"pen_set_saturation","set pen saturation to"},
        {"pen_change_saturation","change pen saturation by"},
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

    // drag block
    Block* dragging_block    = nullptr;
    int    drag_offset_x     = 0;
    int    drag_offset_y     = 0;
    bool   drag_from_palette = false;
    float  drag_start_x      = 0;   // موقعیت قبل از drag (برای undo)
    float  drag_start_y      = 0;

    // snap
    int    snap_target_id    = -1;

    // ورودی متنی
    int    editing_block_id  = -1;
    int    editing_input_idx = 0;
    std::string input_buffer = "";
    int    old_input_val     = 0;   // برای undo input change

    // sprite panel
    int    active_sprite_id  = 1;
    bool   show_sprite_panel = true;

    // Variables
    bool   show_var_dialog   = false;
    std::string new_var_name = "";
    // Variable Monitor drag
    int    dragging_var_idx  = -1;  // کدام variable monitor در حال drag است
    int    var_drag_ox       = 0;
    int    var_drag_oy       = 0;

    // Controls
    bool   step_mode             = false;
    bool   show_extensions_panel = false;
    bool   show_costume_panel    = false;

    // ── Undo / Redo ──────────────────────────────────────────────────────────
    std::vector<UndoAction> undo_stack;
    std::vector<UndoAction> redo_stack;
    static const int MAX_UNDO = 50;

    void pushUndo(const UndoAction& a) {
        undo_stack.push_back(a);
        if ((int)undo_stack.size() > MAX_UNDO)
            undo_stack.erase(undo_stack.begin());
        redo_stack.clear();
    }

    // ── Right-click Context Menu ──────────────────────────────────────────
    bool   show_context_menu  = false;
    int    ctx_block_id       = -1;    // بلوک زیر راست-کلیک
    int    ctx_menu_x         = 0;
    int    ctx_menu_y         = 0;

    // ── Backdrop Chooser ──────────────────────────────────────────────────
    bool   show_backdrop_panel = false;
    int    editing_backdrop_idx = -1;  // کدام backdrop در حال rename

    // ── Costume Panel ─────────────────────────────────────────────────────
    bool   show_costume_editor  = false;
    int    editing_costume_idx  = -1;
};

// ── helper: SDL_PointInRect بدون address-of-rvalue ────────────────────────────
static inline bool pointInRect(int px, int py, SDL_Rect r) {
    return px>=r.x && px<r.x+r.w && py>=r.y && py<r.y+r.h;
}

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
    // Stage coordinates (global در scope main برای event handlers)
    int g_stage_x = 0, g_stage_y = 0, g_stage_w = 480, g_stage_h = 360;

    Project project;
    SensingManager sensing;
    SoundManager   sound_mgr;
    Runtime rt;
    runtime_init(&rt, &project);
    rt.sensing   = &sensing;
    rt.sound_mgr = &sound_mgr;
    sound_init(sound_mgr);
    // اضافه کردن صداهای پیش‌فرض
    sound_add(sound_mgr, "pop",    "assets/sounds/pop.wav");
    sound_add(sound_mgr, "meow",   "assets/sounds/meow.wav");
    sound_add(sound_mgr, "laser",  "assets/sounds/laser.wav");

    int next_block_id = 100;

    // ── تابع کمکی: ساخت پروژه پیش‌فرض (sprite + when_start) ──
    auto setup_default_project = [&]() {
        project.sprites.clear();
        project.variables.clear();
        // متغیر "answer" برای Ask block (پیش‌فرض، مخفی)
        {
            Variable ans_var;
            ans_var.name    = "answer";
            ans_var.value   = 0.0f;
            ans_var.visible = false;
            project.variables.push_back(ans_var);
        }
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
        // costume پیش‌فرض
        {
            Costume c; c.name = "costume1";
            c.path = g_assets_path + "sprite1.png";
            spr.costumes.push_back(c);
            spr.costume_index = 0;
            spr.costume_path  = c.path;
        }
        project.sprites.push_back(spr);
        // when_start_id را ذخیره کن (بعداً b.id را می‌دانیم)

        // backdrop پیش‌فرض
        project.backdrops.clear();
        {
            Backdrop bd; bd.name = "backdrop1";
            bd.path = g_assets_path + "background.png";
            project.backdrops.push_back(bd);
        }
        project.active_backdrop_idx = 0;

        // بلوک when_start پیش‌فرض (مربوط به spr)
        Block b;
        b.id          = next_block_id++;
        b.type        = "when_start";
        b.x           = SIDEBAR_W + 40;
        b.y           = TOOLBAR_H + 40;
        b.sprite_owner = spr.id;  // ← مربوط به sprite1
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
    SDL_Rect btn_step  = {WINDOW_W/2 + 52, 8, 52, 34};
    SDL_Rect btn_pause = {WINDOW_W/2 + 108, 8, 60, 34};
    SDL_Rect btn_undo  = {WINDOW_W/2 + 174, 8, 44, 34};
    SDL_Rect btn_redo  = {WINDOW_W/2 + 222, 8, 44, 34};
    SDL_Rect btn_turbo = {WINDOW_W/2 + 270, 8, 54, 34};
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

                // ── بستن Context Menu با کلیک خارج ──
                if (ui.show_context_menu) {
                    const int CM_W=172, CM_H=28, N=5;
                    SDL_Rect cm_rect={ui.ctx_menu_x, ui.ctx_menu_y, CM_W, N*CM_H};
                    if (!pointInRect(mp.x, mp.y, {ui.ctx_menu_x,ui.ctx_menu_y,CM_W,N*CM_H}))
                        ui.show_context_menu=false;
                }

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
                else if (SDL_PointInRect(&mp, &btn_pause)) {
                    if (runtime_isRunning(&rt)) {
                        runtime_pause(&rt);
                        logInfo("Runtime paused.");
                    } else if (runtime_isPaused(&rt)) {
                        runtime_resume(&rt);
                        logInfo("Runtime resumed.");
                    }
                }
                else if (SDL_PointInRect(&mp, &btn_undo)) {
                    // شبیه‌سازی Ctrl+Z
                    SDL_Event fake; fake.type=SDL_KEYDOWN;
                    fake.key.keysym.sym=122 /*z*/;
                    // مستقیم اجرا: تکرار منطق undo
                    if (!ui.undo_stack.empty()) {
                        UndoAction a = ui.undo_stack.back(); ui.undo_stack.pop_back();
                        if (a.type==ActionType::BLOCK_MOVE){
                            Block* b=findBlock(project,a.block_id);
                            if(b){b->x=a.old_x;b->y=a.old_y;}
                        } else if (a.type==ActionType::BLOCK_SNAP){
                            Block* b=findBlock(project,a.block_id);
                            if(b) b->nextBlockId=a.old_next_id;
                        } else if (a.type==ActionType::INPUT_CHANGE){
                            Block* b=findBlock(project,a.block_id);
                            if(b&&a.input_idx<(int)b->inputs.size())
                                b->inputs[a.input_idx]=a.old_input_val;
                        }
                        ui.redo_stack.push_back(a);
                        logInfo("Undo via button");
                    }
                }
                else if (SDL_PointInRect(&mp, &btn_redo)) {
                    if (!ui.redo_stack.empty()) {
                        UndoAction a = ui.redo_stack.back(); ui.redo_stack.pop_back();
                        if (a.type==ActionType::BLOCK_MOVE){
                            Block* b=findBlock(project,a.block_id);
                            if(b){b->x=a.new_x;b->y=a.new_y;}
                        } else if (a.type==ActionType::BLOCK_SNAP){
                            Block* b=findBlock(project,a.block_id);
                            if(b) b->nextBlockId=a.new_next_id;
                        } else if (a.type==ActionType::INPUT_CHANGE){
                            Block* b=findBlock(project,a.block_id);
                            if(b&&a.input_idx<(int)b->inputs.size())
                                b->inputs[a.input_idx]=a.new_input_val;
                        }
                        ui.undo_stack.push_back(a);
                        logInfo("Redo via button");
                    }
                }
                else if (SDL_PointInRect(&mp, &btn_turbo)) {
                    rt.turbo_mode = !rt.turbo_mode;
                    logInfo(rt.turbo_mode ? "Turbo ON (30x)" : "Turbo OFF");
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
                // --- دکمه Backdrop Chooser (زیر stage) ---
                else if (pointInRect(mp.x,mp.y,{g_stage_x,g_stage_y+g_stage_h+4,90,22})) {
                    ui.show_backdrop_panel  = !ui.show_backdrop_panel;
                    ui.show_costume_editor  = false;
                    ui.show_extensions_panel = false;
                    ui.show_context_menu    = false;
                }
                // --- دکمه Costume Editor (زیر stage) ---
                else if (pointInRect(mp.x,mp.y,{g_stage_x+96,g_stage_y+g_stage_h+4,90,22})) {
                    ui.show_costume_editor   = !ui.show_costume_editor;
                    ui.show_backdrop_panel   = false;
                    ui.show_extensions_panel = false;
                    ui.show_context_menu     = false;
                }
                // --- Backdrop Panel کلیک ها ---
                else if (ui.show_backdrop_panel) {
                    int bp_w=340, bp_h=300;
                    int bp_x=g_stage_x, bp_y=g_stage_y+g_stage_h-bp_h;
                    if (bp_y < g_stage_y) bp_y=g_stage_y;
                    // X بستن
                    SDL_Rect bd_x_btn={bp_x+bp_w-30,bp_y+4,24,24};
                    if (SDL_PointInRect(&mp,&bd_x_btn)) {
                        ui.show_backdrop_panel=false;
                    } else {
                        // کلیک روی backdrop برای فعال کردن
                        int by=bp_y+38;
                        for (int bi=0; bi<(int)project.backdrops.size(); ++bi) {
                            SDL_Rect br={bp_x+6,by,bp_w-12,36};
                            if (SDL_PointInRect(&mp,&br)) {
                                project.active_backdrop_idx=bi;
                                project.isModified=true;
                                logInfo("Backdrop switched to: "+project.backdrops[bi].name);
                                break;
                            }
                            by+=40;
                        }
                        // دکمه Add Backdrop
                        SDL_Rect ab_r={bp_x+6,bp_y+bp_h-36,bp_w-12,28};
                        if (SDL_PointInRect(&mp,&ab_r)) {
                            Backdrop nb; nb.name="backdrop"+std::to_string(project.backdrops.size()+1); nb.path="";
                            project.backdrops.push_back(nb);
                            project.active_backdrop_idx=(int)project.backdrops.size()-1;
                            project.isModified=true;
                            logInfo("Added backdrop: "+nb.name);
                        }
                    }
                }
                // --- Costume Panel کلیک ها ---
                else if (ui.show_costume_editor) {
                    Sprite* act_spr=nullptr;
                    for (auto& s:project.sprites) if(s.id==ui.active_sprite_id){act_spr=&s;break;}
                    int cp_w=300, cp_h=280;
                    int cp_x=g_stage_x+g_stage_w-cp_w, cp_y=g_stage_y;
                    // X
                    SDL_Rect cs_x_btn={cp_x+cp_w-30,cp_y+4,24,24};
                    if (SDL_PointInRect(&mp,&cs_x_btn)) {
                        ui.show_costume_editor=false;
                    } else if (act_spr) {
                        // کلیک روی costume
                        int cy2=cp_y+38;
                        for (int ci=0; ci<(int)act_spr->costumes.size(); ++ci) {
                            SDL_Rect cr={cp_x+6,cy2,cp_w-12,40};
                            if (SDL_PointInRect(&mp,&cr)) {
                                act_spr->costume_index=ci;
                                act_spr->costume_path=act_spr->costumes[ci].path;
                                project.isModified=true;
                                logInfo("Costume: "+act_spr->costumes[ci].name);
                                break;
                            }
                            cy2+=44;
                        }
                        // دکمه Add Costume
                        SDL_Rect ac_r={cp_x+6,cp_y+cp_h-36,cp_w-12,28};
                        if (SDL_PointInRect(&mp,&ac_r)) {
                            Costume nc;
                            nc.name="costume"+std::to_string(act_spr->costumes.size()+1);
                            nc.path="";
                            act_spr->costumes.push_back(nc);
                            act_spr->costume_index=(int)act_spr->costumes.size()-1;
                            project.isModified=true;
                            logInfo("Added costume: "+nc.name);
                        }
                    }
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
                    const int EXT_BTN_H2 = 32;
                    int btn_y = area_category.y + area_category.h - EXT_BTN_H2 - 2;
                    SDL_Rect ext_btn = {area_category.x+2, btn_y, CATEGORY_W-4, EXT_BTN_H2};
                    if (SDL_PointInRect(&mp, &ext_btn)) {
                        ui.show_extensions_panel = !ui.show_extensions_panel;
                    } else {
                        int cat_h2 = (area_category.h - EXT_BTN_H2 - 4) / (int)CATEGORIES.size();
                        cat_h2 = std::max(28, cat_h2);
                        int rel_y = my - area_category.y;
                        int idx2 = rel_y / cat_h2;
                        if (idx2 >= 0 && idx2 < (int)CATEGORIES.size())
                            ui.active_category = CATEGORIES[idx2].name;
                    }
                }
                // ── بستن Extensions Panel با کلیک X ──
                else if (ui.show_extensions_panel) {
                    int ep_w=380;
                    int ep_h=(int)EXTENSIONS.size()*70+70;
                    int ep_x=WINDOW_W/2-ep_w/2, ep_y=WINDOW_H/2-ep_h/2;
                    // دکمه X بستن
                    SDL_Rect close_btn={ep_x+ep_w-34,ep_y+6,26,24};
                    if (SDL_PointInRect(&mp, &close_btn)) {
                        ui.show_extensions_panel = false;
                    } else {
                        // کلیک روی دکمه Add/Added هر extension
                        int ey2=ep_y+44;
                        for (int ei=0; ei<(int)EXTENSIONS.size(); ++ei) {
                            int row_h=64;
                            SDL_Rect btn_r={ep_x+ep_w-90, ey2+16, 74, 28};
                            if (SDL_PointInRect(&mp, &btn_r)) {
                                EXTENSIONS[ei].enabled = !EXTENSIONS[ei].enabled;
                                // اگه فعال شد، category رو اضافه کن
                                if (EXTENSIONS[ei].enabled) {
                                    bool found=false;
                                    for (auto& c:CATEGORIES)
                                        if(c.name==EXTENSIONS[ei].name){found=true;break;}
                                    if(!found){
                                        CategoryInfo ci;
                                        ci.name=EXTENSIONS[ei].name;
                                        ci.color=EXTENSIONS[ei].color;
                                        CATEGORIES.push_back(ci);
                                        ui.active_category = EXTENSIONS[ei].name;
                                        logInfo("Extension added: " + EXTENSIONS[ei].name);
                                    }
                                } else {
                                    // حذف category
                                    CATEGORIES.erase(
                                        std::remove_if(CATEGORIES.begin(),CATEGORIES.end(),
                                            [&](const CategoryInfo& c){return c.name==EXTENSIONS[ei].name;}),
                                        CATEGORIES.end());
                                    logInfo("Extension removed: " + EXTENSIONS[ei].name);
                                }
                                break;
                            }
                            ey2 += row_h;
                        }
                    }
                }
                // --- Context Menu click ---
                else if (ui.show_context_menu) {
                    const int CM_W = 170, CM_ITEM_H = 28;
                    // آیتم‌ها: Duplicate, Delete, Add Comment, Disable
                    struct CtxItem { std::string label; int action; };
                    static const CtxItem CTX_ITEMS[] = {
                        {"Duplicate block", 1},
                        {"Delete block",    2},
                        {"Add comment",     3},
                        {"Disable block",   4},
                        {"Clean up blocks", 5},
                    };
                    const int N_CTX = 5;
                    bool hit = false;
                    for (int ci = 0; ci < N_CTX; ++ci) {
                        SDL_Rect ir = {ui.ctx_menu_x, ui.ctx_menu_y + ci*CM_ITEM_H,
                                       CM_W, CM_ITEM_H};
                        if (SDL_PointInRect(&mp, &ir)) {
                            hit = true;
                            Block* cb = findBlock(project, ui.ctx_block_id);
                            if (CTX_ITEMS[ci].action == 1 && cb) {
                                // Duplicate
                                Block nb = *cb;
                                nb.id = next_block_id++;
                                nb.x += 20; nb.y += 20;
                                nb.nextBlockId = -1;
                                // ثبت undo
                                UndoAction ua; ua.type=ActionType::BLOCK_ADD;
                                ua.block_id=nb.id; ua.saved_block=nb;
                                ui.pushUndo(ua);
                                project.blocks.push_back(nb);
                                logInfo("Duplicated block: " + nb.type);
                            }
                            else if (CTX_ITEMS[ci].action == 2 && cb) {
                                // Delete
                                UndoAction ua; ua.type=ActionType::BLOCK_DELETE;
                                ua.block_id=cb->id; ua.saved_block=*cb;
                                ui.pushUndo(ua);
                                // قطع کردن لینک از بلوک قبلی
                                for (auto& ob : project.blocks)
                                    if (ob.nextBlockId == cb->id) ob.nextBlockId=-1;
                                project.blocks.erase(
                                    std::remove_if(project.blocks.begin(),project.blocks.end(),
                                        [&](const Block& bl){return bl.id==ui.ctx_block_id;}),
                                    project.blocks.end());
                                logInfo("Deleted block");
                            }
                            else if (CTX_ITEMS[ci].action == 5) {
                                // Clean up: مرتب کردن بلوک‌های loose
                                float cx = SIDEBAR_W + 30.f, cy = TOOLBAR_H + 30.f;
                                for (auto& bl : project.blocks) {
                                    // بلوک بدون parent = root
                                    bool is_child = false;
                                    for (auto& ob : project.blocks)
                                        if (ob.nextBlockId == bl.id){ is_child=true; break; }
                                    if (!is_child && bl.type!="when_start") {
                                        bl.x = cx; bl.y = cy; cy += bl.height + 10;
                                        if (cy > WINDOW_H - 100) { cy=TOOLBAR_H+30; cx+=220; }
                                    }
                                }
                            }
                            break;
                        }
                    }
                    if (!hit) ui.show_context_menu = false;
                    if (CTX_ITEMS[0].action != 0) ui.show_context_menu = false;
                }
                // --- Variable dialog click ---
                else if (ui.show_var_dialog) {
                    int dlg_w=300, dlg_h=120;
                    int dlg_x=WINDOW_W/2-dlg_w/2, dlg_y=WINDOW_H/2-dlg_h/2;
                    SDL_Rect ok_btn     = {dlg_x+60,  dlg_y+80, 70, 26};
                    SDL_Rect cancel_btn = {dlg_x+140, dlg_y+80, 70, 26};
                    if (SDL_PointInRect(&mp, &ok_btn) && !ui.new_var_name.empty()) {
                        // چک تکراری نبودن
                        bool dup = false;
                        for (auto& v : project.variables)
                            if (v.name == ui.new_var_name) { dup=true; break; }
                        if (!dup) {
                            Variable var;
                            var.name    = ui.new_var_name;
                            var.value   = 0;
                            var.visible = true;
                            project.variables.push_back(var);
                            logInfo("Variable created: " + ui.new_var_name);
                        }
                        ui.show_var_dialog = false;
                        ui.new_var_name    = "";
                        SDL_StopTextInput();
                    }
                    else if (SDL_PointInRect(&mp, &cancel_btn)) {
                        ui.show_var_dialog = false;
                        ui.new_var_name    = "";
                        SDL_StopTextInput();
                    }
                }

                // --- Ask Dialog: کلیک دکمه ✓ ---
                else if (rt.ask_active && SDL_PointInRect(&mp, &area_stage)) {
                    // بررسی کلیک روی دکمه ارسال
                    int aw = g_stage_w - 40;
                    int ax = g_stage_x + 20;
                    int ay = g_stage_y + g_stage_h - 60;
                    SDL_Rect submit_rect = {ax+aw-42, ay+4, 40, 40};
                    if (SDL_PointInRect(&mp, &submit_rect)) {
                        runtime_submit_answer(&rt, rt.ask_buffer);
                    }
                }
                // --- Variable Monitor drag (کلیک روی monitor روی stage) ---
                else if (SDL_PointInRect(&mp, &area_stage) && !ui.show_var_dialog && !ui.show_extensions_panel) {
                    for (int vi = 0; vi < (int)project.variables.size(); ++vi) {
                        auto& var = project.variables[vi];
                        if (!var.visible) continue;
                        int mon_x = g_stage_x + (int)var.monitor_x;
                        int mon_y = g_stage_y + (int)var.monitor_y;
                        int mon_w = (int)(var.name.size()*8) + 68;
                        SDL_Rect mr = {mon_x, mon_y, mon_w, 22};
                        if (SDL_PointInRect(&mp, &mr)) {
                            ui.dragging_var_idx = vi;
                            ui.var_drag_ox = mx - mon_x;
                            ui.var_drag_oy = my - mon_y;
                            break;
                        }
                    }
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
                        // عرض بلوک: label + inputs + padding
                        {
                            std::string lbl = nb.inputs.empty()
                                ? labelForBlockType(nb.type)
                                : labelNoNumbers(nb.type);
                            int lbl_px  = (int)lbl.size() * 8 + 16; // ~8px/char + padding
                            int inp_px  = (int)nb.inputs.size() * 46; // 42px + 4 gap
                            nb.width  = std::max(BLOCK_W, lbl_px + inp_px);
                            nb.height = BLOCK_H;
                        }
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
                                ui.drag_start_x      = b.x;   // ذخیره برای Undo
                                ui.drag_start_y      = b.y;
                                break;
                            }
                        }
                    }
                }
            }

            // ── Mouse Motion ──
                    else if (event.type == SDL_MOUSEMOTION && ui.dragging_var_idx >= 0) {
                // drag variable monitor
                auto& var = project.variables[ui.dragging_var_idx];
                var.monitor_x = (float)(event.motion.x - ui.var_drag_ox - g_stage_x);
                var.monitor_y = (float)(event.motion.y - ui.var_drag_oy - g_stage_y);
                // clamp داخل stage
                var.monitor_x = std::max(0.f, std::min(var.monitor_x, (float)(g_stage_w - 100)));
                var.monitor_y = std::max(0.f, std::min(var.monitor_y, (float)(g_stage_h - 30)));
            }
            else if (event.type == SDL_MOUSEMOTION && ui.dragging_block) {
                ui.dragging_block->x = (float)(event.motion.x - ui.drag_offset_x);
                ui.dragging_block->y = (float)(event.motion.y - ui.drag_offset_y);
                ui.snap_target_id = findSnapTarget(*ui.dragging_block, project);
            }

            // ── Mouse Up ──
            if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
                // رها کردن variable monitor
                if (ui.dragging_var_idx >= 0) {
                    ui.dragging_var_idx = -1;
                }
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
                int k2 = event.key.keysym.sym;
                sense_update_key(sensing, k2, true);

                // ── Ask Dialog keyboard ──
                if (rt.ask_active) {
                    if (k2 == SDLK_RETURN || k2 == SDLK_KP_ENTER) {
                        runtime_submit_answer(&rt, rt.ask_buffer);
                    } else if (k2 == SDLK_BACKSPACE && !rt.ask_buffer.empty()) {
                        rt.ask_buffer.pop_back();
                    } else if (k2 == SDLK_ESCAPE) {
                        runtime_submit_answer(&rt, "");
                    }
                } else if (k2 == SDLK_ESCAPE) {
                    // ESC: بستن همه panel های باز
                    ui.show_context_menu     = false;
                    ui.show_backdrop_panel   = false;
                    ui.show_costume_editor   = false;
                    ui.show_extensions_panel = false;
                    ui.show_var_dialog       = false;
                    ui.editing_block_id      = -1;
                }

                // ── Undo: Ctrl+Z ──
                if ((k2 == 122 /*z*/) && (SDL_GetModState() & KMOD_CTRL)) {
                    if (!ui.undo_stack.empty()) {
                        UndoAction a = ui.undo_stack.back();
                        ui.undo_stack.pop_back();
                        // اجرای undo
                        if (a.type == ActionType::BLOCK_MOVE) {
                            Block* b = findBlock(project, a.block_id);
                            if (b) { b->x = a.old_x; b->y = a.old_y; }
                        } else if (a.type == ActionType::BLOCK_SNAP) {
                            // شکستن snap: nextBlockId رو برگردون
                            Block* b = findBlock(project, a.block_id);
                            if (b) b->nextBlockId = a.old_next_id;
                        } else if (a.type == ActionType::BLOCK_ADD) {
                            // حذف بلوک اضافه‌شده
                            project.blocks.erase(
                                std::remove_if(project.blocks.begin(), project.blocks.end(),
                                    [&](const Block& bl){ return bl.id == a.block_id; }),
                                project.blocks.end());
                        } else if (a.type == ActionType::BLOCK_DELETE) {
                            project.blocks.push_back(a.saved_block);
                        } else if (a.type == ActionType::INPUT_CHANGE) {
                            Block* b = findBlock(project, a.block_id);
                            if (b && a.input_idx < (int)b->inputs.size())
                                b->inputs[a.input_idx] = a.old_input_val;
                        }
                        ui.redo_stack.push_back(a);
                        project.isModified = true;
                        logInfo("Undo: " + std::to_string((int)a.type));
                    }
                }
                // ── Redo: Ctrl+Y ──
                else if ((k2 == SDLK_y) && (SDL_GetModState() & KMOD_CTRL)) {
                    if (!ui.redo_stack.empty()) {
                        UndoAction a = ui.redo_stack.back();
                        ui.redo_stack.pop_back();
                        // اجرای redo
                        if (a.type == ActionType::BLOCK_MOVE) {
                            Block* b = findBlock(project, a.block_id);
                            if (b) { b->x = a.new_x; b->y = a.new_y; }
                        } else if (a.type == ActionType::BLOCK_SNAP) {
                            Block* b = findBlock(project, a.block_id);
                            if (b) b->nextBlockId = a.new_next_id;
                        } else if (a.type == ActionType::BLOCK_ADD) {
                            project.blocks.push_back(a.saved_block);
                        } else if (a.type == ActionType::BLOCK_DELETE) {
                            project.blocks.erase(
                                std::remove_if(project.blocks.begin(), project.blocks.end(),
                                    [&](const Block& bl){ return bl.id == a.block_id; }),
                                project.blocks.end());
                        } else if (a.type == ActionType::INPUT_CHANGE) {
                            Block* b = findBlock(project, a.block_id);
                            if (b && a.input_idx < (int)b->inputs.size())
                                b->inputs[a.input_idx] = a.new_input_val;
                        }
                        ui.undo_stack.push_back(a);
                        project.isModified = true;
                        logInfo("Redo: " + std::to_string((int)a.type));
                    }
                }
                // ── Delete block: DEL key ──
                else if (k2 == SDLK_DELETE && ui.editing_block_id == -1) {
                    // حذف بلوک زیر ماوس (اگه snap نشده)
                    // بعداً پیاده می‌شه با right-click menu
                }

                // ── Variable Dialog Keyboard ──
                if (ui.show_var_dialog) {
                    if (k2 == SDLK_RETURN || k2 == SDLK_KP_ENTER) {
                        if (!ui.new_var_name.empty()) {
                            bool dup = false;
                            for (auto& v : project.variables)
                                if (v.name == ui.new_var_name) { dup=true; break; }
                            if (!dup) {
                                Variable nv; nv.name=ui.new_var_name; nv.value=0; nv.visible=true;
                                project.variables.push_back(nv);
                            }
                        }
                        ui.show_var_dialog = false; ui.new_var_name = ""; SDL_StopTextInput();
                    } else if (k2 == SDLK_ESCAPE) {
                        ui.show_var_dialog = false; ui.new_var_name = ""; SDL_StopTextInput();
                    } else if (k2 == SDLK_BACKSPACE && !ui.new_var_name.empty()) {
                        ui.new_var_name.pop_back();
                    }
                }
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
            else if (event.type == SDL_KEYUP) {
                sense_update_key(sensing, (int)event.key.keysym.sym, false);
            }
            // ── Right-click → Context Menu ──
            else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 3  /*SDL_BUTTON_RIGHT*/) {
                int rx = event.button.x, ry = event.button.y;
                ui.show_context_menu  = false;
                ui.ctx_block_id       = -1;
                // پیدا کردن بلوک زیر ماوس
                for (auto& b : project.blocks) {
                    SDL_Rect br = {(int)b.x, (int)b.y, (int)b.width, (int)b.height};
                    if (pointInRect(rx, ry, br)) {
                        ui.show_context_menu = true;
                        ui.ctx_block_id      = b.id;
                        ui.ctx_menu_x        = rx;
                        ui.ctx_menu_y        = ry;
                        break;
                    }
                }
                // بستن پانل‌های دیگه
                if (ui.show_context_menu) {
                    ui.show_backdrop_panel = false;
                    ui.show_extensions_panel = false;
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
                // ── Ask dialog: تایپ متن ──
                if (rt.ask_active) {
                    rt.ask_buffer += event.text.text;
                }
                else if (ui.show_var_dialog) {
                    for (char c : std::string(event.text.text))
                        if (std::isalnum((unsigned char)c) || c=='_')
                            ui.new_var_name += c;
                }
                if (ui.editing_block_id != -1) {
                // فقط ارقام و نقطه اعشار
                std::string incoming = event.text.text;
                for (char c : incoming) {
                    if (std::isdigit(c) || (c == '.' && ui.input_buffer.find('.') == std::string::npos))
                        ui.input_buffer += c;
                }
                } // end editing_block
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
            if (font_normal) renderText(renderer, font_normal, "▶|Step",
                btn_step.x+4, btn_step.y+9, {255,255,255,255});
        }
        // دکمه Pause/Resume (زرد)
        {
            bool is_paused = runtime_isPaused(&rt);
            SDL_Color pc = is_paused ? SDL_Color{200,160,0,255} : SDL_Color{160,130,0,255};
            fillRect(renderer, btn_pause.x, btn_pause.y, btn_pause.w, btn_pause.h, pc);
            drawRect(renderer, btn_pause.x, btn_pause.y, btn_pause.w, btn_pause.h, {100,80,0,255});
            std::string plbl = is_paused ? "▶Res" : "||Pse";
            if (font_small) renderText(renderer, font_small, plbl,
                btn_pause.x+5, btn_pause.y+11, {255,255,255,255});
        }
        // دکمه Undo (Ctrl+Z)
        {
            bool can_undo = !ui.undo_stack.empty();
            SDL_Color uc = can_undo ? SDL_Color{80,80,120,255} : SDL_Color{50,50,60,255};
            fillRect(renderer, btn_undo.x, btn_undo.y, btn_undo.w, btn_undo.h, uc);
            drawRect(renderer, btn_undo.x, btn_undo.y, btn_undo.w, btn_undo.h,
                can_undo?SDL_Color{120,120,180,255}:SDL_Color{60,60,80,255});
            SDL_Color tc = can_undo?SDL_Color{255,255,255,255}:SDL_Color{100,100,100,255};
            if (font_small) renderText(renderer, font_small, "↩ Z", btn_undo.x+4, btn_undo.y+11, tc);
        }
        // دکمه Redo (Ctrl+Y)
        {
            bool can_redo = !ui.redo_stack.empty();
            SDL_Color rc = can_redo ? SDL_Color{80,80,120,255} : SDL_Color{50,50,60,255};
            fillRect(renderer, btn_redo.x, btn_redo.y, btn_redo.w, btn_redo.h, rc);
            drawRect(renderer, btn_redo.x, btn_redo.y, btn_redo.w, btn_redo.h,
                can_redo?SDL_Color{120,120,180,255}:SDL_Color{60,60,80,255});
            SDL_Color tc = can_redo?SDL_Color{255,255,255,255}:SDL_Color{100,100,100,255};
            if (font_small) renderText(renderer, font_small, "↪ Y", btn_redo.x+4, btn_redo.y+11, tc);
        }
        // دکمه Turbo ⚡
        {
            bool turbo = rt.turbo_mode;
            SDL_Color tc = turbo ? SDL_Color{200,160,0,255} : SDL_Color{55,55,65,255};
            fillRect(renderer, btn_turbo.x, btn_turbo.y, btn_turbo.w, btn_turbo.h, tc);
            drawRect(renderer, btn_turbo.x, btn_turbo.y, btn_turbo.w, btn_turbo.h,
                turbo?SDL_Color{255,220,0,255}:SDL_Color{90,90,110,255});
            if (font_small) renderText(renderer, font_small,
                turbo ? "⚡ ON" : "⚡ OFF",
                btn_turbo.x+6, btn_turbo.y+11,
                turbo?SDL_Color{20,20,0,255}:SDL_Color{160,160,160,255});
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

        // دکمه "Add Extension" در پایین sidebar
        const int EXT_BTN_H = 32;
        {
            int btn_y = area_category.y + area_category.h - EXT_BTN_H - 2;
            fillRect(renderer, area_category.x+2, btn_y, CATEGORY_W-4, EXT_BTN_H,
                {40,40,80,220});
            drawRect(renderer, area_category.x+2, btn_y, CATEGORY_W-4, EXT_BTN_H,
                {80,80,140,255});
            if (font_small) renderText(renderer, font_small, "+ Ext",
                area_category.x+8, btn_y+9, {160,200,255,255});
        }

        int cat_h = (area_category.h - EXT_BTN_H - 4) / (int)CATEGORIES.size();
        cat_h = std::max(28, cat_h);
        for (int i = 0; i < (int)CATEGORIES.size(); ++i) {
            const auto& cat = CATEGORIES[i];
            int cy = area_category.y + i * cat_h;
            bool active = (cat.name == ui.active_category);

            //배경
            SDL_Color bg = active ? cat.color : SDL_Color{45,45,45,255};
            fillRect(renderer, area_category.x, cy, area_category.w, cat_h-1, bg);

            // متن (اول حرف)
            // اسم کامل category
            if (font_small) {
                renderText(renderer, font_small, cat.name,
                    area_category.x + 5, cy + cat_h/2 - 6,
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

            // دکمه "Make a Variable" در بالای Variables palette
            if (ui.active_category == "Variables") {
                SDL_Rect mkvar_btn = {area_palette.x+6, py, PALETTE_W-12, 28};
                fillRect(renderer, mkvar_btn.x, mkvar_btn.y, mkvar_btn.w, mkvar_btn.h,
                    {255,140,26,255});
                drawRect(renderer, mkvar_btn.x, mkvar_btn.y, mkvar_btn.w, mkvar_btn.h,
                    {180,100,0,255});
                if (font_small) renderText(renderer, font_small, "Make a Variable",
                    mkvar_btn.x+8, mkvar_btn.y+7, {255,255,255,255});
                py += 36;
                // نمایش متغیرهای موجود
                for (auto& var : project.variables) {
                    std::string vline = var.name + " = " + std::to_string((int)var.value);
                    fillRect(renderer, area_palette.x+6, py, PALETTE_W-12, 22,
                        {200,120,20,200});
                    if (font_small) renderText(renderer, font_small, vline,
                        area_palette.x+10, py+4, {255,255,255,255});
                    py += 26;
                    if (py > area_palette.y + area_palette.h - 30) break;
                }
                py += 8;
            }

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

            // رندر label — محدود به فضای قبل از input fields
            if (font_small) {
                std::string lbl = b.inputs.empty()
                    ? labelForBlockType(b.type)
                    : labelNoNumbers(b.type);

                // فضای label = عرض بلوک - فضای inputs - padding
                {
                    int n_inputs     = (int)b.inputs.size();
                    int fields_px    = n_inputs * (36 + 4 + 2); // FIELD_W+GAP+margin
                    int label_max_px = (int)b.width - fields_px - 18;
                    label_max_px = std::max(label_max_px, 12);
                    int max_chars = label_max_px / 8; // ~8px per char
                    if (max_chars > 0 && (int)lbl.size() > max_chars)
                        lbl = lbl.substr(0, std::max(max_chars, 1));
                }

                renderText(renderer, font_small, lbl,
                    (int)b.x + 8,
                    (int)b.y + (int)b.height/2 - 7,
                    {255,255,255,255});
            }

            // رندر input fields (از راست بلوک)
            {
                int input_count = (int)b.inputs.size();
                const int FIELD_W = 36;
                const int FIELD_H = 22;
                const int FIELD_GAP = 4;
                // کل فضای inputs از راست
                int total_fields_w = input_count * (FIELD_W + FIELD_GAP);
                for (int k = 0; k < input_count && k < 3; ++k) {
                    int field_x = (int)b.x + (int)b.width - total_fields_w + k*(FIELD_W+FIELD_GAP);
                    int field_y = (int)b.y + ((int)b.height - FIELD_H) / 2;

                    bool is_editing = (ui.editing_block_id == b.id && ui.editing_input_idx == k);

                    // پس‌زمینه field
                    SDL_Color field_bg = is_editing ? SDL_Color{255,255,200,255}
                                                    : SDL_Color{255,255,255,180};
                    fillRect(renderer, field_x, field_y, FIELD_W, FIELD_H, field_bg);
                    SDL_Color border_col = is_editing ? SDL_Color{255,120,0,255}
                                                      : SDL_Color{100,100,100,180};
                    drawRect(renderer, field_x, field_y, FIELD_W, FIELD_H, border_col);

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
        // به‌روز کردن stage coordinates برای event loop
        g_stage_x = stage_x; g_stage_y = stage_y;
        g_stage_w = stage_draw_w; g_stage_h = stage_draw_h;
        // آپدیت sensing با موقعیت ماوس فعلی (هر فریم)
        {
            int raw_mx=0, raw_my=0;
            Uint32 mbtn = SDL_GetMouseState(&raw_mx, &raw_my);
            float sx = (raw_mx - stage_x - stage_draw_w/2.0f) * 480.0f / stage_draw_w;
            float sy = -(raw_my - stage_y - stage_draw_h/2.0f) * 360.0f / stage_draw_h;
            sense_update_mouse(sensing, sx, sy, (mbtn & SDL_BUTTON(1)) != 0);
        }
        // رندر backdrop فعال
        {
            std::string bd_path = "";
            if (!project.backdrops.empty()) {
                int bi = project.active_backdrop_idx;
                if (bi >= 0 && bi < (int)project.backdrops.size())
                    bd_path = project.backdrops[bi].path;
            }
            // اگر backdrop path خالیه، از bg_texture اصلی استفاده کن
            SDL_Texture* bd_tex = bg_texture;
            if (!bd_path.empty())
                bd_tex = loadTexture(bd_path);
            if (bd_tex) {
                SDL_Rect bg_dst = {stage_x, stage_y, stage_draw_w, stage_draw_h};
                SDL_RenderCopy(renderer, bd_tex, nullptr, &bg_dst);
            } else {
                fillRect(renderer, stage_x, stage_y, stage_draw_w, stage_draw_h, {255,255,255,255});
            }
        }
        drawRect(renderer, stage_x, stage_y, stage_draw_w, stage_draw_h, {100,100,100,255});

        // ── رندر Pen Canvas (خطوط و stampها) ──────────────────────────────
        // تبدیل مختصات Scratch → pixel stage
        auto scratch_to_px = [&](float sx, float sy, int& px, int& py) {
            px = stage_x + stage_draw_w/2 + (int)(sx * stage_draw_w / 480.0f);
            py = stage_y + stage_draw_h/2 - (int)(sy * stage_draw_h / 360.0f);
        };

        // خطوط
        for (auto& stroke : project.pen_canvas.strokes) {
            int x1,y1,x2,y2;
            scratch_to_px(stroke.x1, stroke.y1, x1, y1);
            scratch_to_px(stroke.x2, stroke.y2, x2, y2);
            // رسم چند خط برای ضخامت
            int thickness = std::max(1, (int)stroke.size);
            SDL_SetRenderDrawColor(renderer, stroke.r, stroke.g, stroke.b, 255);
            for (int t = -thickness/2; t <= thickness/2; ++t) {
                SDL_RenderDrawLine(renderer, x1+t, y1,   x2+t, y2);
                SDL_RenderDrawLine(renderer, x1,   y1+t, x2,   y2+t);
            }
        }

        // stampها
        for (auto& st : project.pen_canvas.stamps) {
            int px, py;
            scratch_to_px(st.world_x, st.world_y, px, py);
            int sw2 = (int)(st.width  * st.size_percent / 100.0f);
            int sh2 = (int)(st.height * st.size_percent / 100.0f);
            sw2 = std::max(sw2, 8); sh2 = std::max(sh2, 8);
            SDL_Texture* st_tex = st.costume_path.empty()
                ? nullptr : loadTexture(st.costume_path);
            if (st_tex) {
                SDL_Rect dst = {px-sw2/2, py-sh2/2, sw2, sh2};
                SDL_RenderCopy(renderer, st_tex, nullptr, &dst);
            } else {
                fillRect(renderer, px-sw2/2, py-sh2/2, sw2, sh2,
                    {(Uint8)st.r, (Uint8)st.g, (Uint8)st.b, 180});
            }
        }

        // رندر spriteها
        // مختصات Scratch: x=0,y=0 مرکز stage. x از -240 تا +240، y از -180 تا +180
        // مختصات Stage روی صفحه: stage_x..stage_x+STAGE_W-20

        // ── رندر Clone ها ─────────────────────────────────────────────────
        for (auto& clone : rt.clones) {
            const Sprite& cspr = clone.data;
            if (!cspr.visible) continue;
            float px = stage_x + stage_draw_w/2.0f + cspr.x * stage_draw_w / 480.0f;
            float py = stage_y + stage_draw_h/2.0f - cspr.y * stage_draw_h / 360.0f;
            float sw = cspr.width  * cspr.size_percent/100.f * stage_draw_w/480.f;
            float sh = cspr.height * cspr.size_percent/100.f * stage_draw_h/360.f;
            SDL_Rect dr={(int)(px-sw/2),(int)(py-sh/2),(int)sw,(int)sh};
            SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,100,149,237,160);
            SDL_RenderFillRect(renderer,&dr);
            SDL_SetRenderDrawColor(renderer,60,100,200,220);
            SDL_RenderDrawRect(renderer,&dr);
            if (font_small)
                renderText(renderer,font_small,"*",
                    (int)px-3,(int)py-5,{200,220,255,255});
        }

        // ── رندر Sprite های اصلی ──────────────────────────────────────────
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

        // ── Backdrop Chooser Panel ───────────────────────────────────────────
        if (ui.show_backdrop_panel) {
            int bp_w = 340, bp_h = 300;
            int bp_x = stage_x, bp_y = stage_y + stage_draw_h - bp_h;
            // clamp
            if (bp_y < stage_y) bp_y = stage_y;

            // سایه
            fillRect(renderer, bp_x+4, bp_y+4, bp_w, bp_h, {0,0,0,80});
            // پس‌زمینه
            fillRect(renderer, bp_x, bp_y, bp_w, bp_h, {24,26,38,250});
            drawRect(renderer, bp_x, bp_y, bp_w, bp_h, {80,100,160,255});

            // نوار عنوان
            fillRect(renderer, bp_x, bp_y, bp_w, 32, {36,50,90,255});
            if (font_bold) renderText(renderer,font_bold,"Backdrops",
                bp_x+10,bp_y+8,{255,255,255,255});
            // دکمه X
            fillRect(renderer,bp_x+bp_w-30,bp_y+4,24,24,{160,40,40,255});
            if (font_bold) renderText(renderer,font_bold,"X",
                bp_x+bp_w-22,bp_y+8,{255,255,255,255});

            // لیست backdrop ها
            int by = bp_y+38;
            for (int bi=0; bi<(int)project.backdrops.size(); ++bi) {
                auto& bd = project.backdrops[bi];
                bool active = (bi == project.active_backdrop_idx);
                SDL_Color row_c = active
                    ? SDL_Color{50,80,160,255} : SDL_Color{36,40,58,255};
                fillRect(renderer, bp_x+6, by, bp_w-12, 36, row_c);
                drawRect(renderer, bp_x+6, by, bp_w-12, 36,
                    active?SDL_Color{100,150,255,255}:SDL_Color{60,70,100,255});
                // پیش‌نمایش رنگ (placeholder)
                SDL_Color prev_c = {(Uint8)(40+bi*40),(Uint8)(80+bi*30),(Uint8)(180-bi*20),255};
                fillRect(renderer, bp_x+10, by+4, 44, 28, prev_c);
                drawRect(renderer, bp_x+10, by+4, 44, 28, {200,200,200,80});
                // نام
                if (font_small) renderText(renderer,font_small,bd.name,
                    bp_x+62,by+12,active?SDL_Color{255,255,255,255}:SDL_Color{180,190,210,255});
                // آیکون active
                if (active && font_small)
                    renderText(renderer,font_small,"✓",bp_x+bp_w-26,by+12,{100,220,100,255});
                by += 40;
                if (by > bp_y+bp_h-60) break;
            }

            // دکمه "Add Backdrop"
            {
                int ab_y = bp_y+bp_h-36;
                fillRect(renderer,bp_x+6,ab_y,bp_w-12,28,{40,80,50,255});
                drawRect(renderer,bp_x+6,ab_y,bp_w-12,28,{60,160,80,255});
                if(font_small) renderText(renderer,font_small,"+ Add Backdrop",
                    bp_x+bp_w/2-55,ab_y+7,{180,240,180,255});
            }
        }

        // ── Costume Editor Panel ──────────────────────────────────────────────
        if (ui.show_costume_editor) {
            // sprite فعال
            Sprite* act_spr = nullptr;
            for (auto& s : project.sprites) if (s.id==ui.active_sprite_id){ act_spr=&s; break; }

            int cp_w=300, cp_h=280;
            int cp_x=stage_x+stage_draw_w-cp_w, cp_y=stage_y;

            // سایه
            fillRect(renderer,cp_x+4,cp_y+4,cp_w,cp_h,{0,0,0,80});
            // پس‌زمینه
            fillRect(renderer,cp_x,cp_y,cp_w,cp_h,{26,24,40,250});
            drawRect(renderer,cp_x,cp_y,cp_w,cp_h,{100,80,160,255});

            // عنوان
            fillRect(renderer,cp_x,cp_y,cp_w,32,{50,36,90,255});
            std::string ct_title = "Costumes";
            if (act_spr) ct_title += " - " + act_spr->name;
            if(font_bold) renderText(renderer,font_bold,ct_title,cp_x+8,cp_y+8,{255,255,255,255});
            // X
            fillRect(renderer,cp_x+cp_w-30,cp_y+4,24,24,{160,40,40,255});
            if(font_bold) renderText(renderer,font_bold,"X",cp_x+cp_w-22,cp_y+8,{255,255,255,255});

            if (act_spr) {
                int cy2 = cp_y+38;
                for (int ci=0; ci<(int)act_spr->costumes.size(); ++ci) {
                    auto& cos = act_spr->costumes[ci];
                    bool active = (ci==act_spr->costume_index);
                    SDL_Color row_c=active?SDL_Color{80,50,160,255}:SDL_Color{38,36,58,255};
                    fillRect(renderer,cp_x+6,cy2,cp_w-12,40,row_c);
                    drawRect(renderer,cp_x+6,cy2,cp_w-12,40,
                        active?SDL_Color{160,120,255,255}:SDL_Color{70,60,100,255});
                    // preview
                    SDL_Color pc={(Uint8)(180-ci*30),(Uint8)(100+ci*40),(Uint8)(200+ci*10),255};
                    fillRect(renderer,cp_x+10,cy2+4,36,32,pc);
                    drawRect(renderer,cp_x+10,cy2+4,36,32,{200,200,200,60});
                    // نام
                    if(font_small) renderText(renderer,font_small,cos.name,
                        cp_x+54,cy2+8,active?SDL_Color{255,255,255,255}:SDL_Color{180,180,210,255});
                    // شماره
                    if(font_small) renderText(renderer,font_small,
                        "#"+std::to_string(ci+1),cp_x+54,cy2+22,{120,120,150,255});
                    if(active && font_small)
                        renderText(renderer,font_small,"✓",cp_x+cp_w-24,cy2+14,{140,220,140,255});
                    cy2+=44;
                    if(cy2>cp_y+cp_h-50) break;
                }
                // دکمه Add Costume
                int ac_y=cp_y+cp_h-36;
                fillRect(renderer,cp_x+6,ac_y,cp_w-12,28,{50,40,80,255});
                drawRect(renderer,cp_x+6,ac_y,cp_w-12,28,{120,80,200,255});
                if(font_small) renderText(renderer,font_small,"+ Add Costume",
                    cp_x+cp_w/2-50,ac_y+7,{200,180,255,255});
            }
        }

        // ── Extensions Panel ─────────────────────────────────────────────────
        if (ui.show_extensions_panel) {
            // ── dim overlay ──
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0,0,0,120);
            SDL_Rect overlay={0,0,WINDOW_W,WINDOW_H};
            SDL_RenderFillRect(renderer,&overlay);
            // blend mode restored automatically

            int ep_w=380, ep_h=(int)EXTENSIONS.size()*70+70;
            int ep_x=WINDOW_W/2-ep_w/2, ep_y=WINDOW_H/2-ep_h/2;
            // پس‌زمینه panel
            fillRect(renderer,ep_x,ep_y,ep_w,ep_h,{28,28,38,255});
            drawRect(renderer,ep_x,ep_y,ep_w,ep_h,{90,90,120,255});
            // نوار عنوان
            fillRect(renderer,ep_x,ep_y,ep_w,36,{40,40,60,255});
            if(font_bold) renderText(renderer,font_bold,"Add Extension",
                ep_x+14,ep_y+9,{255,255,255,255});
            // دکمه X
            fillRect(renderer,ep_x+ep_w-34,ep_y+6,26,24,{180,50,50,255});
            drawRect(renderer,ep_x+ep_w-34,ep_y+6,26,24,{255,80,80,255});
            if(font_bold) renderText(renderer,font_bold,"X",
                ep_x+ep_w-24,ep_y+9,{255,255,255,255});

            // لیست extensions
            int ey=ep_y+44;
            for(int ei=0;ei<(int)EXTENSIONS.size();++ei){
                auto& ext=EXTENSIONS[ei];
                int row_h=64;
                // پس‌زمینه ردیف
                SDL_Color row_bg=ext.enabled
                    ? SDL_Color{(Uint8)std::min(255,(int)ext.color.r/3+20),
                                (Uint8)std::min(255,(int)ext.color.g/3+20),
                                (Uint8)std::min(255,(int)ext.color.b/3+20),255}
                    : SDL_Color{42,42,55,255};
                fillRect(renderer,ep_x+8,ey,ep_w-16,row_h-4,row_bg);
                drawRect(renderer,ep_x+8,ey,ep_w-16,row_h-4,
                    ext.enabled?ext.color:SDL_Color{70,70,85,255});
                // آیکون مربع رنگی
                fillRect(renderer,ep_x+16,ey+10,44,44,ext.color);
                drawRect(renderer,ep_x+16,ey+10,44,44,{255,255,255,60});
                // نام (بزرگ)
                if(font_bold) renderText(renderer,font_bold,ext.name,
                    ep_x+68,ey+10,{255,255,255,255});
                // توضیح (کوچک)
                if(font_small) renderText(renderer,font_small,ext.description,
                    ep_x+68,ey+28,{160,170,180,255});
                // دکمه Add/Added
                int btn_x=ep_x+ep_w-90, btn_y=ey+16, btn_w=74, btn_h=28;
                SDL_Color btn_col=ext.enabled
                    ? SDL_Color{40,160,40,255}
                    : SDL_Color{60,120,220,255};
                fillRect(renderer,btn_x,btn_y,btn_w,btn_h,btn_col);
                drawRect(renderer,btn_x,btn_y,btn_w,btn_h,{255,255,255,60});
                std::string btn_lbl=ext.enabled?"✓ Added":"Add";
                if(font_small) renderText(renderer,font_small,btn_lbl,
                    btn_x+(ext.enabled?8:20),btn_y+7,{255,255,255,255});
                ey+=row_h;
            }
        }

        // ── Right-click Context Menu ─────────────────────────────────────────
        if (ui.show_context_menu && ui.ctx_block_id >= 0) {
            struct CtxItem { const char* label; SDL_Color color; };
            static const CtxItem CM[] = {
                {"Duplicate block", {70, 140, 220, 255}},
                {"Delete block",    {200,  60,  60, 255}},
                {"Add comment",     {80,  160,  80, 255}},
                {"Disable block",   {140, 100,  40, 255}},
                {"Clean up blocks", {100,  80, 160, 255}},
            };
            const int N=5, CM_W=172, CM_H=28;
            int cmx = ui.ctx_menu_x, cmy = ui.ctx_menu_y;
            // clamp به داخل window
            if (cmx+CM_W > WINDOW_W) cmx = WINDOW_W-CM_W-4;
            if (cmy+N*CM_H > WINDOW_H) cmy = WINDOW_H-N*CM_H-4;

            // سایه
            fillRect(renderer, cmx+3, cmy+3, CM_W, N*CM_H, {0,0,0,80});
            // پس‌زمینه کل
            fillRect(renderer, cmx, cmy, CM_W, N*CM_H, {28,30,42,248});
            drawRect(renderer, cmx, cmy, CM_W, N*CM_H, {80,90,120,255});

            // hover تشخیص
            int hov_mx, hov_my;
            SDL_GetMouseState(&hov_mx, &hov_my);

            for (int ci=0; ci<N; ++ci) {
                SDL_Rect ir = {cmx, cmy+ci*CM_H, CM_W, CM_H};
                bool hov = pointInRect(hov_mx, hov_my, ir);
                // پس‌زمینه hover
                if (hov) fillRect(renderer, ir.x, ir.y, ir.w, ir.h, {50,55,75,255});
                // خط جداکننده
                if (ci>0) {
                    SDL_SetRenderDrawColor(renderer,60,65,85,255);
                    SDL_RenderDrawLine(renderer,cmx,cmy+ci*CM_H,cmx+CM_W,cmy+ci*CM_H);
                }
                // آیکون رنگی کوچک
                fillRect(renderer, cmx+4, cmy+ci*CM_H+8, 12, 12, CM[ci].color);
                // متن
                if (font_small)
                    renderText(renderer, font_small, CM[ci].label,
                        cmx+20, cmy+ci*CM_H+8,
                        hov?SDL_Color{255,255,255,255}:SDL_Color{200,205,215,255});
            }
        }

        // ── Ask & Answer Dialog ──────────────────────────────────────────────
        if (rt.ask_active) {
            // نمایش input box روی stage
            int aw = stage_draw_w - 40;
            int ax = stage_x + 20;
            int ay = stage_y + stage_draw_h - 60;

            // پس‌زمینه
            fillRect(renderer, ax, ay, aw, 48, {240,240,255,245});
            drawRect(renderer, ax, ay, aw, 48, {80,120,220,255});

            // دکمه ارسال (✓)
            int submit_w = 40;
            fillRect(renderer, ax+aw-submit_w-2, ay+4, submit_w, 40,
                {80,160,80,255});
            drawRect(renderer, ax+aw-submit_w-2, ay+4, submit_w, 40,
                {60,200,60,255});
            if (font_bold) renderText(renderer, font_bold, "✓",
                ax+aw-submit_w+8, ay+12, {255,255,255,255});

            // متن تایپ‌شده
            std::string display = rt.ask_buffer;
            if ((SDL_GetTicks()/500)%2==0) display += "|";  // cursor چشمک‌زن
            fillRect(renderer, ax+4, ay+4, aw-submit_w-12, 40, {255,255,255,255});
            drawRect(renderer, ax+4, ay+4, aw-submit_w-12, 40, {160,180,230,255});
            if (font_small && !display.empty())
                renderText(renderer, font_small, display,
                    ax+8, ay+14, {20,20,80,255});
            else if (font_small)
                renderText(renderer, font_small, "Type your answer...",
                    ax+8, ay+14, {160,160,200,255});
        }

        // ── Variable Dialog ──────────────────────────────────────────────────
        if (ui.show_var_dialog) {
            int dlg_w = 300, dlg_h = 120;
            int dlg_x = WINDOW_W/2 - dlg_w/2;
            int dlg_y = WINDOW_H/2 - dlg_h/2;
            fillRect(renderer, dlg_x, dlg_y, dlg_w, dlg_h, {50,50,60,240});
            drawRect(renderer, dlg_x, dlg_y, dlg_w, dlg_h, {100,100,120,255});
            if (font_bold)  renderText(renderer, font_bold,  "New Variable",
                dlg_x+10, dlg_y+10, {255,255,255,255});
            if (font_small) renderText(renderer, font_small, "Name:",
                dlg_x+10, dlg_y+40, {200,200,200,255});
            // input box
            fillRect(renderer, dlg_x+60, dlg_y+36, 180, 24, {255,255,255,255});
            if (font_small) renderText(renderer, font_small, ui.new_var_name,
                dlg_x+64, dlg_y+40, {0,0,0,255});
            // OK / Cancel
            fillRect(renderer, dlg_x+60,  dlg_y+80, 70, 26, {60,160,60,255});
            fillRect(renderer, dlg_x+140, dlg_y+80, 70, 26, {160,60,60,255});
            if (font_small) renderText(renderer, font_small, "OK",
                dlg_x+85,  dlg_y+85, {255,255,255,255});
            if (font_small) renderText(renderer, font_small, "Cancel",
                dlg_x+148, dlg_y+85, {255,255,255,255});
        }

        // ── دکمه Backdrop chooser (پایین-چپ stage) ──────────────────────────
        {
            SDL_Rect bd_btn = {stage_x, stage_y + stage_draw_h + 4, 90, 22};
            bool bd_hov = false;
            {
                int hx,hy; SDL_GetMouseState(&hx,&hy);
                bd_hov = pointInRect(hx, hy, bd_btn);
            }
            fillRect(renderer, bd_btn.x, bd_btn.y, bd_btn.w, bd_btn.h,
                ui.show_backdrop_panel ? SDL_Color{60,120,220,255}
                : (bd_hov ? SDL_Color{50,80,160,255} : SDL_Color{35,45,70,255}));
            drawRect(renderer, bd_btn.x, bd_btn.y, bd_btn.w, bd_btn.h, {80,110,200,255});
            if (font_small)
                renderText(renderer, font_small, "🎨 Backdrop",
                    bd_btn.x+4, bd_btn.y+5, {200,220,255,255});
        }
        // ── دکمه Costume editor (کنار Backdrop) ──────────────────────────────
        {
            SDL_Rect cs_btn = {stage_x+96, stage_y + stage_draw_h + 4, 90, 22};
            bool cs_hov = false;
            { int hx,hy; SDL_GetMouseState(&hx,&hy);
              cs_hov = pointInRect(hx, hy, cs_btn); }
            fillRect(renderer, cs_btn.x, cs_btn.y, cs_btn.w, cs_btn.h,
                ui.show_costume_editor ? SDL_Color{120,60,220,255}
                : (cs_hov ? SDL_Color{90,50,160,255} : SDL_Color{45,35,70,255}));
            drawRect(renderer, cs_btn.x, cs_btn.y, cs_btn.w, cs_btn.h, {120,80,200,255});
            if (font_small)
                renderText(renderer, font_small, "👔 Costumes",
                    cs_btn.x+4, cs_btn.y+5, {220,200,255,255});
        }

        // ── Clone counter و Turbo indicator روی stage ────────────────────────
        if (!rt.clones.empty()) {
            std::string ci = std::to_string(rt.clones.size()) + " clones";
            int ciw = (int)ci.size()*7+8;
            fillRect(renderer,stage_x+stage_draw_w-ciw-4,stage_y+stage_draw_h-20,ciw,16,{40,80,180,210});
            if(font_small) renderText(renderer,font_small,ci,
                stage_x+stage_draw_w-ciw,stage_y+stage_draw_h-18,{200,220,255,255});
        }
        if (rt.turbo_mode) {
            fillRect(renderer,stage_x+2,stage_y+2,62,16,{160,120,0,210});
            if(font_small) renderText(renderer,font_small,"⚡ TURBO",
                stage_x+4,stage_y+3,{255,240,80,255});
        }

        // ── نمایش Variable Monitor ها روی stage (draggable) ────────────────
        for (int vi = 0; vi < (int)project.variables.size(); ++vi) {
            auto& var = project.variables[vi];
            if (!var.visible) continue;

            // موقعیت monitor: stage_x + offset ذخیره‌شده در var
            int mon_x = stage_x + (int)var.monitor_x;
            int mon_y = stage_y + (int)var.monitor_y;

            // محاسبه عرض بر اساس محتوا
            char val_buf[32];
            snprintf(val_buf, sizeof(val_buf), "%.4g", var.value);
            int name_w  = (int)var.name.size() * 8;
            int val_w   = (int)strlen(val_buf) * 8;
            int mon_w   = name_w + val_w + 28;
            int mon_h   = 22;

            // Scratch-style monitor: نام با پس‌زمینه آبی + مقدار با پس‌زمینه سفید
            // بخش نام
            fillRect(renderer, mon_x,        mon_y, name_w+12, mon_h, {76,130,218,255});
            // بخش مقدار
            fillRect(renderer, mon_x+name_w+12, mon_y, val_w+16, mon_h, {255,255,255,220});
            drawRect(renderer, mon_x,        mon_y, mon_w,       mon_h, {50,100,180,255});

            if (font_small) {
                // نام (سفید روی آبی)
                renderText(renderer, font_small, var.name,
                    mon_x+4, mon_y+4, {255,255,255,255});
                // مقدار (تیره روی سفید)
                renderText(renderer, font_small, std::string(val_buf),
                    mon_x+name_w+16, mon_y+4, {20,20,60,255});
            }

            // هایلایت اگه در حال drag است
            if (ui.dragging_var_idx == vi) {
                drawRect(renderer, mon_x-2, mon_y-2, mon_w+4, mon_h+4, {255,200,50,255});
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
                int info_x  = panel_x + 8;
                int info_y2 = panel_y + SPRITE_THUMB_H + 10;
                const int LH = 15;  // line height

                // ردیف 1: نام
                renderText(renderer, font_small, active_spr->name,
                    info_x, info_y2, {220,220,220,255});
                // ردیف 2: x, y
                std::string xy_str =
                    "x:" + std::to_string((int)active_spr->x) +
                    "  y:" + std::to_string((int)active_spr->y);
                renderText(renderer, font_small, xy_str,
                    info_x, info_y2 + LH, {160,220,160,255});
                // ردیف 3: dir و size
                std::string ds_str =
                    "dir:" + std::to_string((int)active_spr->direction) +
                    "  sz:" + std::to_string((int)active_spr->size_percent) + "%";
                renderText(renderer, font_small, ds_str,
                    info_x, info_y2 + LH*2, {160,160,220,255});
                // ردیف 4: pen status
                {
                    auto& ps = active_spr->pen_state;
                    std::string pen_str = ps.is_down ? "Pen: DOWN" : "Pen: up";
                    SDL_Color pen_col = ps.is_down
                        ? SDL_Color{(Uint8)ps.r,(Uint8)ps.g,(Uint8)ps.b,255}
                        : SDL_Color{120,120,120,255};
                    renderText(renderer, font_small, pen_str,
                        info_x, info_y2 + LH*3, pen_col);
                    if (ps.is_down) {
                        fillRect(renderer, info_x+68, info_y2+LH*3, 12, 12,
                            {(Uint8)ps.r,(Uint8)ps.g,(Uint8)ps.b,255});
                    }
                }
                // وضعیت قلم
                auto& ps = active_spr->pen_state;
                std::string pen_str = std::string("Pen: ") +
                    (ps.is_down ? "DOWN" : "up") +
                    "  size:" + std::to_string((int)ps.size);
                SDL_Color pen_col = ps.is_down
                    ? SDL_Color{(Uint8)ps.r,(Uint8)ps.g,(Uint8)ps.b,255}
                    : SDL_Color{100,100,100,255};
                renderText(renderer, font_small, pen_str,
                    info_x, info_y2 + 14, pen_col);

                // نقطه رنگ قلم
                if (ps.is_down) {
                    fillRect(renderer, info_x+90, info_y2+12, 14, 14,
                        {(Uint8)ps.r,(Uint8)ps.g,(Uint8)ps.b,255});
                    drawRect(renderer, info_x+90, info_y2+12, 14, 14,
                        {200,200,200,255});
                }

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
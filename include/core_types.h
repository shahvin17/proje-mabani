#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <string>
#include <vector>
#include "pen.h"

using namespace std;

// ── Block ─────────────────────────────────────────────────────────────────────
struct Block {
    int    id           = -1;
    string type;
    vector<int> inputs;
    int    nextBlockId  = -1;
    float  x = 0, y = 0;
    float  width  = 200.0f;
    float  height = 60.0f;
    int    sprite_owner = -1;  // ← کدام sprite این بلوک را دارد (-1 = مشترک)
};

// ── Costume ───────────────────────────────────────────────────────────────────
struct Costume { string name, path; };

// ── Backdrop ──────────────────────────────────────────────────────────────────
struct Backdrop { string name, path; };

// ── Sprite ───────────────────────────────────────────────────────────────────
struct Sprite {
    int    id           = -1;
    string name;
    float  x = 0, y = 0;
    float  direction    = 90.0f;
    bool   visible      = true;
    float  width        = 50.0f;
    float  height       = 50.0f;
    bool   draggable    = false;

    // Looks
    string say_message  = "";
    float  say_timer    = 0.0f;
    float  size_percent = 100.0f;

    // Costumes
    vector<Costume> costumes;
    int    costume_index = 0;
    string costume_path  = "";

    // Script: when_start block id مربوط به این sprite
    int    when_start_id = -1;

    PenState pen_state;
};

// ── Variable ─────────────────────────────────────────────────────────────────
struct Variable {
    string name;
    float  value   = 0.0f;
    bool   visible = true;
    // Monitor روی stage
    float  monitor_x    = 8.0f;   // موقعیت monitor روی stage (pixel)
    float  monitor_y    = 8.0f;
    bool   monitor_drag = false;  // در حال drag است؟
};

// ── Project ──────────────────────────────────────────────────────────────────
struct Project {
    vector<Sprite>   sprites;
    vector<Block>    blocks;
    vector<Variable> variables;
    vector<Backdrop> backdrops;
    int  active_backdrop_idx = 0;
    bool isModified          = false;
    PenCanvas pen_canvas;
};

// ── SoundAsset ───────────────────────────────────────────────────────────────
struct SoundAsset { string name, path; };

// ── Undo/Redo Action ─────────────────────────────────────────────────────────
enum class ActionType {
    BLOCK_MOVE,       // بلوک جا به جا شد
    BLOCK_ADD,        // بلوک اضافه شد
    BLOCK_DELETE,     // بلوک حذف شد
    BLOCK_SNAP,       // دو بلوک snap شدن
    BLOCK_UNSNAP,     // snap شکسته شد
    INPUT_CHANGE,     // مقدار input عوض شد
    SPRITE_ADD,       // sprite اضافه شد
    SPRITE_DELETE,    // sprite حذف شد
};

struct UndoAction {
    ActionType type;
    // اطلاعات کافی برای undo/redo
    int   block_id     = -1;
    int   sprite_id    = -1;
    float old_x        = 0, old_y = 0;
    float new_x        = 0, new_y = 0;
    int   old_next_id  = -1;
    int   new_next_id  = -1;
    int   old_input_val = 0;
    int   new_input_val = 0;
    int   input_idx    = 0;
    // برای add/delete: snapshot کامل
    Block   saved_block;
    Sprite  saved_sprite;
};

#endif
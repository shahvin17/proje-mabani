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
    string str_param    = "";   // برای broadcast name، comment text، key name
    int    nextBlockId  = -1;
    float  x = 0, y = 0;
    float  width  = 200.0f;
    float  height = 60.0f;
    int    sprite_owner = -1;
    bool   disabled     = false;  // برای Disable block
    string comment      = "";     // برای Add Comment
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

    // Script: when_start block id
    int    when_start_id    = -1;
    // Multiple event triggers: list of (type, block_id) pairs
    // e.g. {"when_key_pressed", 42}, {"when_clicked", 55}
    struct EventHook { string event_type; string param; int block_id; };
    vector<EventHook> event_hooks;

    PenState pen_state;
};

// ── Variable ─────────────────────────────────────────────────────────────────
struct Variable {
    string name;
    float  value   = 0.0f;
    bool   visible = true;
    float  monitor_x = 8.0f;
    float  monitor_y = 8.0f;
    bool   monitor_drag = false;
};

// ── List variable ─────────────────────────────────────────────────────────────
struct ListVar {
    string         name;
    vector<string> items;
    bool   visible   = true;
    float  monitor_x = 8.0f;
    float  monitor_y = 40.0f;
    float  monitor_w = 100.0f;
    float  monitor_h = 160.0f;
};

// ── Broadcast message ────────────────────────────────────────────────────────
struct BroadcastDef {
    string name;   // e.g. "game over"
    int    id;
};

// ── Project ──────────────────────────────────────────────────────────────────
struct Project {
    vector<Sprite>       sprites;
    vector<Block>        blocks;
    vector<Variable>     variables;
    vector<ListVar>      lists;
    vector<Backdrop>     backdrops;
    vector<BroadcastDef> broadcasts;
    int  active_backdrop_idx = 0;
    bool isModified          = false;
    PenCanvas pen_canvas;
};

// ── SoundAsset ───────────────────────────────────────────────────────────────
struct SoundAsset { string name, path; };

// ── Undo/Redo Action ─────────────────────────────────────────────────────────
enum class ActionType {
    BLOCK_MOVE, BLOCK_ADD, BLOCK_DELETE, BLOCK_SNAP, BLOCK_UNSNAP,
    INPUT_CHANGE, SPRITE_ADD, SPRITE_DELETE,
};
struct UndoAction {
    ActionType type;
    int   block_id = -1, sprite_id = -1;
    float old_x=0, old_y=0, new_x=0, new_y=0;
    int   old_next_id=-1, new_next_id=-1;
    int   old_input_val=0, new_input_val=0, input_idx=0;
    Block  saved_block;
    Sprite saved_sprite;
};

#endif
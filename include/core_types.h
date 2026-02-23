#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <string>
#include <vector>
#include "pen.h"

using namespace std;

struct Block {
    int id;
    string type;
    vector<int> inputs;
    int nextBlockId;
    float x, y;
    float width  = 200.0f;
    float height = 60.0f;
};

// ── Costume ──────────────────────────────────────────────────────────────────
struct Costume {
    string name;
    string path;   // مسیر PNG
};

// ── Backdrop ─────────────────────────────────────────────────────────────────
struct Backdrop {
    string name;
    string path;
};

// ── Sprite ───────────────────────────────────────────────────────────────────
struct Sprite {
    int id;
    string name;
    float x = 0.0f, y = 0.0f;
    float direction  = 90.0f;
    bool  visible    = true;
    float width      = 50.0f;
    float height     = 50.0f;
    bool  draggable  = false;

    // Looks
    string say_message  = "";
    float  say_timer    = 0.0f;
    float  size_percent = 100.0f;

    // Costumes
    vector<Costume> costumes;
    int    costume_index = 0;
    string costume_path  = "";   // backward compat = costumes[costume_index].path

    PenState pen_state;
};

// ── Variable ─────────────────────────────────────────────────────────────────
struct Variable {
    string name;
    float  value   = 0.0f;
    bool   visible = true;
};

// ── Project ──────────────────────────────────────────────────────────────────
struct Project {
    vector<Sprite>   sprites;
    vector<Block>    blocks;
    vector<Variable> variables;
    vector<Backdrop> backdrops;
    int  active_backdrop_idx = 0;
    bool isModified = false;
    PenCanvas pen_canvas;
};

#endif
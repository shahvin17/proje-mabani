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

struct Sprite {
    int id;
    string name;
    float x = 0.0f;
    float y = 0.0f;
    float direction = 90.0f;
    bool  visible   = true;
    float width     = 50.0f;
    float height    = 50.0f;
    bool  draggable = false;

    // --- Looks ---
    string say_message  = "";
    float  say_timer    = 0.0f;   // -1 = نامحدود, 0 = خاموش
    float  size_percent = 100.0f;

    PenState pen_state;
};

struct Project {
    vector<Sprite> sprites;
    vector<Block>  blocks;
    bool isModified = false;
};

#endif
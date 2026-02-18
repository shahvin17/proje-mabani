#ifndef CORE_TYPES_H
#define CORE_TYPES_H

#include <vector>
#include <string>
using namespace std;

struct Block {
    int id;
    string type;              // فعلاً string نگه می‌داریم
    vector<int> inputs;
    int nextBlockId;
    float x, y;
};

struct Sprite {
    int id;
    string name;
    float x, y;
    float direction;
    bool visible;
};

struct Project {
    vector<Sprite> sprites;
    vector<Block> blocks;
    bool isModified = false;
};

#endif
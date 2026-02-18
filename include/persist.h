#ifndef PERSIST_H
#define PERSIST_H
#include <iostream>
using namespace std;
#include <fstream>
#include <vector>
#include <string>
struct Block {
    int id;
    string type;
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
bool saveProject(const Project& project, const string& path);

bool loadProject(Project& project, const string& path);
bool saveProjectAndMark(Project& project, const string& path);

#endif
#include "persist.h"
#include <fstream>

bool saveProject(const Project& project, const string& path) {
    ofstream file(path);
    if (!file.is_open())
        return false;
    file << "FOP1" << endl;
    file << project.sprites.size() << endl;
    for (const auto& s : project.sprites) {
        file << s.id << " " << s.name << " "
             << s.x << " " << s.y << " "
             << s.direction << " " << s.visible << endl;
    }

    file << project.blocks.size() << endl;
    for (const auto& b : project.blocks) {
        file << b.id << " " << b.type << " "
             << b.x << " " << b.y << " "
             << b.nextBlockId << " "
             << b.inputs.size();

        for (int inId : b.inputs) file << " " << inId;
        file << endl;
    }

    if (!file.good()) return false;
    return true;
}

bool loadProject(Project& project, const string& path) {
    ifstream file(path);
    if (!file.is_open())
        return false;
    string ver;
    file >> ver;
    if (ver != "FOP1") return false;
    project.sprites.clear();
    project.blocks.clear();
    size_t spriteCount;
    file >> spriteCount;

    for (size_t i = 0; i < spriteCount; i++) {
        Sprite s;
        file >> s.id >> s.name >> s.x >> s.y >> s.direction >> s.visible;
        project.sprites.push_back(s);
    }

    size_t blockCount;
    file >> blockCount;

    for (size_t i = 0; i < blockCount; i++) {
        Block b;
        size_t inputsCount;

        file >> b.id >> b.type >> b.x >> b.y >> b.nextBlockId >> inputsCount;

        b.inputs.clear();
        b.inputs.reserve(inputsCount);

        for (size_t k = 0; k < inputsCount; k++) {
            int inId;
            file >> inId;
            b.inputs.push_back(inId);
        }

        project.blocks.push_back(b);
    }

    if (!file.good()) return false;

    project.isModified = false;
    return true;
}

void newProject(Project& project) {
    project.sprites.clear();
    project.blocks.clear();
    project.isModified = false;
}
bool saveProjectAndMark(Project& project, const string& path) {
    if (saveProject(project, path)) {
        project.isModified = false;


        return true;
    }
    return false;
}
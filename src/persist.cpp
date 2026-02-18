#include "persist.h"
#include <fstream>

bool saveProject(const Project& project, const string& path) {
    ofstream file(path);
    if (!file.is_open())
        return false;
    file << "FOP1" << endl;

    file << project.sprites.size() <<endl;

    for (const auto& s : project.sprites) {
        file << s.id << " " << s.name << " "
             << s.x << " " << s.y << " "
             << s.direction << " " << s.visible <<endl;
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

    size_t count;
    file >> count;

    for (size_t i = 0; i < count; i++) {
        Sprite s;
        file >> s.id >> s.name >> s.x >> s.y >> s.direction >> s.visible;
        project.sprites.push_back(s);
    }

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


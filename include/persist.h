#ifndef PERSIST_H
#define PERSIST_H

#include "core_types.h"
#include <fstream>
using namespace std;

bool saveProject(const Project& project, const string& path);
bool loadProject(Project& project, const string& path);
bool saveProjectAndMark(Project& project, const string& path);
void newProject(Project& project);

#endif
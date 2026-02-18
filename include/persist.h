
struct Project {
    vector<Sprite> sprites;
    vector<Block> blocks;
    bool isModified = false;
};
bool saveProject(const Project& project, const string& path);

bool loadProject(Project& project, const string& path);
bool saveProjectAndMark(Project& project, const string& path);

#endif
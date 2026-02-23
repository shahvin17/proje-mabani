#include "persist.h"
#include "core_types.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

// ─── Simple JSON Writer ───────────────────────────────────────────────────────
static std::string escStr(const std::string& s) {
    std::string r; r.reserve(s.size()+2); r+='"';
    for (char c : s) {
        if      (c=='"')  r+="\\\"";
        else if (c=='\\') r+="\\\\";
        else if (c=='\n') r+="\\n";
        else              r+=c;
    }
    r+='"'; return r;
}

bool saveProject(const Project& proj, const std::string& path) {
    std::ofstream f(path);
    if (!f) { logError("Save: cannot open " + path); return false; }

    f << "{\n";

    // ── Sprites ──
    f << "  \"sprites\": [\n";
    for (size_t i = 0; i < proj.sprites.size(); ++i) {
        const auto& s = proj.sprites[i];
        f << "    {\n";
        f << "      \"id\": "          << s.id             << ",\n";
        f << "      \"name\": "        << escStr(s.name)   << ",\n";
        f << "      \"x\": "           << s.x              << ",\n";
        f << "      \"y\": "           << s.y              << ",\n";
        f << "      \"direction\": "   << s.direction      << ",\n";
        f << "      \"visible\": "     << (s.visible?"true":"false") << ",\n";
        f << "      \"size_percent\": "<< s.size_percent   << ",\n";
        f << "      \"costume_path\": "<< escStr(s.costume_path) << "\n";
        f << "    }";
        if (i+1 < proj.sprites.size()) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // ── Blocks ──
    f << "  \"blocks\": [\n";
    for (size_t i = 0; i < proj.blocks.size(); ++i) {
        const auto& b = proj.blocks[i];
        f << "    {\n";
        f << "      \"id\": "          << b.id               << ",\n";
        f << "      \"type\": "        << escStr(b.type)     << ",\n";
        f << "      \"nextBlockId\": " << b.nextBlockId      << ",\n";
        f << "      \"x\": "           << b.x                << ",\n";
        f << "      \"y\": "           << b.y                << ",\n";
        f << "      \"width\": "       << b.width            << ",\n";
        f << "      \"height\": "      << b.height           << ",\n";
        f << "      \"inputs\": [";
        for (size_t j = 0; j < b.inputs.size(); ++j) {
            f << b.inputs[j];
            if (j+1 < b.inputs.size()) f << ",";
        }
        f << "]\n";
        f << "    }";
        if (i+1 < proj.blocks.size()) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // ── Variables ──
    f << "  \"variables\": [\n";
    for (size_t i = 0; i < proj.variables.size(); ++i) {
        const auto& v = proj.variables[i];
        f << "    {\"name\": " << escStr(v.name)
          << ", \"value\": " << v.value
          << ", \"visible\": " << (v.visible?"true":"false")
          << "}";
        if (i+1 < proj.variables.size()) f << ",";
        f << "\n";
    }
    f << "  ]\n";

    f << "}\n";
    logInfo("Project saved to: " + path);
    return true;
}

// ─── Simple JSON Parser ───────────────────────────────────────────────────────
// یه parser ساده که JSON های تولید‌شده توسط saveProject رو می‌خونه

static void skipWS(const std::string& s, size_t& i) {
    while (i<s.size() && (s[i]==' '||s[i]=='\n'||s[i]=='\r'||s[i]=='\t')) i++;
}

static std::string readStr(const std::string& s, size_t& i) {
    if (i>=s.size()||s[i]!='"') return "";
    i++; std::string r;
    while (i<s.size()&&s[i]!='"') {
        if (s[i]=='\\'&&i+1<s.size()) { i++;
            if (s[i]=='n') r+='\n'; else r+=s[i];
        } else r+=s[i];
        i++;
    }
    if (i<s.size()) i++;
    return r;
}

static float readNum(const std::string& s, size_t& i) {
    skipWS(s,i);
    size_t st=i;
    if (i<s.size()&&(s[i]=='-'||s[i]=='+')) i++;
    while (i<s.size()&&(std::isdigit(s[i])||s[i]=='.')) i++;
    try { return std::stof(s.substr(st,i-st)); } catch(...) { return 0; }
}

static bool readBool(const std::string& s, size_t& i) {
    skipWS(s,i);
    if (s.substr(i,4)=="true") { i+=4; return true; }
    if (s.substr(i,5)=="false"){ i+=5; return false; }
    return false;
}

static void expect(const std::string& s, size_t& i, char c) {
    skipWS(s,i); if (i<s.size()&&s[i]==c) i++;
}

static std::string readKey(const std::string& s, size_t& i) {
    skipWS(s,i);
    auto k = readStr(s,i);
    expect(s,i,':');
    return k;
}

bool loadProject(Project& proj, const std::string& path) {
    std::ifstream f(path);
    if (!f) { logError("Load: cannot open " + path); return false; }
    std::string s((std::istreambuf_iterator<char>(f)),
                   std::istreambuf_iterator<char>());

    proj.sprites.clear();
    proj.blocks.clear();
    proj.variables.clear();
    proj.pen_canvas.strokes.clear();
    proj.pen_canvas.stamps.clear();

    size_t i = 0;
    expect(s,i,'{');

    while (i < s.size()) {
        skipWS(s,i);
        if (s[i]=='}') break;
        if (s[i]==',') { i++; continue; }

        std::string section = readKey(s,i);
        skipWS(s,i);
        expect(s,i,'[');

        // ── Sprites ──
        if (section == "sprites") {
            while (i<s.size()) {
                skipWS(s,i);
                if (s[i]==']') { i++; break; }
                if (s[i]==',') { i++; continue; }
                expect(s,i,'{');
                Sprite sp;
                while (i<s.size()) {
                    skipWS(s,i);
                    if (s[i]=='}') { i++; break; }
                    if (s[i]==',') { i++; continue; }
                    std::string k = readKey(s,i);
                    skipWS(s,i);
                    if      (k=="id")           sp.id           = (int)readNum(s,i);
                    else if (k=="name")         sp.name         = readStr(s,i);
                    else if (k=="x")            sp.x            = readNum(s,i);
                    else if (k=="y")            sp.y            = readNum(s,i);
                    else if (k=="direction")    sp.direction    = readNum(s,i);
                    else if (k=="visible")      sp.visible      = readBool(s,i);
                    else if (k=="size_percent") sp.size_percent = readNum(s,i);
                    else if (k=="costume_path") sp.costume_path = readStr(s,i);
                }
                proj.sprites.push_back(sp);
            }
        }
        // ── Blocks ──
        else if (section == "blocks") {
            while (i<s.size()) {
                skipWS(s,i);
                if (s[i]==']') { i++; break; }
                if (s[i]==',') { i++; continue; }
                expect(s,i,'{');
                Block bl;
                while (i<s.size()) {
                    skipWS(s,i);
                    if (s[i]=='}') { i++; break; }
                    if (s[i]==',') { i++; continue; }
                    std::string k = readKey(s,i);
                    skipWS(s,i);
                    if      (k=="id")          bl.id          = (int)readNum(s,i);
                    else if (k=="type")        bl.type        = readStr(s,i);
                    else if (k=="nextBlockId") bl.nextBlockId = (int)readNum(s,i);
                    else if (k=="x")           bl.x           = readNum(s,i);
                    else if (k=="y")           bl.y           = readNum(s,i);
                    else if (k=="width")       bl.width       = readNum(s,i);
                    else if (k=="height")      bl.height      = readNum(s,i);
                    else if (k=="inputs") {
                        expect(s,i,'[');
                        while (i<s.size()) {
                            skipWS(s,i);
                            if (s[i]==']') { i++; break; }
                            if (s[i]==',') { i++; continue; }
                            bl.inputs.push_back((int)readNum(s,i));
                        }
                    }
                }
                proj.blocks.push_back(bl);
            }
        }
        // ── Variables ──
        else if (section == "variables") {
            while (i<s.size()) {
                skipWS(s,i);
                if (s[i]==']') { i++; break; }
                if (s[i]==',') { i++; continue; }
                expect(s,i,'{');
                Variable vr;
                while (i<s.size()) {
                    skipWS(s,i);
                    if (s[i]=='}') { i++; break; }
                    if (s[i]==',') { i++; continue; }
                    std::string k = readKey(s,i);
                    skipWS(s,i);
                    if      (k=="name")    vr.name    = readStr(s,i);
                    else if (k=="value")   vr.value   = readNum(s,i);
                    else if (k=="visible") vr.visible = readBool(s,i);
                }
                proj.variables.push_back(vr);
            }
        } else {
            // skip unknown section
            int depth=1;
            while(i<s.size()&&depth>0){
                if(s[i]=='['||s[i]=='{') depth++;
                else if(s[i]==']'||s[i]=='}') depth--;
                i++;
            }
        }
    }

    logInfo("Project loaded from: " + path + 
            " (" + std::to_string(proj.sprites.size()) + " sprites, " +
            std::to_string(proj.blocks.size()) + " blocks)");
    return true;
}

// ─── saveProjectAndMark ──────────────────────────────────────────────────────
bool saveProjectAndMark(Project& proj, const std::string& path) {
    bool ok = saveProject(proj, path);
    if (ok) proj.isModified = false;
    return ok;
}

// ─── newProject ──────────────────────────────────────────────────────────────
void newProject(Project& proj) {
    proj.sprites.clear();
    proj.blocks.clear();
    proj.variables.clear();
    proj.pen_canvas.strokes.clear();
    proj.pen_canvas.stamps.clear();
    proj.isModified = false;

    // یک sprite پیش‌فرض
    Sprite sp;
    sp.id = 1; sp.name = "Sprite1";
    sp.x = 0; sp.y = 0; sp.direction = 90;
    sp.visible = true; sp.width = 48; sp.height = 48;
    sp.size_percent = 100;
    sp.costume_path = "";
    proj.sprites.push_back(sp);

    logInfo("New project created.");
}
#include "vars_ops.h"

bool VarStore::defineVar(const std::string& name, const Value& initial) {
    if (name.empty()) return false;
    if (vars.find(name) != vars.end()) return false;

    vars[name] = initial;
    return true;
}

bool VarStore::setVar(const std::string& name, const Value& v) {
    auto it = vars.find(name);
    if (it == vars.end()) return false;
    it->second = v;
    return true;
}

bool VarStore::changeVar(const std::string& name, double delta) {
    auto it = vars.find(name);
    if (it == vars.end()) return false;
    if (it->second.type != Value::Type::Number) return false;
    it->second.num += delta;
    return true;
}

const Value* VarStore::getVar(const std::string& name) const {
    auto it = vars.find(name);
    if (it == vars.end()) return nullptr;
    return &it->second;
}

const std::unordered_map<std::string, Value>& VarStore::all() const {
    return vars;
}

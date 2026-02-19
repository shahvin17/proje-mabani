#ifndef VARS_OPS_H
#define VARS_OPS_H

#include <string>
#include <unordered_map>

struct Value {
    enum Type { NUMBER, STRING };

    Type type = NUMBER;
    double num = 0.0;
    std::string str;
};

inline Value value_number(double x) {
    Value v;
    v.type = Value::NUMBER;
    v.num = x;
    return v;
}

inline Value value_text(const std::string& s) {
    Value v;
    v.type = Value::STRING;
    v.str = s;
    return v;
}

struct VarStore {
    std::unordered_map<std::string, Value> vars;
};

bool var_define(VarStore& vs, const std::string& name, const Value& initial);

bool var_set(VarStore& vs, const std::string& name, const Value& v);

bool var_change(VarStore& vs, const std::string& name, double delta);


const Value* var_get(const VarStore& vs, const std::string& name);

const std::unordered_map<std::string, Value>& var_all(const VarStore& vs);

#endif

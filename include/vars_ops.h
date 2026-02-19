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

    const Value* getVar(const std::string& name) const;

    const std::unordered_map<std::string, Value>& all() const;

private:
    std::unordered_map<std::string, Value> vars;
};

#endif

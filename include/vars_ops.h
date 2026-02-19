#ifndef VARS_OPS_H
#define VARS_OPS_H

#include <string>
#include <unordered_map>


struct Value {
    enum Type { NUMBER, STRING };

    Type type = NUMBER;
    double num = 0.0;
    std::string str;

    static Value number(double x) {
        Value v;
        v.type = Type::Number;
        v.num = x;
        return v;
    }

    static Value text(const std::string& s) {
        Value v;
        v.type = Type::String;
        v.str = s;
        return v;
    }
};


class VarStore {
public:

    bool defineVar(const std::string& name, const Value& initial);


    bool setVar(const std::string& name, const Value& v);


    bool changeVar(const std::string& name, double delta);

    const Value* getVar(const std::string& name) const;

    const std::unordered_map<std::string, Value>& all() const;

private:
    std::unordered_map<std::string, Value> vars;
};

#endif

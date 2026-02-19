#include "vars_ops.h"
#include <cmath>
#include <iostream>
#include <cmath>

using namespace std;
bool var_define(VarStore& vs, const std::string& name, const Value& initial) {
    if (name.empty()) return false;

    if (vs.vars.find(name) != vs.vars.end())
        return false;

    vs.vars[name] = initial;
    return true;
}

bool var_set(VarStore& vs, const std::string& name, const Value& v) {
    auto it = vs.vars.find(name);
    if (it == vs.vars.end()) return false;

    it->second = v;
    return true;
}

bool var_change(VarStore& vs, const std::string& name, double delta) {
    auto it = vs.vars.find(name);
    if (it == vs.vars.end()) return false;
    if (it->second.type != Value::NUMBER) return false;

    it->second.num += delta;
    return true;
}

const Value* var_get(const VarStore& vs, const std::string& name) {
    auto it = vs.vars.find(name);
    if (it == vs.vars.end()) return nullptr;
    return &it->second;
}

const std::unordered_map<std::string, Value>& var_all(const VarStore& vs) {
    return vs.vars;
}

static bool check_is_number(const Value& v, const string& op_name, bool& out_error) {
    if (v.type != Value::NUMBER) {
        cerr << "[Error] Invalid Input in '" << op_name << "': Expected NUMBER." << endl;
        out_error = true;
        return false;
    }
    return true;
}
Value op_add(const Value& a, const Value& b, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "ADD", out_error) || !check_is_number(b, "ADD", out_error)) return value_number(0.0);
    return value_number(a.num + b.num);
}

Value op_sub(const Value& a, const Value& b, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "SUB", out_error) || !check_is_number(b, "SUB", out_error)) return value_number(0.0);
    return value_number(a.num - b.num);
}

Value op_mul(const Value& a, const Value& b, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "MUL", out_error) || !check_is_number(b, "MUL", out_error)) return value_number(0.0);
    return value_number(a.num * b.num);
}

Value op_div(const Value& a, const Value& b, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "DIV", out_error) || !check_is_number(b, "DIV", out_error)) return value_number(0.0);

    if (b.num == 0.0) {
        cerr << "[Math Error] Division by zero!" << endl;
        out_error = true;
        return value_number(0.0);
    }
    return value_number(a.num / b.num);
}

Value op_sqrt(const Value& a, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "SQRT", out_error)) return value_number(0.0);

    if (a.num < 0.0) {
        cerr << "[Math Error] Cannot calculate square root of a negative number: " << a.num << endl;
        out_error = true;
        return value_number(0.0);
    }
    return value_number(std::sqrt(a.num));
}

Value op_string_letter_at(const Value& str, const Value& index, bool& out_error) {

    if (str.type != Value::STRING) {
        cerr << "[String Error] Expected STRING input." << endl;
        out_error = true;
        return value_text("");
    }
    out_error = false;

    if (index.type != Value::NUMBER) {
        cerr << "[String Error] Index must be a number." << endl;
        out_error = true;
        return value_text("");
    }

    int idx = static_cast<int>(index.num) - 1;

    int n = (int)str.str.size();
    if (idx < 0 || idx >= n) {
        cerr << "[String Error] Index out of bounds. Length is " << str.str.length() << " but requested index " << (idx+1) << endl;
         out_error = true;return value_text("");
    }
    return value_text(std::string(1, str.str[idx]));
}

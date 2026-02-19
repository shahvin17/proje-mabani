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
    if (name.empty()) return false;

    auto it = vs.vars.find(name);
    if (it == vs.vars.end()) return false;

    it->second = v;
    return true;
}

bool var_change(VarStore& vs, const std::string& name, double delta) {
    if (name.empty()) return false;

    auto it = vs.vars.find(name);
    if (it == vs.vars.end()) return false;
    if (it->second.type != Value::NUMBER) return false;

    it->second.num += delta;
    return true;
}

const Value* var_get(const VarStore& vs, const std::string& name) {
    if (name.empty()) return nullptr;
    
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

Value op_greater_than(const Value& a, const Value& b, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "GT", out_error) || !check_is_number(b, "GT", out_error))
        return value_number(0.0);
    return value_number(a.num > b.num ? 1.0 : 0.0);
}

Value op_less_than(const Value& a, const Value& b, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "LT", out_error) || !check_is_number(b, "LT", out_error))
        return value_number(0.0);
    return value_number(a.num < b.num ? 1.0 : 0.0);
}


Value op_equals(const Value& a, const Value& b) {
    if (a.type == Value::STRING && b.type == Value::STRING) return value_number((a.str == b.str) ? 1.0 : 0.0);
    if (a.type == Value::NUMBER && b.type == Value::NUMBER) return value_number((a.num == b.num) ? 1.0 : 0.0);
    return value_number(0.0);
}

static bool is_true(const Value& v) {
    if (v.type == Value::STRING) return !v.str.empty();
    return v.num != 0.0;
}

Value op_and(const Value& a, const Value& b) { return value_number((is_true(a) && is_true(b)) ? 1.0 : 0.0); }
Value op_or(const Value& a, const Value& b) { return value_number((is_true(a) || is_true(b)) ? 1.0 : 0.0); }
Value op_not(const Value& a) { return value_number(!is_true(a) ? 1.0 : 0.0); }

Value op_string_length(const Value& str, bool& out_error) {
    out_error = false;
    if (str.type != Value::STRING) {
        cerr << "[String Error] LENGTH expects STRING." << endl;
        out_error = true;
        return value_number(0.0);
    }
    return value_number((double)str.str.size());
}

Value op_string_join(const Value& str1, const Value& str2, bool& out_error) {
    out_error = false;
    if (str1.type != Value::STRING || str2.type != Value::STRING) {
        cerr << "[String Error] JOIN expects STRING + STRING." << endl;
        out_error = true;
        return value_text("");
    }
    return value_text(str1.str + str2.str);
}

Value op_abs(const Value& a) { return (a.type == Value::NUMBER) ? value_number(std::abs(a.num)) : value_number(0.0); }
Value op_floor(const Value& a) { return (a.type == Value::NUMBER) ? value_number(std::floor(a.num)) : value_number(0.0); }
Value op_ceil(const Value& a) { return (a.type == Value::NUMBER) ? value_number(std::ceil(a.num)) : value_number(0.0); }


static double deg_to_rad(double deg) { return (deg * 3.14159265) / 180.0; }

Value op_sin(const Value& a) { return (a.type == Value::NUMBER) ? value_number(std::sin(deg_to_rad(a.num))) : value_number(0.0); }
Value op_cos(const Value& a) { return (a.type == Value::NUMBER) ? value_number(std::cos(deg_to_rad(a.num))) : value_number(0.0); }

Value op_modulo(const Value& a, const Value& b, bool& out_error) {
    out_error = false;
    if (!check_is_number(a, "MOD", out_error) || !check_is_number(b, "MOD", out_error)) return value_number(0.0);
    if (b.num == 0.0) {
        cerr << "[Math Error] Modulo by zero!" << endl;
        out_error = true;
        return value_number(0.0);
    }
    return value_number(std::fmod(a.num, b.num));
}

Value op_xor(const Value& a, const Value& b) {
    bool valA = is_true(a);
    bool valB = is_true(b);
    return value_number((valA != valB) ? 1.0 : 0.0);
}

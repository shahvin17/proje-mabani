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


Value op_add(const Value& a, const Value& b, bool& out_error);
Value op_sub(const Value& a, const Value& b, bool& out_error);
Value op_mul(const Value& a, const Value& b, bool& out_error);
Value op_div(const Value& a, const Value& b, bool& out_error);


Value op_greater_than(const Value& a, const Value& b);
Value op_less_than(const Value& a, const Value& b);
Value op_equals(const Value& a, const Value& b);

Value op_and(const Value& a, const Value& b);
Value op_or(const Value& a, const Value& b);
Value op_not(const Value& a);
Value op_string_length(const Value& str, bool& out_error);
Value op_string_join(const Value& str1, const Value& str2, bool& out_error);
Value op_string_letter_at(const Value& str, const Value& index, bool& out_error);


Value op_abs(const Value& a);
Value op_sqrt(const Value& a, bool& out_error);
Value op_floor(const Value& a);
Value op_ceil(const Value& a);
Value op_sin(const Value& a);
Value op_cos(const Value& a);
Value op_modulo(const Value& a, const Value& b, bool& out_error);
Value op_xor(const Value& a, const Value& b);

#endif

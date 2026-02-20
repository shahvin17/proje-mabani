#ifndef LOOKS_H
#define LOOKS_H
#include <string>

void looks_show();
void looks_hide();

void looks_say(const std::string& text);
const std::string& looks_get_text();
bool looks_has_text();
void looks_clear();

#endif
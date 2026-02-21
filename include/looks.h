#ifndef LOOKS_H
#define LOOKS_H

#include <string>
#include "core_types.h"

// توجه: این فیلدها باید به struct Sprite در core_types.h اضافه شوند:
//   std::string say_message = "";
//   float       say_timer   = 0.0f;
//   float       size_percent = 100.0f;

void looks_say(Sprite& spr, const std::string& msg);
void looks_say_for(Sprite& spr, const std::string& msg, float seconds);
void looks_show(Sprite& spr);
void looks_hide(Sprite& spr);
void looks_set_size(Sprite& spr, float percent);
void looks_change_size(Sprite& spr, float delta);

#endif
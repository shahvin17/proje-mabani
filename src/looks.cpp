#include "looks.h"
#include <algorithm>

void looks_say(Sprite& spr, const std::string& msg) {
    spr.say_message = msg;
    spr.say_timer = -1; // نامحدود
}

void looks_say_for(Sprite& spr, const std::string& msg, float seconds) {
    spr.say_message = msg;
    spr.say_timer = seconds;
}

void looks_show(Sprite& spr) { spr.visible = true; }
void looks_hide(Sprite& spr) { spr.visible = false; }

void looks_set_size(Sprite& spr, float percent) {
    spr.size_percent = std::max(5.0f, std::min(percent, 500.0f));
}

void looks_change_size(Sprite& spr, float delta) {
    looks_set_size(spr, spr.size_percent + delta);
}
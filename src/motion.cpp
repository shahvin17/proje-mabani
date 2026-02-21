#include "motion.h"
#include <cmath>

void motion_move(Sprite& spr, float steps) {
    float rad = (spr.direction - 90.0f) * 3.14159265f / 180.0f;
    spr.x += steps * std::cos(rad);
    spr.y -= steps * std::sin(rad);
}

void motion_turn_right(Sprite& spr, float degrees) {
    spr.direction += degrees;
    while (spr.direction >= 360.0f) spr.direction -= 360.0f;
}

void motion_turn_left(Sprite& spr, float degrees) {
    spr.direction -= degrees;
    while (spr.direction < 0.0f) spr.direction += 360.0f;
}

void motion_goto(Sprite& spr, float x, float y) {
    spr.x = x;
    spr.y = y;
}

void motion_set_x(Sprite& spr, float x) { spr.x = x; }
void motion_set_y(Sprite& spr, float y) { spr.y = y; }
void motion_change_x(Sprite& spr, float dx) { spr.x += dx; }
void motion_change_y(Sprite& spr, float dy) { spr.y += dy; }

void motion_point_direction(Sprite& spr, float dir) { spr.direction = dir; }

void motion_bounce(Sprite& spr, float screen_w, float screen_h) {
    if (spr.x < 0 || spr.x > screen_w) spr.direction = 180.0f - spr.direction;
    if (spr.y < 0 || spr.y > screen_h) spr.direction = -spr.direction;
}
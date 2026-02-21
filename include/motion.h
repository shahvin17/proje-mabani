#ifndef MOTION_H
#define MOTION_H

#include "core_types.h"

void motion_move(Sprite& spr, float steps);
void motion_turn_right(Sprite& spr, float degrees);
void motion_turn_left(Sprite& spr, float degrees);
void motion_goto(Sprite& spr, float x, float y);
void motion_set_x(Sprite& spr, float x);
void motion_set_y(Sprite& spr, float y);
void motion_change_x(Sprite& spr, float dx);
void motion_change_y(Sprite& spr, float dy);
void motion_point_direction(Sprite& spr, float dir);
void motion_bounce(Sprite& spr, float screen_w, float screen_h);

#endif
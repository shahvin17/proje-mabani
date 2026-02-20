#ifndef SENSING_H
#define SENSING_H

#include "core_types.h"
#include <string>
#include <unordered_map>
#include <chrono>

struct SensingManager {

    float mouse_x = 0.0f;
    float mouse_y = 0.0f;
    bool is_mouse_down = false;
    std::unordered_map<int, bool> keys_pressed;
    std::string current_answer = "";
    std::chrono::steady_clock::time_point timer_start;
    SensingManager() {
        timer_start = std::chrono::steady_clock::now();
    }
};
void sense_update_mouse(SensingManager& sm, float x, float y, bool is_down);
void sense_update_key(SensingManager& sm, int key_code, bool is_pressed);
bool sense_is_key_pressed(const SensingManager& sm, int key_code);
bool sense_is_mouse_down(const SensingManager& sm);
float sense_get_mouse_x(const SensingManager& sm);
float sense_get_mouse_y(const SensingManager& sm);
float sense_distance_to_mouse(const Sprite& spr, const SensingManager& sm);
float sense_distance_to_sprite(const Sprite& a, const Sprite& b);
bool sense_touching_mouse(const Sprite& spr, const SensingManager& sm);
bool sense_touching_sprite(const Sprite& a, const Sprite& b);
bool sense_touching_edge(const Sprite& spr, float screen_width, float screen_height);
void sense_set_answer(SensingManager& sm, const std::string& ans);
std::string sense_get_answer(const SensingManager& sm);
void sense_set_drag_mode(Sprite& spr, bool draggable);
bool sense_is_draggable(const Sprite& spr);
void sense_reset_timer(SensingManager& sm);
double sense_get_timer(const SensingManager& sm);

#endif

#include "sensing.h"
#include <cmath>

void sense_update_mouse(SensingManager& sm, float x, float y, bool is_down) {
    sm.mouse_x = x;
    sm.mouse_y = y;
    sm.is_mouse_down = is_down;
}

void sense_update_key(SensingManager& sm, int key_code, bool is_pressed) {
    sm.keys_pressed[key_code] = is_pressed;
}

bool sense_is_key_pressed(const SensingManager& sm, int key_code) {
    auto it = sm.keys_pressed.find(key_code);
    if (it != sm.keys_pressed.end()) {
        return it->second;
    }
    return false;
}

bool sense_is_mouse_down(const SensingManager& sm) { return sm.is_mouse_down; }
float sense_get_mouse_x(const SensingManager& sm) { return sm.mouse_x; }
float sense_get_mouse_y(const SensingManager& sm) { return sm.mouse_y; }

float sense_distance_to_mouse(const Sprite& spr, const SensingManager& sm) {
    float dx = spr.x - sm.mouse_x;
    float dy = spr.y - sm.mouse_y;
    return std::sqrt(dx * dx + dy * dy);
}

float sense_distance_to_sprite(const Sprite& a, const Sprite& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool sense_touching_mouse(const Sprite& spr, const SensingManager& sm) {
    float half_w = spr.width / 2.0f;
    float half_h = spr.height / 2.0f;

    return (sm.mouse_x >= spr.x - half_w && sm.mouse_x <= spr.x + half_w &&
            sm.mouse_y >= spr.y - half_h && sm.mouse_y <= spr.y + half_h);
}

bool sense_touching_sprite(const Sprite& a, const Sprite& b) {
    float a_left = a.x - (a.width / 2.0f);
    float a_right = a.x + (a.width / 2.0f);
    float a_top = a.y - (a.height / 2.0f);
    float a_bottom = a.y + (a.height / 2.0f);

    float b_left = b.x - (b.width / 2.0f);
    float b_right = b.x + (b.width / 2.0f);
    float b_top = b.y - (b.height / 2.0f);
    float b_bottom = b.y + (b.height / 2.0f);

    return !(a_right < b_left || a_left > b_right || a_bottom < b_top || a_top > b_bottom);
}

bool sense_touching_edge(const Sprite& spr, float screen_width, float screen_height) {
    float half_w = spr.width / 2.0f;
    float half_h = spr.height / 2.0f;

    return (spr.x - half_w <= 0 || spr.x + half_w >= screen_width ||
            spr.y - half_h <= 0 || spr.y + half_h >= screen_height);
}

void sense_set_answer(SensingManager& sm, const std::string& ans) {
    sm.current_answer = ans;
}

std::string sense_get_answer(const SensingManager& sm) {
    return sm.current_answer;
}

void sense_set_drag_mode(Sprite& spr, bool draggable) {
    spr.draggable = draggable;
}

bool sense_is_draggable(const Sprite& spr) {
    return spr.draggable;
}

void sense_reset_timer(SensingManager& sm) {
    sm.timer_start = std::chrono::steady_clock::now();
}

double sense_get_timer(const SensingManager& sm) {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - sm.timer_start;
    return elapsed.count();
}

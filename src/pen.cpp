#include "pen.h"
#include <algorithm>
#include <cmath>

// ─── تبدیل HSB → RGB ─────────────────────────────────────────────────────────
void hsb_to_rgb(float h, float s, float b, int& r, int& g, int& bo) {
    // h: 0-360, s: 0-100, b: 0-100
    s /= 100.0f; b /= 100.0f;
    if (s <= 0.0f) { r=g=bo=(int)(b*255); return; }
    float hh = fmod(h, 360.0f) / 60.0f;
    int   i  = (int)hh;
    float ff = hh - i;
    float p  = b*(1-s), q=b*(1-s*ff), t=b*(1-s*(1-ff));
    float fr,fg,fb;
    switch(i){
        case 0: fr=b;g=(int)(t*255);fb=(int)(p*255); r=(int)(b*255);g=(int)(t*255);bo=(int)(p*255);return;
        case 1: r=(int)(q*255);g=(int)(b*255);bo=(int)(p*255);return;
        case 2: r=(int)(p*255);g=(int)(b*255);bo=(int)(t*255);return;
        case 3: r=(int)(p*255);g=(int)(q*255);bo=(int)(b*255);return;
        case 4: r=(int)(t*255);g=(int)(p*255);bo=(int)(b*255);return;
        default:r=(int)(b*255);g=(int)(p*255);bo=(int)(q*255);return;
    }
    (void)fr;(void)fg;(void)fb;
}

// ─── هم‌گام‌سازی RGB از HSB ──────────────────────────────────────────────────
static void sync_rgb(PenState& pen) {
    hsb_to_rgb(pen.hue, pen.saturation, pen.brightness, pen.r, pen.g, pen.b);
}

// ─── down/up ──────────────────────────────────────────────────────────────────
void pen_down(PenState& pen, float x, float y) {
    pen.is_down  = true;
    pen.prev_x   = x;
    pen.prev_y   = y;
    pen.has_prev = true;
}

void pen_up(PenState& pen) {
    pen.is_down  = false;
    pen.has_prev = false;
}

bool pen_is_down(const PenState& pen) { return pen.is_down; }

// ─── رسم خط هنگام حرکت ───────────────────────────────────────────────────────
void pen_move_to(PenState& pen, PenCanvas& canvas, float nx, float ny) {
    if (pen.is_down && pen.has_prev) {
        PenStroke s;
        s.x1=pen.prev_x; s.y1=pen.prev_y;
        s.x2=nx;         s.y2=ny;
        s.r=pen.r; s.g=pen.g; s.b=pen.b;
        s.size=pen.size;
        canvas.strokes.push_back(s);
    }
    pen.prev_x   = nx;
    pen.prev_y   = ny;
    pen.has_prev = true;
}

// ─── رنگ RGB مستقیم ──────────────────────────────────────────────────────────
void pen_set_color_rgb(PenState& pen, int r, int g, int b) {
    pen.r=std::clamp(r,0,255);
    pen.g=std::clamp(g,0,255);
    pen.b=std::clamp(b,0,255);
}

// ─── hue ─────────────────────────────────────────────────────────────────────
void pen_set_hue(PenState& pen, float hue) {
    pen.hue = fmod(hue + 360.0f, 360.0f);
    sync_rgb(pen);
}
void pen_change_hue(PenState& pen, float delta) {
    pen_set_hue(pen, pen.hue + delta * 2.0f); // Scratch: hue 0-200 → درجه 0-360
}

// ─── saturation ──────────────────────────────────────────────────────────────
void pen_set_saturation(PenState& pen, float sat) {
    pen.saturation = std::clamp(sat, 0.0f, 100.0f);
    sync_rgb(pen);
}
void pen_change_saturation(PenState& pen, float delta) {
    pen_set_saturation(pen, pen.saturation + delta);
}

// ─── brightness ──────────────────────────────────────────────────────────────
void pen_set_brightness(PenState& pen, float br) {
    pen.brightness = std::clamp(br, 0.0f, 100.0f);
    sync_rgb(pen);
}
void pen_change_brightness(PenState& pen, float delta) {
    pen_set_brightness(pen, pen.brightness + delta);
}

void pen_get_color(const PenState& pen, int& r, int& g, int& b) {
    r=pen.r; g=pen.g; b=pen.b;
}

// ─── اندازه قلم ──────────────────────────────────────────────────────────────
void pen_set_size(PenState& pen, float size) {
    pen.size = std::max(1.0f, size);
}
void pen_change_size(PenState& pen, float delta) {
    pen_set_size(pen, pen.size + delta);
}
float pen_get_size(const PenState& pen) { return pen.size; }

// ─── stamp ───────────────────────────────────────────────────────────────────
void pen_stamp(PenCanvas& canvas, float x, float y,
               float w, float h, float size_pct, const std::string& costume) {
    StampImage si;
    si.world_x      = x;
    si.world_y      = y;
    si.width        = w;
    si.height       = h;
    si.size_percent = size_pct;
    si.costume_path = costume;
    canvas.stamps.push_back(si);
}

// ─── erase all ───────────────────────────────────────────────────────────────
void pen_erase_all(PenCanvas& canvas) {
    canvas.strokes.clear();
    canvas.stamps.clear();
}

// ─── backward compat ─────────────────────────────────────────────────────────
void setPenColor(PenState& pen,int r,int g,int b) { pen_set_color_rgb(pen,r,g,b); }
void changePenColor(PenState& pen,int amount) {
    pen_change_hue(pen, (float)amount);
}
void getPenColor(const PenState& pen,int& r,int& g,int& b) { r=pen.r;g=pen.g;b=pen.b; }
void setPenSize(PenState& pen,float s)  { pen_set_size(pen,s); }
void changePenSize(PenState& pen,float d){ pen_change_size(pen,d); }
float getPenSize(const PenState& pen)   { return pen.size; }
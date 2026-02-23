#ifndef PEN_H
#define PEN_H

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

// ─── یک خط رسم‌شده ───────────────────────────────────────────────────────────
struct PenStroke {
    float x1, y1, x2, y2;   // مختصات Scratch
    int   r, g, b;
    float size;
};

// ─── stamp: snapshot از sprite ───────────────────────────────────────────────
struct StampImage {
    float world_x, world_y;
    float width, height;
    float size_percent;
    std::string costume_path;
    int r=255, g=165, b=0;  // fallback رنگ
};

// ─── وضعیت قلم ───────────────────────────────────────────────────────────────
struct PenState {
    bool  is_down    = false;
    int   r=0, g=0, b=255;
    float size       = 1.0f;
    float brightness = 100.0f;
    float saturation = 100.0f;
    float hue        = 200.0f;  // آبی پیش‌فرض
    float prev_x     = 0.0f;
    float prev_y     = 0.0f;
    bool  has_prev   = false;
};

// ─── Canvas سراسری ───────────────────────────────────────────────────────────
struct PenCanvas {
    std::vector<PenStroke>  strokes;
    std::vector<StampImage> stamps;
};

// ─── API ─────────────────────────────────────────────────────────────────────
void pen_down(PenState& pen, float x, float y);
void pen_up(PenState& pen);
bool pen_is_down(const PenState& pen);
void pen_move_to(PenState& pen, PenCanvas& canvas, float nx, float ny);

void pen_set_color_rgb(PenState& pen, int r, int g, int b);
void pen_set_hue(PenState& pen, float hue);
void pen_change_hue(PenState& pen, float delta);
void pen_set_saturation(PenState& pen, float sat);
void pen_change_saturation(PenState& pen, float delta);
void pen_set_brightness(PenState& pen, float br);
void pen_change_brightness(PenState& pen, float delta);
void pen_get_color(const PenState& pen, int& r, int& g, int& b);

void pen_set_size(PenState& pen, float size);
void pen_change_size(PenState& pen, float delta);
float pen_get_size(const PenState& pen);

void pen_stamp(PenCanvas& canvas, float x, float y,
               float w, float h, float size_pct, const std::string& costume);
void pen_erase_all(PenCanvas& canvas);

void hsb_to_rgb(float h, float s, float b, int& r, int& g, int& bo);

// ── backward compat ──
inline void penDown(PenState& p)            { p.is_down=true; }
inline void penUp(PenState& p)              { p.is_down=false; }
inline bool penIsDown(const PenState& p)    { return p.is_down; }
void setPenColor(PenState& pen,int r,int g,int b);
void changePenColor(PenState& pen,int amount);
void getPenColor(const PenState& pen,int& r,int& g,int& b);
void setPenSize(PenState& pen,float s);
void changePenSize(PenState& pen,float d);
float getPenSize(const PenState& pen);

#endif
#ifndef PEN_H
#define PEN_H

#include <algorithm>
struct PenState {
    bool is_down = false;
    int r = 255;
    int g = 0;
    int b = 0;
    float size = 1.0f;
};

void penDown(PenState& pen);
void penUp(PenState& pen);
bool penIsDown(const PenState& pen);

void setPenColor(PenState& pen, int r, int g, int b);

void changePenColor(PenState& pen, int amount);

void getPenColor(const PenState& pen, int& r, int& g, int& b);

void setPenSize(PenState& pen, float newSize);

void changePenSize(PenState& pen, float deltaSize);

float getPenSize(const PenState& pen);

#endif

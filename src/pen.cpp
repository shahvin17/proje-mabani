#include "pen.h"
#include <iostream>

void penDown(PenState& pen) {
    pen.is_down = true;
}

void penUp(PenState& pen) {
    pen.is_down = false;
}

bool penIsDown(const PenState& pen) {
    return pen.is_down;
}

void setPenColor(PenState& pen, int r, int g, int b) {

    pen.r = std::max(0, std::min(r, 255));
    pen.g = std::max(0, std::min(g, 255));
    pen.b = std::max(0, std::min(b, 255));
}

void changePenColor(PenState& pen, int amount) {

    pen.r = std::max(0, std::min(pen.r + amount, 255));
    pen.g = std::max(0, std::min(pen.g + amount, 255));
    pen.b = std::max(0, std::min(pen.b + amount, 255));
}

void getPenColor(const PenState& pen, int& r, int& g, int& b) {
    r = pen.r;
    g = pen.g;
    b = pen.b;
}

void setPenSize(PenState& pen, float newSize) {
   
    if (newSize < 1.0f) {
        pen.size = 1.0f;
    } else {
        pen.size = newSize;
    }
}

void changePenSize(PenState& pen, float deltaSize) {
    pen.size += deltaSize;

    if (pen.size < 1.0f) {
        pen.size = 1.0f;
    }
}

float getPenSize(const PenState& pen) {
    return pen.size;
}

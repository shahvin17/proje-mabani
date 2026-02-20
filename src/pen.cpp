#include "pen.h"

static bool pen_down = false;
static int pen_r = 255;
static int pen_g = 0;
static int pen_b = 0;

void penDown() {
    pen_down = true;
}

void penUp() {
    pen_down = false;
}

void setPenColor(int r, int g, int b) {
    pen_r = r;
    pen_g = g;
    pen_b = b;
}

bool penIsDown() {
    return pen_down;
}

void getPenColor(int& r, int& g, int& b) {
    r = pen_r;
    g = pen_g;
    b = pen_b;
}
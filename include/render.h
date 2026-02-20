#ifndef RENDER_H
#define RENDER_H
#include <SDL.h>

struct DragRect {
    int x, y;
    int w, h;
    bool dragging;
};

void renderRect(SDL_Renderer* renderer, const DragRect& r);
bool isInside(const DragRect& r, int mx, int my);
#endif
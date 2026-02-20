#ifndef RENDER_H
#define RENDER_H

#include <string>
#include <SDL2/SDL.h>


struct DragRect {
    int x, y;
    int w, h;
    bool dragging;
};

void renderRect(SDL_Renderer* renderer, const DragRect& r);


bool isInside(const DragRect& r, int mx, int my);

// ===============================
// Looks - Say block render
// ===============================
void renderSayText(const std::string& text, int x, int y);

#endif
#include "render.h"

void renderRect(SDL_Renderer* renderer, const DragRect& r) {
    SDL_Rect rect{ r.x, r.y, r.w, r.h };
    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
    SDL_RenderFillRect(renderer, &rect);
}

bool isInside(const DragRect& r, int mx, int my) {
    return (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h);
}
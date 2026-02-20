#include "render.h"

// --- تغییر اینجاست ---

void renderRect(SDL_Renderer* renderer, const SDL_Rect& r, const SDL_Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &r);
}

bool isInside(const SDL_Rect& r, int mx, int my) {
    return (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h);
}
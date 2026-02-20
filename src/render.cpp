#include "render.h"

void renderTestRect(SDL_Renderer* renderer) {
    SDL_Rect rect{100, 100, 120, 80};
    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
    SDL_RenderFillRect(renderer, &rect);
}
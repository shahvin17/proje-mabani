#include "render.h"
#include <iostream>
#include "pen.h"

// ===============================
// Primitive render helpers
// ===============================
void renderRect(SDL_Renderer* renderer, const DragRect& r) {
    if (!renderer) return;

    SDL_Rect rect{ r.x, r.y, r.w, r.h };
    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
    SDL_RenderFillRect(renderer, &rect);
}

bool isInside(const DragRect& r, int mx, int my) {
    return (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h);
}

// ===============================
// Looks: say block render
// ===============================
void renderSayText(const std::string& text, int x, int y) {
    std::cout << "[render] SAY at (" << x << ", " << y << "): "
              << text << std::endl;

}


void renderPenTest(SDL_Renderer* renderer) {
    if (!penIsDown()) return;

    int r, g, b;
    getPenColor(r, g, b);

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawLine(renderer, 100, 100, 300, 300);
}
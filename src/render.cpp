#include "render.h"
#include <iostream>

static TTF_Font* g_font = nullptr;

bool text_init(const char* font_path, int font_size) {
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf init failed: " << TTF_GetError() << std::endl;
        return false;
    }
    g_font = TTF_OpenFont(font_path, font_size);
    if (!g_font) {
        std::cerr << "Font load failed: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

void render_text(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    if (!g_font || text.empty()) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(g_font, text.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dest = { x, y, surf->w, surf->h };
    SDL_RenderCopy(renderer, tex, nullptr, &dest);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void text_quit() {
    if (g_font) { TTF_CloseFont(g_font); g_font = nullptr; }
    TTF_Quit();
}

void renderRect(SDL_Renderer* renderer, const SDL_Rect& r, const SDL_Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &r);
}

bool isInside(const SDL_Rect& r, int mx, int my) {
    return (mx >= r.x && mx <= r.x + r.w &&
            my >= r.y && my <= r.y + r.h);
}
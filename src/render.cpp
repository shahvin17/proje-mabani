#include "render.h"
#include <iostream>

// --- پیاده‌سازی توابع جدید متن ---
static TTF_Font* g_font = nullptr;

bool text_init(const char* font_path, int font_size) {
    if (TTF_Init() == -1) {
        std::cerr << "Failed to initialize SDL_ttf: " << TTF_GetError() << std::endl;
        return false;
    }
    g_font = TTF_OpenFont(font_path, font_size);
    if (!g_font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

void render_text(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color) {
    if (!g_font || text.empty()) return;

    SDL_Surface* text_surface = TTF_RenderText_Blended(g_font, text.c_str(), color);
    if (!text_surface) {
        std::cerr << "Failed to render text: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_Rect dest_rect = {x, y, text_surface->w, text_surface->h};
    
    SDL_RenderCopy(renderer, text_texture, NULL, &dest_rect);

    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);
}

void text_quit() {
    if (g_font) {
        TTF_CloseFont(g_font);
    }
    TTF_Quit();
}

// توابع قبلی (بدون تغییر)
void renderRect(SDL_Renderer* renderer, const SDL_Rect& r, const SDL_Color& color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &r);
}

bool isInside(const SDL_Rect& r, int mx, int my) {
    return (mx >= r.x && mx <= r.x + r.w && my >= r.y && my <= r.y + r.h);
}

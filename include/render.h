#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h> // --- اضافه شد
#include <string>           // --- اضافه شد

// --- توابع جدید برای مدیریت متن ---
bool text_init(const char* font_path, int font_size);
void render_text(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color);
void text_quit();

// توابع قبلی (بدون تغییر)
void renderRect(SDL_Renderer* renderer, const SDL_Rect& r, const SDL_Color& color);
bool isInside(const SDL_Rect& r, int mx, int my);

#endif

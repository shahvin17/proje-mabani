#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>

// ساختار DragRect حذف شد

// تابع برای رندر یک مستطیل SDL
void renderRect(SDL_Renderer* renderer, const SDL_Rect& r, const SDL_Color& color);

// تابع برای بررسی برخورد نقطه با مستطیل SDL
bool isInside(const SDL_Rect& r, int mx, int my);

#endif

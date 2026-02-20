#include "render.h"

longint const titleColor= 0xFF40C040, windowColor= 0xFF103010;
unsigned int screenWidth, screenHeight;
SDL_Renderer * m_renderer;
SDL_Window * m_window;
SDL_Texture * m_texture;
SDL_Surface * m_surface;
SDL_Event m_event;

void initGraphics(){
    Uint32 SDL_flags = SDL_INIT_VIDEO | SDL_INIT_TIMER ;
    Uint32 WND_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_INPUT_GRABBED | SDL_WINDOW_INPUT_FOCUS;
    SDL_Window * m_window;
    SDL_Init( SDL_flags );
    SDL_CreateWindowAndRenderer( 1920, 480, WND_flags, &m_window, &m_renderer );
    SDL_RaiseWindow(m_window);
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    screenWidth = DM.w;
    screenHeight = DM.h;
}

void closeGraphics(){
    SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    IMG_Quit();
    SDL_Quit();
}

void clearScreen(){
    SDL_SetRenderDrawColor( m_renderer, 0, 0, 0, 255 );
    SDL_RenderClear( m_renderer );
    SDL_RenderPresent( m_renderer );
}

void drawWindow(const string Title, const unsigned int x1, y1, x2, y2){
    Sint16 X[4]={x1,x2,x2,x1};
    Sint16 Y[4]={y1,y1,y2,y2};
    filledPolygonColor(m_renderer,X,Y,4,windowColor);
    Sint16 X2[4]={x1,x2,x2,x1};
    Sint16 Y2[4]={y1,y1,y1+20,y1+20};
    filledPolygonColor(m_renderer,X2,Y2,4,titleColor);
}

void drag(){
}

void renderRect(SDL_Renderer* renderer, const DragRect& r) {
    SDL_Rect rect{r.x, r.y, r.w, r.h};
    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
    SDL_RenderFillRect(renderer, &rect);
}

bool isInside(const DragRect& r, int mx, int my) {
    return mx >= r.x && mx <= r.x + r.w &&
           my >= r.y && my <= r.y + r.h;
}
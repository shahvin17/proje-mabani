#include <SDL2/SDL.h>
#include <iostream>

int main(int argc, char* argv[])
{
    // 1. Init SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. Create Window
    SDL_Window* window = SDL_CreateWindow(
        "Drag & Drop - Day 1",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 3. Create Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer)
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // 4. Test block (for drag & drop)
    SDL_Rect block;
    block.x = 300;
    block.y = 200;
    block.w = 100;
    block.h = 100;

    bool running = true;
    bool dragging = false;

    int mouseOffsetX = 0;
    int mouseOffsetY = 0;

    SDL_Event event;

    // 5. Main loop
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            // Mouse button down
            if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                int mx = event.button.x;
                int my = event.button.y;

                if (mx >= block.x && mx <= block.x + block.w &&
                    my >= block.y && my <= block.y + block.h)
                {
                    dragging = true;
                    mouseOffsetX = mx - block.x;
                    mouseOffsetY = my - block.y;
                }
            }

            // Mouse button up
            if (event.type == SDL_MOUSEBUTTONUP)
            {
                dragging = false;
            }

            // Mouse move
            if (event.type == SDL_MOUSEMOTION && dragging)
            {
                block.x = event.motion.x - mouseOffsetX;
                block.y = event.motion.y - mouseOffsetY;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        // Draw block
        SDL_SetRenderDrawColor(renderer, 0, 180, 255, 255);
        SDL_RenderFillRect(renderer, &block);

        // Present
        SDL_RenderPresent(renderer);
    }

    // 6. Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
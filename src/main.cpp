#include <SDL2/SDL.h>
#include <iostream>
#include "render.h"
#include "pen.h"

int main(int argc, char* argv[]) {
    // =========================
    // SDL INIT
    // =========================
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Scratch - Looks Test",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window creation failed\n";
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer) {
        std::cerr << "Renderer creation failed\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    // =========================
    // MAIN LOOP
    // =========================

    penDown();
    setPenColor(0, 255, 0);
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }


        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);


        renderSayText("Hello Scratch!", 100, 100);

        renderPenTest(renderer);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    // =========================
    // CLEANUP
    // =========================
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
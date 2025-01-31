#include <SDL2/SDL.h>
#include <vector>
#include <iostream>

int minX = 0;
int minY = 0;

int maxX = 0;
int maxY = 0;

void handleWindowResize(int width, int height, SDL_Renderer *renderer) {
    maxX = width;
    maxY = height;
    // Clear to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void drawArray(std::vector<int> *list, SDL_Renderer *renderer) {
    int rectXSize = maxX / list->size();
    //int rectYSize = maxY
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int n = 0; n * rectXSize < maxX; n++) {
        SDL_Rect rect = {n * rectXSize, 0, rectXSize, 100};
        SDL_RenderDrawRect(renderer, &rect);
    }


    SDL_RenderPresent(renderer);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr){
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        return 1;
    }

    SDL_GetRendererOutputSize(renderer, &minX, &minY);

    maxX = minX - 1;
    maxY = minY - 1;

    std::vector<int> list = {1,2,3,4,5};
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
            case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                        int width = event.window.data1;
                        int height = event.window.data2;
                        handleWindowResize(width, height, renderer);
                    }
                    break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_q) {
                    quit = true;
                }
                break;
            }

        }

        drawArray(&list, renderer);

        SDL_Delay(200);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

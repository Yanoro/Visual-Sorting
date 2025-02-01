#pragma once

#include "simulation.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <thread>

#define DELAY 1

class SimulationDraw : public Simulation {
private:
  SDL_Renderer *renderer;
  SDL_Window *window;

  int windowHeight = 1920;
  int windowWidth = 1080;

  const int SIMULATION_FAILURE = -1;
  const int SIMULATION_SUCCESS= 0;

public:
    SimulationDraw() : Simulation() {
        // TODO: Handle failure in a better way
        this->initializeSDL();
    }

    void handleWindowResize(int width, int height, SDL_Renderer *renderer) {
        windowHeight = width;
        windowWidth = height;
        // Clear to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    int initializeSDL() {
        std::cout << "Initializing SDL..." << std::endl;
        SDL_Init(SDL_INIT_VIDEO);

        this->window = SDL_CreateWindow("Sorting Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        windowHeight, windowWidth, SDL_WINDOW_SHOWN);
        if (window == nullptr) {
            SDL_Log("Failed to create window: %s", SDL_GetError());
            return SIMULATION_FAILURE;
        }

        this->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr){
            SDL_Log("Failed to create renderer: %s", SDL_GetError());
            return SIMULATION_FAILURE;
        }

        SDL_SetWindowResizable(window, SDL_FALSE);

        std::cout << "Initialization was a success!" << std::endl;
        return SIMULATION_SUCCESS;
    }

    void drawArray() {
        std::vector<int> *list = this->getList();
        size_t lSize = list->size();
        int rectXSize = windowHeight / lSize;
        int ceiling = windowWidth - 200;
        auto max = std::max_element(list->begin(), list->end());
        int rectYSize = ceiling / *max;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int n = 0; n < lSize; n++) {
            int currYCoords = windowWidth - (rectYSize * list->at(n));
            SDL_Rect rect = {n * rectXSize, currYCoords, rectXSize, rectYSize * list->at(n)};
            SDL_RenderFillRect(renderer, &rect);
        }

        // After drawing all the normal rectangles we draw over them all the rectangles that
        // need to be highlighted according to the simulator
        //
        // We do it here instead of inside the previous for loop to avoid having to check for every rectangle
        // whether it should be highlighted or not
        for (const auto element : *getRedHighlight()) {
            std::cout << "ELEM: " << element << std::endl;
            int currYCoords = windowWidth - (rectYSize * list->at(element));
            SDL_Rect rect = {element * rectXSize, currYCoords, rectXSize, rectYSize * list->at(element)};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
            SDL_RenderFillRect(renderer, &rect);
        }

    }

    bool handleInput() {
        SDL_Event event;
        bool quit = false;
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
                    std::cout << windowHeight << ", " << windowWidth << std::endl;
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_q) {
                    quit = true;
                }
                else if (event.key.keysym.sym == SDLK_r) {
                    resetList();
                }
                break;
            }
        }
        return quit;
    }

    void loop() {
        bool quit = false;
        SDL_Event event;
        std::thread t(&Simulation::initSorting, this);

        while (!quit) {
            quit = handleInput();
            SDL_RenderClear(renderer);
            drawArray();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

            SDL_RenderPresent(renderer);

            SDL_Delay(DELAY);
        }

        SDL_DestroyRenderer(this->renderer);
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }

};

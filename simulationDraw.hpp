#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <thread>
#include <string>

#include "simulation.hpp"

class SimulationDraw : public Simulation {
private:
    std::string sortName;

    SDL_Renderer *renderer;
    SDL_Window *window;

    int windowHeight;
    int windowWidth;

    size_t listSize;

    std::thread simThread;

    const int SIMULATION_SUCCESS= 0;

public:
    SimulationDraw(size_t listSize) : Simulation(listSize) {
        this->listSize = listSize;

        initializeSDL();
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

        SDL_DisplayMode displayMode;
        if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
            SDL_Log("Failed to get display mode: %s", SDL_GetError());
            throw std::runtime_error("Failed to get display mode");
        }

        windowHeight = displayMode.h;
        windowWidth = displayMode.w;

        this->window = SDL_CreateWindow("Sorting Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        windowHeight, windowWidth, SDL_WINDOW_FULLSCREEN);
        if (window == nullptr) {
            SDL_Log("Failed to create window: %s", SDL_GetError());
            throw std::runtime_error("Failed to create window");
        }



        this->renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr){
            SDL_Log("Failed to create renderer: %s", SDL_GetError());
            throw std::runtime_error("Failed to create renderer");
        }

        SDL_SetWindowResizable(window, SDL_FALSE);

        std::cout << "Initialization was a success!" << std::endl;
        return SIMULATION_SUCCESS;
    }

    void drawArray() {
        std::vector<int> *list = this->getList();
        int lSize = list->size();
        int rectXSize = windowHeight / lSize;
        int ceiling = windowWidth * 0.9 ;
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
            int currYCoords = windowWidth - (rectYSize * list->at(element));
            SDL_Rect rect = {element * rectXSize, currYCoords, rectXSize, rectYSize * list->at(element)};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
            SDL_RenderFillRect(renderer, &rect);
        }

    }

    void printVector(std::vector<int> *vec) {
        for (const auto element : *vec) {
            std::cout << element << " ";
        }

        std::cout << std::endl;
    }

    void restartSimulation() {
        stopSim(); // Signals the sim thread to stop
        simThread.join();
        resetList(listSize);
        startSim();
        simThread = std::thread(&Simulation::initSorting, this);
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
                    restartSimulation();
                }
                else if (event.key.keysym.sym == SDLK_LEFT) {
                    decSteps();
                }
                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    incSteps();
                }

                break;
            }
        }
        return quit;
    }

    void loop() {
        bool quit = false;
        SDL_Event event;

        simThread = std::thread(&Simulation::initSorting, this);

        while (!quit) {
            quit = handleInput();

            drawArray();
            printSteps();
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderPresent(renderer);
            SDL_Delay(DELAY);
            SDL_RenderClear(renderer);
        }

        SDL_DestroyRenderer(this->renderer);
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }

};

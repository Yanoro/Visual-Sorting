#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_ttf.h>
#include <algorithm>
#include <thread>
#include <string>

#include "simulation.hpp"

class SimulationDraw : public Simulation {
private:
    std::string sortName;

    SDL_Renderer *renderer;
    SDL_Window *window;

    TTF_Font *font;

    int windowHeight;
    int windowWidth;

    size_t listSize;

    std::thread simThread;

public:
    SimulationDraw(size_t listSize) : Simulation(listSize) {
        this->listSize = listSize;
        initializeSDL();
        initializeTTF();
    }

    void handleWindowResize(int width, int height, SDL_Renderer *renderer) {
        windowHeight = width;
        windowWidth = height;
        // Clear to black
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    void initializeTTF() {
        TTF_Init();
        font = TTF_OpenFont("Roboto.ttf", 24);
        if (font == nullptr) {
            std::cerr << "Error: " << TTF_GetError() << std::endl;
            throw std::runtime_error("Couldn't find font!");
        }
    }

    void initializeSDL() {
        std::cout << "Initializing SDL..." << std::endl;
        SDL_Init(SDL_INIT_VIDEO);

        SDL_DisplayMode displayMode;
        if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
            SDL_Log("Failed to get display mode: %s", SDL_GetError());
            throw std::runtime_error("Failed to get display mode");
        }

        windowHeight = displayMode.h;
        windowWidth = displayMode.w;

        window = SDL_CreateWindow("Sorting Visualizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        windowHeight, windowWidth, SDL_WINDOW_FULLSCREEN);
        if (window == nullptr) {
            SDL_Log("Failed to create window: %s", SDL_GetError());
            throw std::runtime_error("Failed to create window");
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == nullptr){
            SDL_Log("Failed to create renderer: %s", SDL_GetError());
            throw std::runtime_error("Failed to create renderer");
        }

        SDL_SetWindowResizable(window, SDL_FALSE);

    }

    void drawArray() {
        std::vector<int> *list = getList();
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
        for (const auto element : *getHighlights()) {
            int currYCoords = windowWidth - (rectYSize * list->at(element));
            SDL_Rect rect = {element * rectXSize, currYCoords, rectXSize, rectYSize * list->at(element)};
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    void drawText() {
        SDL_Color textColorWhite = {255, 255, 255, 0};

        const char *algoStr = "Algorithm: BubbleSort";
        std::string swapString = "Swaps: " + std::to_string(getSwapCount());
        std::string compString = "Comparisons: " + std::to_string(getCompCount());
        const char *swapStr = swapString.c_str();
        const char *compStr = compString.c_str();

        SDL_Surface* AlgoSurface = TTF_RenderText_Solid(font, algoStr,textColorWhite);
        SDL_Surface* swapSurface = TTF_RenderText_Solid(font, swapStr, textColorWhite);
        SDL_Surface* compSurface = TTF_RenderText_Solid(font, compStr, textColorWhite);

        SDL_Texture* algoTexture = SDL_CreateTextureFromSurface(renderer, AlgoSurface);
        SDL_Texture* swapTexture = SDL_CreateTextureFromSurface(renderer, swapSurface);
        SDL_Texture* compTexture = SDL_CreateTextureFromSurface(renderer, compSurface);

        SDL_Rect *algoRect = new SDL_Rect {0, 0, 200, 50};
        SDL_Rect *swapRect = new SDL_Rect {300, 0, 200, 50};
        SDL_Rect *compRect = new SDL_Rect {800, 0, 200, 50};

        SDL_RenderCopy(renderer, algoTexture, NULL, algoRect);
        SDL_RenderCopy(renderer, swapTexture, NULL, swapRect);
        SDL_RenderCopy(renderer, compTexture, NULL, compRect);

        SDL_FreeSurface(AlgoSurface);
        SDL_DestroyTexture(algoTexture);

        SDL_FreeSurface(swapSurface);
        SDL_DestroyTexture(swapTexture);

        SDL_FreeSurface(compSurface);
        SDL_DestroyTexture(compTexture);
    }

    void printVector(std::vector<int> *vec) {
        for (const auto element : *vec) {
            std::cout << element << " ";
        }

        std::cout << std::endl;
    }

    void restartSimulation() {
        stopSim(); // Signals the sim thread to stop
        restartCount();
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

        simThread = std::thread(&Simulation::initSorting, this);

        while (!quit) {
            quit = handleInput();

            drawArray();
            drawText();
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderPresent(renderer);
            SDL_Delay(DELAY);
            SDL_RenderClear(renderer);
        }

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

};

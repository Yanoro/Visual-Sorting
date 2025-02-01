#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <array>
#include <condition_variable>
#include <thread>
#include <mutex>

#define DEFAULT_LIST_SIZE 480
#define DEFAULT_MIN_ELEMENT_VAL 1 // Smallest value that can be in the list
#define DEFAULT_MAX_ELEMENT_VAL 100 // Biggest value that can be in the list

#define DELAY 1

int windowHeight = 1920;
int windowWidth = 1080;

// TODO: See if we can make this not global
std::mutex sortingMutex;
std::condition_variable sortingCV;          // Condition variable for signaling
bool drawTurn = true;

class Simulation {
    private:
    std::vector<int> *list;
    bool isDrawing = false;
    //int greenHighlight[2];
    std::array<int, 2> redHighlight;

    public:
    Simulation () {
        resetList();
        redHighlight[0] = 0;
        redHighlight[1] = 1;
    }

    void resetList() {
        this->list = genRandomList(DEFAULT_LIST_SIZE,
                                   DEFAULT_MIN_ELEMENT_VAL,
                                   DEFAULT_MAX_ELEMENT_VAL);
    }

    std::vector<int> *genRandomList(size_t listSize, int minVal, int maxVal) {
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(minVal, maxVal);

        std::vector<int> *vec = new std::vector<int>(listSize);
        for (int i = i; i < listSize; i++) {
            (*vec)[i] = distr(eng);
        }
        return vec;
    }

    bool isListSorted(std::vector<int> *vec) {
        for (int i = 0; i < vec->size() - 2; i++) {
            if ((*vec)[i] > (*vec)[i + 1]) {
                std::cout << "Not sorted " << i << " " << i + 1 << std::endl;
                std::cout << (*vec)[i] << " " << (*vec)[i + 1] << std::endl;
                return false;
            }
        }
        std::cout << "sorted" << std::endl;
        return true;
    }

    void printVector(std::vector<int> *vec) {
        for (const auto element : *vec) {
            std::cout << element << " ";
        }

        std::cout << std::endl;
    }

    void testSorting() {
        std::vector<int> *vec = genRandomList(DEFAULT_LIST_SIZE,
                                              DEFAULT_MIN_ELEMENT_VAL,
                                              DEFAULT_MAX_ELEMENT_VAL);
        bool prev = isDrawing;
        printVector(vec);
        isDrawing = false;
        bubbleSort(vec);
        isDrawing = prev;
        printVector(vec);
        isListSorted(vec);
    }

    std::vector<int> *getList() {
        return this->list;
    }

    std::array<int, 2> *getRedHighlight() {
        return &this->redHighlight;
    }

    void drawingStage() {
        std::unique_lock<std::mutex> lock(sortingMutex);
        drawTurn = true;
        sortingCV.notify_one();
        lock.unlock();

        lock.lock();
        sortingCV.wait(lock, [](){return !drawTurn;});
    }

    void simulationStage() {
        std::unique_lock<std::mutex> lock(sortingMutex);
        drawTurn = false;
        sortingCV.notify_one();
        lock.unlock();

        lock.lock();
        sortingCV.wait(lock, [](){return drawTurn;});
    }

    void finishThread() {
        drawTurn = true;
        sortingCV.notify_one();
    }

    bool biggerThan(std::vector<int> *arr, size_t k, size_t j) {
//        if (isDrawing) { drawingStage(); }
        return (*arr)[k] > (*arr)[j];
    }

    void swap(std::vector<int> *arr, size_t k, size_t j) {
        if (isDrawing) { drawingStage(); }

        int tmp = (*arr)[k];
        (*arr)[k] = (*arr)[j];
        (*arr)[j] = tmp;

        // Used while drawing to highlight currently swapped indexes
        redHighlight[0] = k;
        redHighlight[1] = j;
    }

    void initSorting() {
        //TODO: Check if we should copy or just pass the pointer
        isDrawing = true;
        bubbleSort(this->list);
        finishThread();
    }

    // TODO: Probably better to make this into it's own class
    void bubbleSort(std::vector<int> *arr) {
        for (int n = 1; n < arr->size() - 2; n++) {
            for (int j = 0; j < arr->size() - 1; j++) {
                if (biggerThan(arr, j, j + 1)) {
                    swap(arr, j, j + 1);
                }
            }
        }

    }
};

class SimulationDraw : public Simulation {
private:
    SDL_Renderer *renderer;
    SDL_Window *window;

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
            handleInput();
            SDL_RenderClear(renderer);
            drawArray();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

            SDL_RenderPresent(renderer);

            SDL_Delay(DELAY);

            if (not isListSorted(getList())) {
                simulationStage();
            }
            else {
                std::cout << "Finished sorting the list" << std::endl;
            }
        }

        SDL_DestroyRenderer(this->renderer);
        SDL_DestroyWindow(this->window);
        SDL_Quit();
    }

};

int main() {
    SimulationDraw drawSim;
    //drawSim.testSorting();
    drawSim.loop();

    return 0;
}

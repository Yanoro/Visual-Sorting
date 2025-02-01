#include <SDL2/SDL.h>
#include "simulationDraw.hpp"

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) != 0) {
        SDL_Log("Failed to get display mode: %s", SDL_GetError());
        return 1;
    }

    SimulationDraw drawSim(displayMode.w / 2);
    //drawSim.testSorting();
    drawSim.loop();

    return 0;
}

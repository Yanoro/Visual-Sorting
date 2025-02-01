#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <random>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <algorithm>

#define DEFAULT_LIST_SIZE 480
#define DEFAULT_MIN_ELEMENT_VAL 1 // Smallest value that can be in the list
#define DEFAULT_MAX_ELEMENT_VAL 100 // Biggest value that can be in the list

#define STEPS_UNTIL_SLEEP_START 10
#define STEP_INC 100
#define STEP_DEC 100

#define DELAY 1

#define forSort(x, n) for(size_t x = 0; x < n && not stopSimulation; x++)

class Simulation {
private:
    bool stopSimulation = false;

    size_t steps_until_sleep = STEPS_UNTIL_SLEEP_START;
    size_t step_counter = steps_until_sleep;

    std::vector<int> *list = NULL;
    std::array<int, 2> redHighlight;
    std::mutex sortingMutex;
    std::condition_variable sortingCV;

    public:

    Simulation (size_t listSize) {
        resetList(listSize);
        redHighlight[0] = 0;
        redHighlight[1] = 1;
    }

    void resetList(size_t listSize) {
        if (this->list != NULL) {
            delete this->list;
        }

        this->list = genRandomList(listSize,
                                   DEFAULT_MIN_ELEMENT_VAL,
                                   DEFAULT_MAX_ELEMENT_VAL);
    }


    std::vector<int> *genRandomList(size_t listSize, int minVal, int maxVal) {
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(minVal, maxVal);

        std::vector<int> *vec = new std::vector<int>(listSize);
        for (size_t i = 0; i < listSize; i++) {
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

    std::vector<int> *getList() {
        return this->list;
    }

    std::array<int, 2> *getRedHighlight() {
        return &this->redHighlight;
    }

    void stopSim() {
        stopSimulation = true;
    }

    void startSim() {
        stopSimulation = false;
    }

    void incSteps() {
        steps_until_sleep += STEP_INC;
        step_counter = steps_until_sleep;
    }

    void decSteps() {
        if (steps_until_sleep > STEP_DEC) {
            steps_until_sleep -= STEP_DEC;
            step_counter = steps_until_sleep;
        }
    }

    void printSteps() {
        std::cout << steps_until_sleep << std::endl;
    }

    bool biggerThan(std::vector<int> *arr, size_t k, size_t j) {
        return (*arr)[k] > (*arr)[j];
    }

    void swap(std::vector<int> *arr, size_t k, size_t j) {
        int tmp = (*arr)[k];
        (*arr)[k] = (*arr)[j];
        (*arr)[j] = tmp;

        // Used while drawing to highlight currently swapped indexes
        redHighlight[0] = k;
        redHighlight[1] = j;

        // For every steps_until_sleep swaps, sleep for a while
        step_counter = (step_counter - 1) % steps_until_sleep;
        if (step_counter == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
        }
    }

    void initSorting() {
        bubbleSort(this->list);
    }

    // TODO: Probably better to make this into it's own class
    void bubbleSort(std::vector<int> *arr) {
        forSort(n, arr->size() - 2) {
            forSort(j, arr->size() - 1) {
                if (biggerThan(arr, j, j + 1)) {
                    swap(arr, j, j + 1);
                }
            };
        };
    }

};


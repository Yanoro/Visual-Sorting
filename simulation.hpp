#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <random>
#include <string>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <algorithm>

#define DEFAULT_LIST_SIZE 480
#define DEFAULT_MIN_ELEMENT_VAL 1 // Smallest value that can be in the list
#define DEFAULT_MAX_ELEMENT_VAL 100 // Biggest value that can be in the list

#define STEPS_UNTIL_SLEEP_START 1
#define STEP_INC 100
#define STEP_DEC 100

#define DELAY 1

// Macro meant to be used inside the sorting functions,
#define forSort(init, cond, update) \
    for(init; cond && not stopSimulation; update)

//TODO: Remove later
class SortingAlgo;

class Simulation {
private:
    bool stopSimulation = false;

    unsigned int swapCount = 0;
    unsigned int comparisonCount = 0;

    size_t steps_until_sleep = STEPS_UNTIL_SLEEP_START;
    size_t step_counter = steps_until_sleep;

    SortingAlgo *algo;

    std::vector<int> *list = NULL;
    // Used while drawing to highlight currently swapped indexes
    std::vector<int> highlight;
    std::vector<SortingAlgo> algos;

    public:

    Simulation (size_t listSize, std::vector<SortingAlgo> *algos) {
        resetList(listSize);
        this->algos = *algos;
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

    std::vector<int> *getHighlights() {
        return &this->highlight;
    }

    int getSwapCount() {
        return swapCount;
    }

    int getCompCount() {
        return comparisonCount;
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

    void restartCount() {
        comparisonCount = 0;
        swapCount = 0;
    }

    void printSteps() {
        std::cout << steps_until_sleep << std::endl;
    }

    void setHighlights(std::vector<int> *newHighlights) {
        highlight.clear();
        highlight = *newHighlights;
    }

    bool biggerThan(std::vector<int> *arr, size_t k, size_t j) {
        comparisonCount++;
        return (*arr)[k] > (*arr)[j];
    }

    bool lessThan(std::vector<int> *arr, size_t k, size_t j) {
        comparisonCount++;
        return (*arr)[k] < (*arr)[j];
    }

    void step() {
        step_counter = (step_counter - 1) % steps_until_sleep;
        if (step_counter == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DELAY));
        }
    }

    void set(std::vector<int> *arr, size_t k, int j) {
        (*arr)[k] = j;

        std::vector<int> highlights = {static_cast<int>(k)};
        setHighlights(&highlights);

        step();
    }

    void swap(std::vector<int> *arr, size_t k, size_t j) {
        int tmp = (*arr)[k];
        (*arr)[k] = (*arr)[j];
        (*arr)[j] = tmp;

        swapCount++;

        std::vector<int> highlights = {
            static_cast<int>(k),
            static_cast<int>(j)
        };
        setHighlights(&highlights);

        step();
    }

    void initSorting() {
        mergeSort(this->list);
        //bubbleSort(this->list);
    }

    void mergeSort(std::vector<int> *arr) {
        helper(arr, 0, arr->size() - 1);
    }

    void helper(std::vector<int> *arr, size_t start, size_t end) {
        size_t size = end - start;
        if (size == 0) {
            return;
        }
        if (size == 1) {
            if (biggerThan(arr, start, end)) {
                swap(arr, start, end);
                return;
            }
        }
        else {
            helper(arr, start, start + (size / 2));
            helper(arr, start + (size/2) + 1, end);
            merge(arr, start, start + (size/2), start + (size/2) + 1, end);
        }
    };

    void merge(std::vector<int> *arr, size_t s1, size_t e1, size_t s2, size_t e2) {
        size_t size1 = e1 - s1 + 1;
        size_t size2 = e2 - s2 + 1;
        size_t orig_s1 = s1;
        std::vector<int> mergedList(size1 + size2);
        forSort(size_t i = 0, s1 <= e1 or s2 <= e2, i++) {
            if (s1 <= e1 and (lessThan(arr, s1, s2) or s2 > e2)) {
                mergedList[i] = (*arr)[s1++];
            }
            else {
                mergedList[i] = (*arr)[s2++];
            }
        }

        forSort(size_t i = 0, i < size1 + size2, i++) {
            set(arr, orig_s1 + i, mergedList[i]);
        }

    };

    void printVector(std::vector<int> *vec) {
        for (const auto element : *vec) {
            std::cout << element << " ";
        }

        std::cout << std::endl;
    }

    void testSorting() {
        std::vector<int> *vec = genRandomList(100,
                                              DEFAULT_MIN_ELEMENT_VAL,
                                              DEFAULT_MAX_ELEMENT_VAL);


        printVector(vec);
        mergeSort(vec);
        printVector(vec);
        isListSorted(vec);
    }

};

class SortingAlgo {
    private:
     std::string name;
     Simulation *sim;
    public:
    SortingAlgo(std::string algoName, Simulation *sim) {
        this->name = algoName;
        this->sim = sim;
    }

    Simulation *getSim() {
        return this->sim;
    }

    void sort(std::vector<int> *arr);
};

class BubbleSort : public SortingAlgo {
public:
    // TODO: Probably better to make this into it's own class
    // Test this again to see if it works with the forSort
    void sort(std::vector<int> *arr) {
        forSort(size_t n = 0, n < arr->size() - 2, n++) {
            forSort(size_t j = 0, j < arr->size() - 1, j++) {
                if (getSim()->biggerThan(arr, j, j + 1)) {
                    getSim()->swap(arr, j, j + 1);
                }
            };
        };
    }

};

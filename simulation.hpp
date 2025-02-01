#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <random>
#include <mutex>
#include <condition_variable>

#define DEFAULT_LIST_SIZE 480
#define DEFAULT_MIN_ELEMENT_VAL 1 // Smallest value that can be in the list
#define DEFAULT_MAX_ELEMENT_VAL 100 // Biggest value that can be in the list

class Simulation {
private:
  bool isDrawing = false;
  bool drawTurn = true;

  std::vector<int> *list;
  std::array<int, 2> redHighlight;
  std::mutex sortingMutex;
  std::condition_variable sortingCV;


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
        sortingCV.wait(lock, [this](){return !drawTurn;});
    }

    void simulationStage() {
        std::unique_lock<std::mutex> lock(sortingMutex);
        drawTurn = false;
        sortingCV.notify_one();
        lock.unlock();

        lock.lock();
        sortingCV.wait(lock, [this](){return drawTurn;});
    }

    void finishThread() {
        drawTurn = true;
        sortingCV.notify_one();
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
    }

    void initSorting() {
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

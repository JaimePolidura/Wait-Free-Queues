#pragma once

#include "shared.hpp"

namespace jaime {

template<typename T>
class spsc_object_pool {
private:
    std::atomic<std::queue<T> *> producers;
    std::atomic<std::queue<T> *> consumers;
    std::function<T ()> creator;

    std::atomic_bool producerPuttingValue;

public:
    explicit spsc_object_pool(std::function<T()> creator): creator(creator) {
        this->producers.store(new std::queue<T>(), std::memory_order_relaxed);
        this->consumers.store(new std::queue<T>(), std::memory_order_relaxed);
    }
    
    void populate(int nObjects) {
        std::queue<T> * consumersQueue = this->consumers.load(std::memory_order_relaxed);

        for (int i = 0; i < nObjects; ++i) {
            consumersQueue->push(this->creator());
        }
    }

    T take() {
        std::queue<T> * consumersQueue = this->consumers.load(std::memory_order_acquire);
        std::queue<T> * producersQueue = this->producers.load(std::memory_order_acquire);

        bool consumersEmpty = consumersQueue->empty();
        bool producersEmpty = producersQueue->empty();

        if(!consumersEmpty){
            T item = consumersQueue->front();
            consumersQueue->pop();

            return item;
        }
        if(consumersEmpty && producersEmpty) {
            return this->creator();
        }

        this->consumers.store(producersQueue, std::memory_order_release);
        this->producers.store(consumersQueue, std::memory_order_release);

        jaime::utils::spin_wait_on(&this->producerPuttingValue, true);

        return take();
    }

    void put(const T& toPut) {
        producerPuttingValue.store(true, std::memory_order_release);

        std::queue<T> * producersQueue = this->producers.load(std::memory_order_acquire);
        producersQueue->push(toPut);

        producerPuttingValue.store(false, std::memory_order_release);
    }

};

}

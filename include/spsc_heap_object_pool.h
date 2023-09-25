#pragma once

#include "shared.hpp"

namespace jaime {

template<typename T>
class spsc_heap_object_pool {
private:
    std::atomic<std::queue<T *> *> producers;
    std::atomic<std::queue<T *> *> consumers;

    std::atomic_bool producerPuttingValue;

public:
    explicit spsc_heap_object_pool() {
        this->producers.store(new std::queue<T *>(), std::memory_order_relaxed);
        this->consumers.store(new std::queue<T *>(), std::memory_order_relaxed);
    }
    
    void populate(int nObjects, auto ... args) {
        std::queue<T *> * consumersQueue = this->consumers.load(std::memory_order_relaxed);

        for (int i = 0; i < nObjects; ++i) {
            consumersQueue->push(new T(args...));
        }
    }

    T * take() {
        return takeOrCreateWithArgs();
    }

    T * takeOrCreateWithArgs(auto ... args) {
        std::queue<T *> * consumersQueue = this->consumers.load(std::memory_order_acquire);
        std::queue<T *> * producersQueue = this->producers.load(std::memory_order_acquire);

        bool consumersEmpty = consumersQueue->empty();
        bool producersEmpty = producersQueue->empty();

        if(!consumersEmpty){
            return this->popFromQueue(consumersQueue);
        }
        if(consumersEmpty && producersEmpty) {
            return new T(args...);
        }

        this->consumers.store(producersQueue, std::memory_order_release);
        this->producers.store(consumersQueue, std::memory_order_release);

        jaime::utils::spin_wait_on(&this->producerPuttingValue, true);
        
        return this->popFromQueue(producersQueue); //Already swapped
    }

    void put(T * toPut) {
        producerPuttingValue.store(true, std::memory_order_release);

        std::atomic_thread_fence(std::memory_order_acquire);

        std::queue<T *> * producersQueue = this->producers.load(std::memory_order_acquire);
        producersQueue->push(toPut);

        producerPuttingValue.store(false, std::memory_order_release);
    }

private:
    T * popFromQueue(std::queue<T *> * queue) {
        T * item = queue->front();
        queue->pop();

        return item;
    }

};

}

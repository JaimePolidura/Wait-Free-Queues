#pragma once

#include "utils/utils.hpp"

#include "utils/spin_lock.hpp"

namespace jaime {

template<typename T>
class spsc_heap_object_pool {
private:
    std::atomic<std::queue<T *> *> producers;
    std::atomic<std::queue<T *> *> consumers;

    jaime::utils::spin_lock producer_putting_value_spin_lock;

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

        if(consumersEmpty && producersEmpty) {
            return new T(args...);
        }
        if(!consumersEmpty){
            return this->popFromQueue(consumersQueue);
        }

        this->consumers.store(producersQueue, std::memory_order_release);
        this->producers.store(consumersQueue, std::memory_order_release);

        this->producer_putting_value_spin_lock.wait_until_unlocked();

        return this->takeOrCreateWithArgs(args...);
    }

    void put(T * toPut) {
        this->producer_putting_value_spin_lock.lock();

        std::queue<T *> * producersQueue = this->producers.load(std::memory_order_acquire);
        producersQueue->push(toPut);

        this->producer_putting_value_spin_lock.unlock();
    }

private:
    T * popFromQueue(std::queue<T *> * queue) {
        T * item = queue->front();
        queue->pop();

        return item;
    }
};

}

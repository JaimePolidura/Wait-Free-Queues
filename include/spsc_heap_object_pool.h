#pragma once

#include "utils/utils.hpp"

#include "utils/spin_lock.hpp"

#include <emmintrin.h>

namespace jaime {

template<typename T>
class spsc_heap_object_pool {
private:
    std::atomic<std::queue<T *> *> producers;
    std::atomic<std::queue<T *> *> consumers;

    jaime::utils::spin_lock producer_putting_value_spin_lock;

public:
    explicit spsc_heap_object_pool() {
        this->producers.store(new std::queue<T *>(), std::memory_order_release);
        this->consumers.store(new std::queue<T *>(), std::memory_order_release);
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

        std::atomic_thread_fence(std::memory_order_release);

        this->producer_putting_value_spin_lock.wait_until_unlocked();

        return this->popFromQueue(producersQueue);
    }

    void put(T * toPut) {
        this->producer_putting_value_spin_lock.lock();

        this->producers.load(std::memory_order_acquire)->push(toPut);

        this->producer_putting_value_spin_lock.unlock();
    }

    ~spsc_heap_object_pool() {
        delete this->consumers;
        delete this->producers;
    }

private:
    T * popFromQueue(std::queue<T *> * queue) {
        T * item = queue->front();
        queue->pop();

        return item;
    }
};

}
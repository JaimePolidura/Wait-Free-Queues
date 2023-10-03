#pragma once

#include "utils/shared.hpp"

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
        return take_or_create_with_args();
    }

    T * take_or_create_with_args(auto ... args) {
        std::queue<T *> * consumers_queue = this->consumers.load(std::memory_order_acquire);
        if(!consumers_queue->empty()){
            return this->popFromQueue(consumers_queue);
        }

        std::queue<T *> * producers_queue = this->producers.load(std::memory_order_acquire);
        if(consumers_queue->empty() && producers_queue->empty()) {
            return new T(args...);
        }

        this->consumers.store(producers_queue, std::memory_order_release);
        this->producers.store(consumers_queue, std::memory_order_release);

        std::atomic_thread_fence(std::memory_order_release);

        this->producer_putting_value_spin_lock.wait_until_unlocked();

        return this->popFromQueue(producers_queue);
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

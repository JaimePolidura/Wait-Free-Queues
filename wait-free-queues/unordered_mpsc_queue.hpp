#pragma once

#include "utils/shared.hpp"
#include "utils/per_thread_slot_array_allocator.h"
#include "spsc_queue.hpp"
#include "mpsc_queue.h"

namespace jaime {

template<typename T>
class unordered_mpsc_queue : private jaime::mpsc_queue<T> {
private:
    using slot_t = int;

public:
    explicit unordered_mpsc_queue(int n_slots): jaime::mpsc_queue<T>(n_slots) {}

    void enqueue(const T& value) {
        this->get_queue()->enqueue(value);
    }

    std::optional<T> dequeue() {
        for(typename jaime::mpsc_queue<T>::dequeue_iterator it = this->begin(); it != this->end(); ++it){
            slot_t actual_slot_to_dequeue = *it;
            spsc_queue<T> * queue = this->slots + actual_slot_to_dequeue;
            std::optional<T> dequeued_optional = queue->dequeue();

            if(dequeued_optional.has_value()){
                this->last_slot_dequeued = actual_slot_to_dequeue;
                return dequeued_optional.value();
            }
        }

        return std::nullopt;
    }
};

}
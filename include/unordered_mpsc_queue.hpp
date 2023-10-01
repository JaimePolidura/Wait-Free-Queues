#pragma once

#include "utils/utils.hpp"
#include "utils/per_thread_slot_array_allocator.h"

#include "spsc_queue.hpp"

namespace jaime {

static thread_local int thread_number;
static std::atomic<int> thread_numbers;

template<typename T>
class unordered_mpsc_queue {
private:
    using slot_t = int;

    jaime::utils::per_thread_slot_array_allocator slot_allocator;
    spsc_queue<T> * slots;
    slot_t n_slots;

    uint8_t padding_cache_line[64];
    slot_t last_slot_dequeued{};

public:
    explicit unordered_mpsc_queue(int n_slots):
            slots(new spsc_queue<T>[n_slots]),
            slot_allocator(n_slots),
            n_slots(n_slots) {
        for (int i = 0; i < n_slots; ++i) {
            new (&this->slots[i]) jaime::spsc_queue<T>();
        }
    }

    void enqueue(const T& value) {
        spsc_queue<T> * queue = this->slots + this->get_slot();
        queue->enqueue(value);
    }

    std::optional<T> dequeue() {
        slot_t starting_slot_to_dequeue = this->get_next_slot_to_dequeue(this->last_slot_dequeued);
        slot_t actual_slot_to_dequeue = starting_slot_to_dequeue;

        do {
            spsc_queue<T> * queue = this->slots + actual_slot_to_dequeue;
            std::optional<T> dequeued_optional = queue->dequeue();

            if(dequeued_optional.has_value()){
                this->last_slot_dequeued = actual_slot_to_dequeue;
                return dequeued_optional.value();
            }

            actual_slot_to_dequeue = this->get_next_slot_to_dequeue(actual_slot_to_dequeue);

        } while (starting_slot_to_dequeue != actual_slot_to_dequeue);

        return std::nullopt;
    }

private:
    int get_slot() {
        if(jaime::thread_number == 0) {
            jaime::thread_number = jaime::thread_numbers.fetch_add(1) + 1;
        }

        return this->slot_allocator.allocate_or_get(jaime::thread_number);
    }

    inline int get_next_slot_to_dequeue(int prev) {
        return prev + 1 < this->n_slots ? ++prev :  0;
    }
};

}
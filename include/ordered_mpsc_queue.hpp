#pragma once

#include "utils/utils.hpp"

#include "spsc_queue.hpp"
#include "utils/per_thread_slot_array_allocator.h"

namespace jaime::lock_free {

static thread_local int thread_number;
static std::atomic<int> thread_numbers;

template<typename T>
class ordered_mpsc_queue {
private:
    using slot_t = int;

    jaime::utils::per_thread_slot_array_allocator slot_allocator;
    spsc_queue<T> * slots;
    slot_t n_slots;

    uint8_t padding_cache_line[64];
    std::atomic_uint64_t last_timestamp_enqueued;

    timestamp_t last_timestamp_dequeued{};
    slot_t last_slot_dequeued{};

public:
    explicit ordered_mpsc_queue(int n_slots):
        slots(new spsc_queue<T>[n_slots]),
        last_timestamp_enqueued(0),
        slot_allocator(n_slots),
        n_slots(n_slots) {
        for (int i = 0; i < n_slots; ++i) {
            new (&this->slots[i]) jaime::spsc_queue<T>();
        }
    }

    void enqueue(const T& value) {
        timestamp_t timestampEnqueued = jaime::utils::increment_and_get(this->last_timestamp_enqueued);
        int slot = this->get_slot();
        spsc_queue<T> * queue = this->slots + slot;

        queue->enqueue(value, timestampEnqueued);
    }

    std::optional<T> dequeue() {
        slot_t starting_slot_to_dequeue = this->get_next_slot_to_dequeue(this->last_slot_dequeued);
        slot_t actual_slot_to_dequeue = starting_slot_to_dequeue;

        do {
            spsc_queue<T> * queue = this->slots + actual_slot_to_dequeue;
            timestamp_t timestamp_expected_to_dequeue = this->last_timestamp_dequeued + 1;
            std::optional<T> dequeued_optional = queue->dequeue(timestamp_expected_to_dequeue);

            if(dequeued_optional.has_value()){
                this->last_timestamp_dequeued = timestamp_expected_to_dequeue;
                this->last_slot_dequeued = actual_slot_to_dequeue;

                return dequeued_optional.value();
            }

            actual_slot_to_dequeue = this->get_next_slot_to_dequeue(actual_slot_to_dequeue);

        } while (starting_slot_to_dequeue != actual_slot_to_dequeue);

        return std::nullopt;
    }

private:
    int get_slot() {
        if(jaime::lock_free::thread_number == 0) {
            jaime::lock_free::thread_number = jaime::lock_free::thread_numbers.fetch_add(1) + 1;
        }

        slot_t slot = this->slot_allocator.allocate_or_get(jaime::lock_free::thread_number);

        if(slot != -1){
            return slot;
        } else {
            throw std::runtime_error("The thread capacity of unordered_mpsc_queue has been exceeded");
        }
    }
    inline int get_next_slot_to_dequeue(int prev) {
        return prev + 1 < this->n_slots ? ++prev :  0;
    }
};

}
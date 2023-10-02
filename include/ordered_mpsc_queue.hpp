#pragma once

#include "utils/utils.hpp"

#include "spsc_queue.hpp"
#include "utils/per_thread_slot_array_allocator.h"
#include "mpsc_queue.h"

namespace jaime::lock_free {

template<typename T>
class ordered_mpsc_queue : private jaime::mpsc_queue<T> {
private:
    using slot_t = int;

    std::atomic_uint64_t last_timestamp_enqueued{0};
    uint8_t padding_cache_line[64];
    timestamp_t last_timestamp_dequeued{0};

public:
    explicit ordered_mpsc_queue(int n_slots): jaime::mpsc_queue<T>(n_slots) {}

    void enqueue(const T& value) {
        timestamp_t timestampEnqueued = this->last_timestamp_enqueued.fetch_add(1, std::memory_order_relaxed) + 1;
        this->get_queue()->enqueue(value, timestampEnqueued);
    }

    std::optional<T> dequeue() {
        for(typename jaime::mpsc_queue<T>::dequeue_iterator it = this->begin(); it != this->end(); ++it){
            slot_t actual_slot_to_dequeue = * it;
            jaime::spsc_queue<T> * queue = this->slots + actual_slot_to_dequeue;

            timestamp_t timestamp_expected_to_dequeue = this->last_timestamp_dequeued + 1;
            std::optional<T> dequeued_optional = queue->dequeue(timestamp_expected_to_dequeue);

            if(dequeued_optional.has_value()){
                this->last_timestamp_dequeued = timestamp_expected_to_dequeue;
                this->last_slot_dequeued = actual_slot_to_dequeue;

                return dequeued_optional.value();
            }
        }

        return std::nullopt;
    }
};

}
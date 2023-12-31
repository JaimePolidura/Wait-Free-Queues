#pragma once

#include "utils/per_thread_slot_array_allocator.h"
#include "utils/shared.hpp"
#include "spsc_queue.hpp"
#include "mpsc_queue.h"

namespace jaime::lock_free {

template<typename T>
class ordered_mpsc_queue : private jaime::mpsc_queue<T> {
private:
    using timestamp_t = uint64_t;
    using slot_t = int;

    std::atomic_uint64_t last_timestamp_enqueued{0};
    uint8_t padding_cache_line[64];
    timestamp_t last_timestamp_dequeued{0};

public:
    explicit ordered_mpsc_queue(slot_t n_slots): jaime::mpsc_queue<T>(n_slots) {}

    void enqueue(const T& value) {
        timestamp_t timestampEnqueued = this->last_timestamp_enqueued.fetch_add(1, std::memory_order_relaxed) + 1;
        this->get_queue()->enqueue(value, timestampEnqueued);
    }

    std::optional<T> dequeue() {
        typename jaime::mpsc_queue<T>::dequeue_iterator it = this->iterator();

        while(it.hasNext()){
            slot_t actual_slot_to_dequeue = it.next();
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

    template<typename Rep, typename Period>
    std::vector<T> dequeue_all_or_sleep_for(std::chrono::duration<Rep, Period> duration) {
        std::vector<T> values = this->dequeue_all();

        while(values.empty()){
            std::this_thread::sleep_for(duration);
            values = this->dequeue_all();
        }

        return values;
    }

    std::vector<T> dequeue_all() {
        std::vector<T> dequeued{};

        typename jaime::mpsc_queue<T>::dequeue_iterator it = this->iterator();

        while (it.hasNext()) {
            slot_t actual_slot_to_dequeue = it.next();
            jaime::spsc_queue<T> * queue = this->slots + actual_slot_to_dequeue;

            timestamp_t timestamp_expected_to_dequeue = this->last_timestamp_dequeued + 1;
            std::optional<T> dequeued_optional = queue->dequeue(timestamp_expected_to_dequeue);

            while(dequeued_optional.has_value()){
                dequeued.push_back(dequeued_optional.value());
                this->last_timestamp_dequeued = timestamp_expected_to_dequeue;
                this->last_slot_dequeued = actual_slot_to_dequeue;

                timestamp_expected_to_dequeue = this->last_timestamp_dequeued + 1;
                dequeued_optional = queue->dequeue(timestamp_expected_to_dequeue);
            }
        }

        return dequeued;
    }
};

}
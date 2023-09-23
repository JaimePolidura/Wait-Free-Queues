#pragma once

#include "shared.h"

#include "spsc_queue.hpp"

namespace jaime::lock_free {

template<typename T, std::size_t nSlots>
class ordered_mpsc_queue {
private:
    spsc_queue<T> slots[nSlots]{};
    std::atomic_uint64_t lastTimestampEnqueued;

    timestamp_t lastTimestampDequeued{};
    int lastSlotDequeued{};

public:
    ordered_mpsc_queue() = default;

    void enqueue(const T& value) {
        timestamp_t timestampEnqueued = increment_and_get(this->lastTimestampEnqueued);
        int slotIndex = get_thread_id();

        if(slotIndex + 1 > nSlots){
            throw std::out_of_range("Increase ordered_mpsc_queue size");
        }

        spsc_queue<T> * queue = &this->slots[slotIndex];
        queue->enqueue(value, timestampEnqueued);
    }

    std::optional<T> dequeue() {
        int startingSlotToDequeue = this->getNextSlotToDequeue(this->lastSlotDequeued);
        int actualSlotToDequeue = startingSlotToDequeue;

        do {
            spsc_queue<T> * queue = &this->slots[actualSlotToDequeue];
            timestamp_t timestampExpectedToDequeue = this->lastTimestampDequeued + 1;
            std::optional<T> dequeuedOptional = queue->dequeue(timestampExpectedToDequeue);

            if(dequeuedOptional.has_value()){
                this->lastTimestampDequeued = timestampExpectedToDequeue;
                this->lastSlotDequeued = actualSlotToDequeue;

                return dequeuedOptional.value();
            }

            actualSlotToDequeue = this->getNextSlotToDequeue(actualSlotToDequeue);

        } while (startingSlotToDequeue != actualSlotToDequeue);

        return std::nullopt;
    }

private:
    inline int getNextSlotToDequeue(int prev) {
        return prev + 1 < nSlots ? ++prev :  0;
    }
};

}
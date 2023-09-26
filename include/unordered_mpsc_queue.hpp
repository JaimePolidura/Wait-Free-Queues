#pragma once

#include "utils/utils.hpp"

#include "spsc_queue.hpp"

namespace jaime {

template<typename T, std::size_t nSlots>
class unordered_mpsc_queue {
private:
    spsc_queue<T> slots[nSlots]{};

    int lastSlotDequeued{};

public:
    unordered_mpsc_queue() = default;

    void enqueue(const T& value) {
        int slotIndex = jaime::utils::get_thread_id();

        if(slotIndex + 1 > nSlots){
            throw std::out_of_range("Increase ordered_mpsc_queue size");
        }

        spsc_queue<T> * queue = &this->slots[slotIndex];
        queue->enqueue(value);
    }

    std::optional<T> dequeue() {
        int startingSlotToDequeue = this->getNextSlotToDequeue(this->lastSlotDequeued);
        int actualSlotToDequeue = startingSlotToDequeue;

        do {
            spsc_queue<T> * queue = &this->slots[actualSlotToDequeue];
            std::optional<T> dequeuedOptional = queue->dequeue();

            if(dequeuedOptional.has_value()){
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
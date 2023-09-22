#pragma once

#include "shared.h"

#include "SingleProducerSingleConsumer.h"

template<typename T, std::size_t nSlots>
class OrderedMultipleProducerSingleConsumer {
private:
    SingleProducerSingleConsumer<T> slots[nSlots]{};
    std::atomic_uint64_t lastTimestampEnqueued;

    timestamp_t lastTimestampDequeued{};
    int lastSlotDequeued{};

public:
    OrderedMultipleProducerSingleConsumer() = default;

    void enqueue(const T& value) {
        timestamp_t timestampEnqueued = increment_and_get(this->lastTimestampEnqueued);
        int slotIndex = get_thread_id();

        if(slotIndex + 1 > nSlots){
            throw std::out_of_range("Increase OrderedMultipleProducerSingleConsumer size");
        }

        SingleProducerSingleConsumer<T> * queue = &this->slots[slotIndex];
        queue->enqueue(value, timestampEnqueued);
    }

    std::optional<T> dequeue() {
        int startingSlotToDequeue = this->getNextSlotToDequeue(this->lastSlotDequeued);
        int actualSlotToDequeue = startingSlotToDequeue;

        do {
            SingleProducerSingleConsumer<T> * queue = &this->slots[actualSlotToDequeue];
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
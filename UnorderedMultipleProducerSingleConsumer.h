#pragma once

#include "shared.h"

#include "SingleProducerSingleConsumer.h"

template<typename T, std::size_t nSlots>
class UnorderedMultipleProducerSingleConsumer {
private:
    SingleProducerSingleConsumer<T> slots[nSlots]{};

    int lastSlotDequeued{};

public:
    UnorderedMultipleProducerSingleConsumer() = default;

    void enqueue(const T& value) {
        int slotIndex = get_thread_id();

        if(slotIndex + 1 > nSlots){
            throw std::out_of_range("Increase OrderedMultipleProducerSingleConsumer size");
        }

        SingleProducerSingleConsumer<T> * queue = &this->slots[slotIndex];
        queue->enqueue(value);
    }

    std::optional<T> dequeue() {
        int startingSlotToDequeue = this->getNextSlotToDequeue(this->lastSlotDequeued);
        int actualSlotToDequeue = startingSlotToDequeue;

        do {
            SingleProducerSingleConsumer<T> * queue = &this->slots[actualSlotToDequeue];
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
    int getNextSlotToDequeue(int prev) {
        return prev + 1 < nSlots ? ++prev :  0;
    }
};
#pragma once

#include "spsc_queue.hpp"
#include "utils/per_thread_slot_array_allocator.h"
#include "spsc_queue.hpp"

namespace jaime {

template<typename T>
class mpsc_queue {
protected:
    using slot_t = int;

    jaime::utils::per_thread_slot_array_allocator slot_allocator;
    spsc_queue<T> * slots;
    slot_t n_slots;

    uint8_t padding_cache_line[64];
    slot_t last_slot_dequeued{0};

    std::atomic<int> thread_numbers{-1};

public:
    explicit mpsc_queue(slot_t n_slots):
            slots(new spsc_queue<T>[n_slots]),
            slot_allocator(n_slots),
            n_slots(n_slots) {
        for (int i = 0; i < n_slots; ++i) {
            new (&this->slots[i]) jaime::spsc_queue<T>();
        }
    }

    spsc_queue<T> * get_queue() {
        return this->slots + this->get_slot();
    }

    slot_t get_slot() {
        static thread_local int thread_number = -1;

        if(thread_number == -1) {
            thread_number = this->thread_numbers.fetch_add(1) + 1;
        }

        return this->slot_allocator.allocate_or_get(thread_number);
    }

    inline slot_t get_next_slot_to_dequeue(slot_t prev) {
        return prev + 1 < this->n_slots ? ++prev : 0;
    }

    ~mpsc_queue() {
        delete[] this->slots;
    }

protected:
    class dequeue_iterator {
    private:
        slot_t n_slots;
        slot_t actual;
        int n_iterations{0};

    public:
        dequeue_iterator(slot_t n_slots, slot_t start): actual(start), n_slots(n_slots) {}

        bool hasNext() {
            return this->n_iterations < this->n_slots;
        }

        slot_t next() {
            this->n_iterations++;
            return this->actual = this->get_next();
        }

    private:
        inline slot_t get_next() {
            return this->actual + 1 < this->n_slots ? this->actual + 1 : 0;
        }
    };

public:
    dequeue_iterator iterator() {
        return dequeue_iterator(this->n_slots, this->get_next_slot_to_dequeue(this->last_slot_dequeued));
    }
};

}
#pragma once

#include "spsc_queue.hpp"
#include "utils/per_thread_slot_array_allocator.h"
#include "spsc_queue.hpp"

namespace jaime {

static thread_local int thread_number;
static std::atomic<int> thread_numbers;

template<typename T>
class mpsc_queue {
protected:
    using slot_t = int;

    jaime::utils::per_thread_slot_array_allocator slot_allocator;
    spsc_queue<T> * slots;
    slot_t n_slots;

    uint8_t padding_cache_line[64];
    slot_t last_slot_dequeued{};

public:
    explicit mpsc_queue(int n_slots):
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
        if(jaime::thread_number == 0) {
            jaime::thread_number = jaime::thread_numbers.fetch_add(1) + 1;
        }

        return this->slot_allocator.allocate_or_get(jaime::thread_number);
    }

    inline slot_t get_next_slot_to_dequeue(int prev) {
        return prev + 1 < this->n_slots ? ++prev : 0;
    }

    ~mpsc_queue() {
        delete[] this->slots;
    }

protected:
    class dequeue_iterator {
        using slot_t = int;
    private:
        slot_t n_slots;
        slot_t start;
        slot_t actual;
        bool first_iteration_already_run;

    public:
        dequeue_iterator(slot_t n_slots, slot_t start): start(start), actual(start), n_slots(n_slots), first_iteration_already_run(false) {}

        bool operator==(const dequeue_iterator& other) const {
            return this->actual == other.actual;
        }

        bool operator!=(const dequeue_iterator& other) const {
            return this->actual != other.actual || !this->first_iteration_already_run;
        }

        slot_t operator*() {
            this->first_iteration_already_run = true;
            return this->actual;
        }

        dequeue_iterator operator++() {
            this->actual = this->actual + 1 < this->n_slots ? ++this->actual : 0;
            return *this;
        }
    };

public:
    dequeue_iterator begin() {
        return dequeue_iterator(this->n_slots, this->get_next_slot_to_dequeue(this->last_slot_dequeued));
    }

    dequeue_iterator end() {
        return dequeue_iterator(this->n_slots, this->last_slot_dequeued);
    }
};

}
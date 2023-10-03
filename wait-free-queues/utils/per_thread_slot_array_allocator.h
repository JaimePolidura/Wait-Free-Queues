#pragma once

#include "utils/shared.hpp"

namespace jaime::utils {

using slot_t = int;

struct allocation_result {
    bool success;
    slot_t slot;
};

class array_entry {
private:
    std::atomic<int> owner;

public:
    array_entry(): owner(-1) {}

    bool is_free() const {
        return this->owner.load(std::memory_order_acquire) == -1;
    }

    bool is_owned_by(int thread_id) const {
        return this->owner.load(std::memory_order_acquire) == thread_id;
    }

    void free() {
        this->owner.store(-1, std::memory_order_release);
    }

    bool try_acquire(int thread_id) {
        auto expected_not_owned = -1;

        return this->owner.compare_exchange_weak(expected_not_owned, thread_id, std::memory_order_release);
    }
};

class per_thread_slot_array_allocator {
private:
    std::vector<array_entry> array;
    slot_t number_slots;

public:
    explicit per_thread_slot_array_allocator(slot_t number_slots): array(std::vector<array_entry>(number_slots)), number_slots(number_slots) {}

    slot_t allocate_or_get(int thread_number) {
        int slot = this->get_slot_owned_by(thread_number);

        if(slot != -1){
            return slot;
        }

        return this->allocate(thread_number).slot;
    }

    slot_t get_slot_owned_by(int thread_id) {
        slot_t start_slot = thread_id % this->number_slots;
        slot_t actual_slot = start_slot;

        do {
            if (this->array[actual_slot].is_owned_by(thread_id)) {
                return actual_slot;
            }

            actual_slot = actual_slot + 1 < this->array.size() ? ++actual_slot : 0;
        } while (start_slot != actual_slot);

        return -1;
    }

    allocation_result allocate(int thread_id) {
        slot_t start_slot = thread_id % this->number_slots;
        slot_t actual_slot = start_slot;

        do {
            array_entry& slot_array_entry = this->array[actual_slot];

            if (slot_array_entry.is_free() && slot_array_entry.try_acquire(thread_id)) {
                return allocation_result{.success = true, .slot = actual_slot};
            }

            actual_slot = actual_slot + 1 < this->array.size() ? ++actual_slot : 0;
        } while (start_slot != actual_slot);

        return allocation_result{.success = false, .slot = -1};
    }

    void deallocate(int thread_id) {
        slot_t start_slot = thread_id % this->number_slots;
        slot_t actual_slot = start_slot;

        do {
            array_entry& slot_array_entry = this->array[actual_slot];

            if (slot_array_entry.is_owned_by(thread_id)) {
                slot_array_entry.free();
                return;
            }

            actual_slot = actual_slot + 1 < this->array.size() ? ++actual_slot : 0;
        } while (start_slot != actual_slot);
    }
};

}
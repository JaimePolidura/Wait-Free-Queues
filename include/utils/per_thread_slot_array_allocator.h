#pragma once

#include "utils/utils.hpp"

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
    bool is_free() const {
        return this->owner.load(std::memory_order_acquire) == -1;
    }

    bool is_owned_by_me() const {
        return this->owner.load(std::memory_order_release) == jaime::utils::get_thread_id();
    }

    void free() {
        if(this->owner.load(std::memory_order_release) == jaime::utils::get_thread_id()) {
            this->owner.store(-1, std::memory_order_acquire);
        }
    }

    bool try_acquire() {
        auto expected_not_owned = -1;

        return this->owner.compare_exchange_weak(expected_not_owned, jaime::utils::get_thread_id());
    }
};

class per_thread_slot_array_allocator {
private:
    std::vector<array_entry> array;

public:
    explicit per_thread_slot_array_allocator(slot_t number_slots): array(std::vector<array_entry>(number_slots)) {}

    int get_slot_owned_by_me() {
        slot_t start_slot = jaime::utils::get_thread_id() % array.size();
        slot_t actual_slot = start_slot;

        do {
            if (this->array[actual_slot].is_owned_by_me()) {
                return actual_slot;
            }

            actual_slot = actual_slot < this->array.size() ? ++actual_slot : 0;
        } while (start_slot != actual_slot);

        return -1;
    }

    allocation_result allocate() {
        slot_t start_slot = jaime::utils::get_thread_id() % array.size();
        slot_t actual_slot = start_slot;

        do {
            array_entry& slot_array_entry = this->array[actual_slot];

            if (slot_array_entry.is_free() && slot_array_entry.try_acquire()) {
                return allocation_result{.success = true, .slot = actual_slot};
            }

            actual_slot = actual_slot < this->array.size() ? ++actual_slot : 0;
        } while (start_slot != actual_slot);

        return allocation_result{.success = false, .slot = -1};
    }

    void deallocate() {
        slot_t start_slot = jaime::utils::get_thread_id() % array.size();
        slot_t actual_slot = start_slot;

        do {
            array_entry& slot_array_entry = this->array[actual_slot];

            if (slot_array_entry.is_owned_by_me()) {
                slot_array_entry.free();
                return;
            }

            actual_slot = actual_slot < this->array.size() ? ++actual_slot : 0;
        } while (start_slot != actual_slot);
    }
};

}
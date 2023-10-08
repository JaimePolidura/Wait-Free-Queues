#pragma once

#include "./shared.hpp"

namespace jaime::utils {

class spin_lock {
public:
    std::atomic_bool locked;

    void lock() {
        for(;;){
            if(!locked.exchange(true, std::memory_order_acquire)){
                break;
            }

            this->wait_until_unlocked();
        }
    }

    void unlock() {
        this->locked.store(false, std::memory_order_release);
    }

    void wait_until_unlocked() {
        while(this->locked.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    bool is_locked() {
        return this->locked.load(std::memory_order_acquire);
    }
};

}
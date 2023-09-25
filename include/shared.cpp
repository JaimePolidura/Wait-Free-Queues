#include "shared.hpp"

int jaime::utils::get_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    uint64_t slotIndex = std::stoull(ss.str());

    return static_cast<int>(slotIndex);
}

void jaime::utils::spin_wait_on(const std::atomic_bool &toWait, bool value) {
    while (toWait.load(std::memory_order_acquire) != value){
        std::this_thread::yield();
    }
}
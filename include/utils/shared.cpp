#include "utils.hpp"

int jaime::utils::get_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    uint64_t slotIndex = std::stoull(ss.str());

    return static_cast<int>(slotIndex);
}
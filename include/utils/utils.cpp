#include "utils.hpp"

int jaime::utils::get_thread_id() {
    std::thread::id my_id = std::this_thread::get_id();
    return static_cast<int>(*((uint8_t *) &my_id));
}
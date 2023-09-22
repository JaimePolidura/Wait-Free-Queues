#pragma once

#include <numeric>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <stdint.h>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <exception>
#include <cstdio>
#include <tgmath.h>
#include <optional>
#include <filesystem>
#include <math.h>
#include <regex>
#include <cstring>

#include <thread>
#include <mutex>
#include <future>
#include <atomic>
#include <condition_variable>

#include <vector>
#include <queue>
#include <map>
#include <unordered_set>
#include <set>
#include <list>

typedef uint64_t epoch_t;
typedef uint64_t timestamp_t;

int get_thread_id() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    uint64_t slotIndex = std::stoull(ss.str());

    return static_cast<int>(slotIndex);
}

template<typename T>
T increment_and_get(std::atomic<T>& atomic) {
    T last;

    do {
        last = atomic.load();
    }while(!atomic.compare_exchange_weak(last, last + 1));

    return ++last;
}
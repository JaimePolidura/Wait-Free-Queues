#pragma once

#include "utils/lock_free_queue.hpp"
#include "utils/blocking_queue.hpp"
#include "utils/shared.hpp"
#include "spsc_queue.hpp"
#include "bench_shared.h"

using namespace std::chrono;

Result spsc_blocking_bench(int n_times);
Result spsc_spsc_bench(int n_times);

void start_spsc_bench() {
    print_result(spsc_blocking_bench(1000000));
    print_result(spsc_spsc_bench(1000000));
}

Result spsc_blocking_bench(int n_times) {
    blocking_queue<int> * queue = new blocking_queue<int>();

    return queue_bench("SPSC", "Blocking", n_times, 1,
                       [queue](auto i){queue->push(i);},
                       [queue](){queue->pop();});
}

Result spsc_spsc_bench(int n_times) {
    jaime::spsc_queue<int> * queue = new jaime::spsc_queue<int>();

    return queue_bench("SPSC", "SPSC", n_times, 1,
                       [queue](auto i){queue->enqueue(i);},
                       [queue](){queue->dequeue();});
}
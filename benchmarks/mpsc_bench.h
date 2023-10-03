#pragma once

#include "utils/lock_free_queue.hpp"
#include "utils/blocking_queue.hpp"
#include "utils/shared.hpp"
#include "unordered_mpsc_queue.hpp"
#include "ordered_mpsc_queue.hpp"
#include "bench_shared.h"

using namespace std::chrono;

Result mpsc_blocking_bench(int n_times, int n_producers);
Result mpsc_ordered_bench(int n_times, int n_producers);
Result mpsc_unordered_bench(int n_times, int n_producers);

void start_mpsc_bench() {
    print_result(mpsc_blocking_bench(1000000, 8));
    print_result(mpsc_ordered_bench(1000000, 8));
    print_result(mpsc_unordered_bench(1000000, 8));
}

Result mpsc_blocking_bench(int n_times, int n_producers) {
    blocking_queue<int> * blocking_queue_to_test = new blocking_queue<int>();

    return queue_bench("MPSC", "Blocking", n_times, n_producers,
                       [blocking_queue_to_test](auto i){blocking_queue_to_test->push(i);},
                       [blocking_queue_to_test](){blocking_queue_to_test->pop();});
}

Result mpsc_ordered_bench(int n_times, int n_producers) {
    jaime::lock_free::ordered_mpsc_queue<int> * queue = new jaime::lock_free::ordered_mpsc_queue<int>(n_producers);

    return queue_bench("MPSC", "Ordered MPSC", n_times, n_producers,
                       [queue](auto i){queue->enqueue(i);},
                       [queue](){queue->dequeue();});
}

Result mpsc_unordered_bench(int n_times, int n_producers) {
    jaime::unordered_mpsc_queue<int> * queue = new jaime::unordered_mpsc_queue<int>(n_producers);

    return queue_bench("MPSC", "Unordered MPSC", n_times, n_producers,
                       [queue](auto i){queue->enqueue(i);},
                       [queue](){queue->dequeue();});
}
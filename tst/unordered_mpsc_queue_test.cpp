#include <gtest/gtest.h>
#include "utils/shared.hpp"
#include "unordered_mpsc_queue.hpp"

TEST(unordered_mpsc_queue, enqueue_dequeue_multiple_threads) {
    std::shared_ptr<jaime::unordered_mpsc_queue<int>> queue = std::make_shared<jaime::unordered_mpsc_queue<int>>(3);
    int n_times = 100000;

    std::thread producer_1 = std::thread([queue, n_times](){
        for (int i = 0; i < n_times; ++i) {
            queue->enqueue(i);
        }
    });
    std::thread producer_2 = std::thread([queue, n_times](){
        for (int i = 0; i < n_times; ++i) {
            queue->enqueue(i);
        }
    });
    std::thread producer_3 = std::thread([queue, n_times](){
        for (int i = 0; i < n_times; ++i) {
            queue->enqueue(i);
        }
    });

    std::thread consumer = std::thread([queue, n_times](){
        for(int i = 0; i < n_times * 3; i++){
            queue->dequeue();
        }
    });

    producer_1.join();
    producer_2.join();
    producer_3.join();
    consumer.join();
}

TEST(unordered_mpsc_queue, enqueue_dequeue_single_thread) {
    jaime::unordered_mpsc_queue<int> queue = jaime::unordered_mpsc_queue<int>(1);
    queue.enqueue(1);
    queue.enqueue(2);

    std::optional<int> dequeue_1 = queue.dequeue();
    ASSERT_TRUE(dequeue_1.has_value() && dequeue_1.value() == 1);

    std::optional<int> dequeue_2 = queue.dequeue();
    ASSERT_TRUE(dequeue_2.has_value() && dequeue_2.value() == 2);

    std::optional<int> dequeue_3 = queue.dequeue();
    ASSERT_TRUE(!dequeue_3.has_value());
}
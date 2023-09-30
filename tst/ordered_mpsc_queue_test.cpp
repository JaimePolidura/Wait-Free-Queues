#include <gtest/gtest.h>
#include "utils/utils.hpp"
#include "ordered_mpsc_queue.hpp"

TEST(ordered_mpsc_queue, enqueue_dequeue_multiple_thread) {
    jaime::lock_free::ordered_mpsc_queue<int> * queue = new jaime::lock_free::ordered_mpsc_queue<int>(3);
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

TEST(ordered_mpsc_queue, enqueue_dequeue_single_thread) {
    jaime::lock_free::ordered_mpsc_queue<int> queue = jaime::lock_free::ordered_mpsc_queue<int>(1);
    queue.enqueue(1);
    queue.enqueue(2);

    std::optional<int> dequeue_1 = queue.dequeue();
    ASSERT_TRUE(dequeue_1.has_value() && dequeue_1.value() == 1);

    std::optional<int> dequeue_2 = queue.dequeue();
    ASSERT_TRUE(dequeue_2.has_value() && dequeue_2.value() == 2);

    std::optional<int> dequeue_3 = queue.dequeue();
    ASSERT_TRUE(!dequeue_3.has_value());
}
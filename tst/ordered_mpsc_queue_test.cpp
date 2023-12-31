#include <gtest/gtest.h>
#include "utils/shared.hpp"
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

TEST(ordered_mpsc_queue, dequeue_all) {
    jaime::lock_free::ordered_mpsc_queue<int> queue = jaime::lock_free::ordered_mpsc_queue<int>(4);
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);
    queue.enqueue(4);

    std::vector<int> dequeued = queue.dequeue_all();

    ASSERT_EQ(dequeued.size(), 4);
    ASSERT_EQ(dequeued[0], 1);
    ASSERT_EQ(dequeued[1], 2);
    ASSERT_EQ(dequeued[2], 3);
    ASSERT_EQ(dequeued[3], 4);
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
#include <gtest/gtest.h>
#include "spsc_queue.hpp"

TEST(spsc_queue, multithreaded_no_race_condition) {
    jaime::spsc_queue<int> * queue = new jaime::spsc_queue<int>();
    int nEnqueues = 1000000;
    bool * raceConditionFound = new bool(false);
    *raceConditionFound = false;

    std::thread producer = std::thread{[queue, nEnqueues](){
        for(int i = 0; i < nEnqueues; i++){
            queue->enqueue(i);
        }
    }};

    std::thread consumer = std::thread{[queue, nEnqueues, raceConditionFound](){
        int prevValue = -1;

        for(int i = 0; i < nEnqueues; i++){
            std::optional<int> dequeued = queue->dequeue();

            if(!dequeued.has_value()) {
                std::this_thread::yield();
                continue;
            }

            int dequeuedValue = dequeued.value();

            if(prevValue + 1 != dequeuedValue){
                *raceConditionFound = true;
                break;
            } else {
                prevValue = dequeuedValue;
            }
        }
    }};

    producer.join();
    consumer.join();

    ASSERT_FALSE(*raceConditionFound);
}

TEST(spsc_queue, one_enqueue) {
    jaime::spsc_queue<int> queue{};
    queue.enqueue(1);

    auto dequeued = queue.dequeue();

    ASSERT_TRUE(dequeued.has_value());
    ASSERT_EQ(dequeued.value(), 1);
}

TEST(spsc_queue, two_enqueues) {
    jaime::spsc_queue<int> queue{};
    queue.enqueue(1);
    queue.enqueue(2);

    auto dequeued1 = queue.dequeue();
    auto dequeued2 = queue.dequeue();
    auto dequeued3 = queue.dequeue();

    ASSERT_TRUE(dequeued1.has_value());
    ASSERT_TRUE(dequeued2.has_value());
    ASSERT_FALSE(dequeued3.has_value());

    ASSERT_EQ(dequeued1.value(), 1);
    ASSERT_EQ(dequeued2.value(), 2);
}

 TEST(spsc_queue, two_dequeues_on_empty) {
    jaime::spsc_queue<int> queue{};

    ASSERT_FALSE(queue.dequeue().has_value());
    ASSERT_FALSE(queue.dequeue().has_value());
}
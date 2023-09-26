#include <gtest/gtest.h>
#include "spsc_heap_object_pool.h"

TEST(spsc_object_pool, ThreadedMultiplePutMultipleTake) {
    jaime::spsc_heap_object_pool<int> * pool = new jaime::spsc_heap_object_pool<int>();
    int nTimes = 100000;
    bool * raceCondition = new bool(false);
    int * raceConditionLastValue = new int(0);
    int * raceConditionValue = new int(0);

    std::thread producer = std::thread([pool, nTimes](){
        for(int i = 0; i < nTimes; i++){
            pool->put(new int(i));
        }
    });

    std::thread consumer = std::thread([pool, nTimes, raceCondition, raceConditionLastValue, raceConditionValue](){
        auto lastValue = 0;

        for(int i = 0; i < nTimes; i++){
            auto value = *pool->takeOrCreateWithArgs(-1);

            bool valueLastValueNotMinusOne = value != -1 && lastValue != -1;

            if(valueLastValueNotMinusOne && (lastValue + 1 != value)) {
                *raceCondition = true;
                *raceConditionValue = value;
                *raceConditionLastValue = lastValue;
                break;
            }

            lastValue = value;
        }
    });

    producer.join();
    consumer.join();

    if(*raceCondition){
        std::cout << "porro" << std::endl;
        nTimes++;
    }

    ASSERT_FALSE(*raceCondition);
}

TEST(spsc_object_pool, SingleThreadedMultiplePutMultipleTake) {
    jaime::spsc_heap_object_pool<int> pool = jaime::spsc_heap_object_pool<int>();
    pool.put(new int(1));
    pool.put(new int(2));

    ASSERT_EQ(*pool.take(), 1);
    ASSERT_EQ(*pool.take(), 2);
    pool.put(new int(3));

    ASSERT_EQ(*pool.take(), 3);
    ASSERT_EQ(*pool.take(), 0);
    ASSERT_EQ(*pool.takeOrCreateWithArgs(10), 10);
}

TEST(spsc_object_pool, Put) {
    jaime::spsc_heap_object_pool<int> pool = jaime::spsc_heap_object_pool<int>();
    pool.put(new int(1));
    pool.put(new int(2));

    ASSERT_EQ(*pool.take(), 1);
    ASSERT_EQ(*pool.take(), 2);
    ASSERT_EQ(*pool.take(), 0);
}
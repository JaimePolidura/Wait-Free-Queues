#include <gtest/gtest.h>
#include "spsc_heap_object_pool.h"

TEST(spsc_object_pool, MultiplePutMultipleTake) {
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
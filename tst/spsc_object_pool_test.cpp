#include <gtest/gtest.h>
#include "spsc_object_pool.h"

TEST(spsc_object_pool, MultiplePutMultipleTake) {
    jaime::spsc_object_pool<int> pool = jaime::spsc_object_pool<int>([](){return -1;});
    pool.put(1);
    pool.put(2);

    ASSERT_EQ(pool.take(), 1);
    ASSERT_EQ(pool.take(), 2);
    pool.put(3);

    ASSERT_EQ(pool.take(), 3);
    ASSERT_EQ(pool.take(), -1);
}

TEST(spsc_object_pool, Put) {
    jaime::spsc_object_pool<int> pool = jaime::spsc_object_pool<int>([](){return -1;});
    pool.put(1);
    pool.put(2);

    ASSERT_EQ(pool.take(), 1);
    ASSERT_EQ(pool.take(), 2);
    ASSERT_EQ(pool.take(), -1);
}

TEST(spsc_object_pool, Take) {
    jaime::spsc_object_pool<int> pool = jaime::spsc_object_pool<int>([](){return 1;});

    ASSERT_EQ(pool.take(), 1);
}
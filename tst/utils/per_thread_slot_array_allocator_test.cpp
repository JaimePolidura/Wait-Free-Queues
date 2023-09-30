#include <gtest/gtest.h>
#include "utils/utils.hpp"
#include "utils/per_thread_slot_array_allocator.h"

TEST(per_thread_slot_array_allocator, no_race_condition) {
    std::shared_ptr<jaime::utils::per_thread_slot_array_allocator> allocator = std::make_shared<jaime::utils::per_thread_slot_array_allocator>(5);

    std::thread thread_1 = std::thread([allocator](){allocator->allocate(jaime::utils::get_thread_id());});
    std::thread thread_2 = std::thread([allocator](){allocator->allocate(jaime::utils::get_thread_id());});
    std::thread thread_3 = std::thread([allocator](){allocator->allocate(jaime::utils::get_thread_id());});
    std::thread thread_4 = std::thread([allocator](){allocator->allocate(jaime::utils::get_thread_id());});
    std::thread thread_5 = std::thread([allocator](){allocator->allocate(jaime::utils::get_thread_id());});

    thread_1.join();
    thread_2.join();
    thread_3.join();
    thread_4.join();
    thread_5.join();

    //Will loop the whole array
    ASSERT_FALSE(allocator->allocate(100).success);
}

TEST(per_thread_slot_array_allocator, get_slot_owned_by) {
    jaime::utils::per_thread_slot_array_allocator allocator{2};
    jaime::utils::allocation_result result1 = allocator.allocate(0);
    jaime::utils::allocation_result result2 = allocator.allocate(1);

    ASSERT_EQ(result1.slot, allocator.get_slot_owned_by(0));
    ASSERT_EQ(result2.slot, allocator.get_slot_owned_by(1));
}

TEST(per_thread_slot_array_allocator, shouldent_allocate_small_space) {
    jaime::utils::per_thread_slot_array_allocator allocator{1};
    jaime::utils::allocation_result result = allocator.allocate(0);

    ASSERT_TRUE(result.success);
    ASSERT_EQ(result.slot, 0);
}

TEST(per_thread_slot_array_allocator, should_deallocate) {
    jaime::utils::per_thread_slot_array_allocator allocator{1};
    allocator.allocate(1);
    allocator.deallocate(1);

    jaime::utils::allocation_result result2 = allocator.allocate(1);

    ASSERT_TRUE(result2.success);
    ASSERT_EQ(result2.slot, 0);
}

TEST(per_thread_slot_array_allocator, should_allocate) {
    jaime::utils::per_thread_slot_array_allocator allocator{2};
    jaime::utils::allocation_result result1 = allocator.allocate(0);
    jaime::utils::allocation_result result2 = allocator.allocate(1);
    jaime::utils::allocation_result result3 = allocator.allocate(2);

    ASSERT_TRUE(result1.success);
    ASSERT_TRUE(result2.success);
    ASSERT_TRUE(result1.slot != result2.slot);

    ASSERT_FALSE(result3.success);
    ASSERT_EQ(result3.slot, -1);
}
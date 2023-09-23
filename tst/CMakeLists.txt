cmake_minimum_required(VERSION 3.26)

add_test(NAME tst COMMAND tst)

include_directories(${CMAKE_PREFIX_PATH}/include)

add_executable(tst main.cpp spsc_queue_test.cpp)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(tst PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main wait-free-queues)

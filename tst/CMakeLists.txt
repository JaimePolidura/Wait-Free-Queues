cmake_minimum_required(VERSION 3.26)

add_executable(tst main.cpp
        SingleProducerSingleConsumerTest.cpp)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(tst PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main wait-free-queue)
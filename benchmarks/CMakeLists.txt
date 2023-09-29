cmake_minimum_required(VERSION 3.26)

add_executable(examples main.cpp
        datastructures/blocking_queue.hpp
        datastructures/lock_free_queue.hpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(examples PRIVATE wait-free-queues)
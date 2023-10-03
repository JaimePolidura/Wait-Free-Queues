option(TESTING OFF)
option(BENCHMARKS OFF)

cmake_minimum_required(VERSION 3.23)
project(wait-free-queues)

set(CMAKE_CXX_STANDARD 23)

add_library(${CMAKE_PROJECT_NAME}
        wait-free-queues/spsc_queue.hpp
        wait-free-queues/utils/shared.hpp
        wait-free-queues/ordered_mpsc_queue.hpp
        wait-free-queues/unordered_mpsc_queue.hpp
        wait-free-queues/main.cpp
        wait-free-queues/spsc_heap_object_pool.h
        wait-free-queues/utils/spin_lock.hpp
        wait-free-queues/utils/spin_lock.hpp
        wait-free-queues/utils/per_thread_slot_array_allocator.h
        wait-free-queues/mpsc_queue.h
        wait-free-queues/wait-free-queues.h
)

include_directories(wait-free-queues)

if(BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

if(TESTING)
    add_subdirectory(tst)
endif()
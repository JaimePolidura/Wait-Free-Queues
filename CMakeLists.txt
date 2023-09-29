option(TESTING ON)

cmake_minimum_required(VERSION 3.26)
project(wait-free-queues)

set(CMAKE_CXX_STANDARD 23)

add_library(${CMAKE_PROJECT_NAME}
        include/spsc_queue.hpp
        include/utils/utils.hpp
        include/ordered_mpsc_queue.hpp
        include/unordered_mpsc_queue.hpp
        include/main.cpp
        benchmarks/main.cpp
        include/spsc_heap_object_pool.h
        include/utils/utils.cpp
        include/utils/spin_lock.hpp
        include/utils/spin_lock.hpp
        include/utils/per_thread_slot_array_allocator.h
)

include_directories(include)

add_subdirectory(benchmarks)

if(TESTING)
    add_subdirectory(tst)
endif()
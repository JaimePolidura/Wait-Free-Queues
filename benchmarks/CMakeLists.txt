cmake_minimum_required(VERSION 3.26)

add_executable(benchmarks main.cpp
        datastructures/blocking_queue.hpp
        datastructures/lock_free_queue.hpp
        spsc_bench.h
        mpsc_bench.h
        bench_shared.h)

include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(benchmarks PRIVATE wait-free-queues)
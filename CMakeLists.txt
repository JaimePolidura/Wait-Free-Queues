option(TESTING ON)

cmake_minimum_required(VERSION 3.26)
project(wait-free-queues)

set(CMAKE_CXX_STANDARD 23)

add_library(${CMAKE_PROJECT_NAME}
        include/spsc_queue.hpp
        include/shared.hpp
        include/ordered_mpsc_queue.hpp
        include/unordered_mpsc_queue.hpp
        include/main.cpp
        examples/main.cpp
        include/spsc_object_pool.h
        include/shared.cpp
)

include_directories(include)

add_subdirectory(examples)

if(TESTING)
    add_subdirectory(tst)
endif()
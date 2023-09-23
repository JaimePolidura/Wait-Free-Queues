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
)

include_directories(include)

if(TESTING)
    add_subdirectory(tst)
endif()
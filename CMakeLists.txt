option(TESTING ON)

cmake_minimum_required(VERSION 3.26)
project(wait-free-queue)

set(CMAKE_CXX_STANDARD 23)

add_library(${CMAKE_PROJECT_NAME}
        include/spsc_queue.hpp
        include/shared.h
        include/ordered_mpsc_queue.hpp
        include/unordered_mpsc_queue.hpp
        tst/main.cpp
)

if(TESTING)
    enable_testing()
    add_subdirectory(tst)
endif()

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

if(TESTING)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
    set(VCPKG_ROOT "C:/vcpkg")
    set(VCPKG_TARGET_TRIPLET "x64-windows")
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")

    enable_testing()
    add_subdirectory(tst)
endif()
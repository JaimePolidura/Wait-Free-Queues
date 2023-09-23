function(setup_vcpkg)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

    if(UNIX)
        set(VCPKG_ROOT "/vcpkg")
        set(VCPKG_TARGET_TRIPLET "x64-linux")
        set(CMAKE_TOOLCHAIN_FILE "/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-linux")
    else()
        set(VCPKG_ROOT "C:/vcpkg")
        set(VCPKG_TARGET_TRIPLET "x64-windows")
        set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-windows")
    endif ()
endfunction()

# START FROM HERE

cmake_minimum_required(VERSION 3.26)

enable_testing()

setup_vcpkg()

include_directories(${CMAKE_PREFIX_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(tst main.cpp spsc_queue_test.cpp)

find_package(GTest CONFIG REQUIRED)
target_link_libraries(tst PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main wait-free-queues)

add_test(NAME tst COMMAND tst)
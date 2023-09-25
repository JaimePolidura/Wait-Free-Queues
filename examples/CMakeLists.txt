cmake_minimum_required(VERSION 3.26)

add_executable(examples main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)

target_link_libraries(examples PRIVATE wait-free-queues)
cmake_minimum_required(VERSION 3.26)
project(wait-free-queue)

set(CMAKE_CXX_STANDARD 23)

add_executable(${CMAKE_PROJECT_NAME} main.cpp
        include/SingleProducerSingleConsumer.hpp
        include/shared.h
        include/OrderedMultipleProducerSingleConsumer.hpp
        include/UnorderedMultipleProducerSingleConsumer.hpp)

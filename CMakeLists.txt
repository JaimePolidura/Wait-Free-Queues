cmake_minimum_required(VERSION 3.26)
project(wait-free-queue)

set(CMAKE_CXX_STANDARD 23)

add_library(wait-free-queue
        include/SingleProducerSingleConsumer.hpp
        include/shared.h
        include/OrderedMultipleProducerSingleConsumer.hpp
        include/UnorderedMultipleProducerSingleConsumer.hpp
        tst/main.cpp
)
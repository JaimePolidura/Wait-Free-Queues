#include <iostream>

#include "utils/utils.hpp"

#include "include/spsc_queue.hpp"
#include "include/ordered_mpsc_queue.hpp"
#include "include/unordered_mpsc_queue.hpp"

void singleWriterSingleReader();
void orderedMultiWriterSingleReader();
void unorderedMultiWriterSingleReader();
void singleReader();

int main() {
//    unorderedMultiWriterSingleReader();
//    orderedMultiWriterSingleReader();
    singleWriterSingleReader();
//    singleReader();

    return 0;
}

struct Value {
    int value;
    int enqueued_by;
};

void unorderedMultiWriterSingleReader() {
    auto queue = new UnorderedMultipleProducerSingleConsumer<Value, 5>();

    std::thread writer1 = std::thread([queue]{
        for(int i = 0; i < 1000000; i++) {
            queue->enqueue(Value{.value = i, .enqueued_by = 1});
        }
    });

    std::thread writer2 = std::thread([queue]{
        for(int i = 0; i < 1000000; i++) {
            queue->enqueue(Value{.value = i, .enqueued_by = 2});
        }
    });

    std::thread writer3 = std::thread([queue]{
        for(int i = 0; i < 1000000; i++) {
            queue->enqueue(Value{.value = i, .enqueued_by = 3});
        }
    });

    std::thread reader = std::thread([queue]{
        for(int i = 0; i < 1000000; i++){
            auto dequeued = queue->dequeue();

            if(!dequeued.has_value()){
                std::this_thread::yield();
            }else{
                std::cout << "[READER] Dequeued from " << dequeued.value().enqueued_by << " of value: " << dequeued.value().value << std::endl;
            }
        }
    });

    writer1.join();
    writer2.join();
    writer3.join();
    reader.join();
}

void orderedMultiWriterSingleReader() {
    auto queue = new OrderedMultipleProducerSingleConsumer<Value, 5>();

    std::thread writer1 = std::thread([queue]{
        for(int i = 0; i < 1000000; i++) {
            queue->enqueue(Value{.value = i, .enqueued_by = 1});
        }
    });

    std::thread writer2 = std::thread([queue]{
        for(int i = 0; i < 1000000; i++) {
            queue->enqueue(Value{.value = i, .enqueued_by = 2});
        }
    });

    std::thread writer3 = std::thread([queue]{
        for(int i = 0; i < 1000000; i++) {
            queue->enqueue(Value{.value = i, .enqueued_by = 3});
        }
    });

    std::thread reader = std::thread([queue]{
        for(int i = 0; i < 1000000; i++){
            auto dequeued = queue->dequeue();

            if(!dequeued.has_value()){
                std::this_thread::yield();
            }else{
                std::cout << "[READER] Dequeued from " << dequeued.value().enqueued_by << " of value: " << dequeued.value().value << std::endl;
            }
        }
    });

    writer1.join();
    writer2.join();
    writer3.join();
    reader.join();
}

void singleWriterSingleReader() {
    SingleProducerSingleConsumer<int> * queue = new SingleProducerSingleConsumer<int>();

    std::thread writer = std::thread([queue]{
        for(int i = 0; i < 1000000; i++) {
            queue->enqueue(i, i);
        }
    });

    std::thread reader = std::thread([queue]{
        for(int i = 0; i < 1000000; i++){
            auto dequeued = queue->dequeue();

            if(!dequeued.has_value()){
                std::this_thread::yield();
            }else{
                std::cout << "[READER] Dequeued value " << dequeued.value() << std::endl;
            }
        }
    });

    reader.join();
    writer.join();
}

void singleReader() {
    auto queue = new OrderedMultipleProducerSingleConsumer<int, 5>();
    queue->dequeue();
}
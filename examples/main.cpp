#include "unordered_mpsc_queue.hpp"
#include "ordered_mpsc_queue.hpp"
#include "spsc_heap_object_pool.h"
#include "spsc_queue.hpp"

void singleWriterSingleReader();
void orderedMultiWriterSingleReader();
void unorderedMultiWriterSingleReader();
void objectPool();

int main() {
//    unorderedMultiWriterSingleReader();
//    orderedMultiWriterSingleReader();
//    singleWriterSingleReader();

    objectPool();

    return 0;
}

struct Value {
    int value;
    int enqueued_by;
};

void unorderedMultiWriterSingleReader() {
    auto queue = new jaime::unordered_mpsc_queue<Value, 5>();

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
    auto queue = new jaime::lock_free::ordered_mpsc_queue<Value, 5>();

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
    auto * queue = new jaime::spsc_queue<int>();

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
//                std::cout << "[READER] Dequeued value " << dequeued.value() << std::endl;
            }
        }
    });

    reader.join();
    writer.join();
}

void objectPool() {
    jaime::spsc_heap_object_pool<int> * pool = new jaime::spsc_heap_object_pool<int>();
    pool->populate(10);

    std::thread producer = std::thread([pool](){
        for(int i = 0; i < 100; i++){
            pool->put(new int(i));
        }
    });

    std::thread consumer = std::thread([pool](){
        for(int i = 0; i < 100; i++){
            int * ptr = pool->take();
            std::cout << *ptr << std::endl;
            pool->put(ptr);
        }
    });

    producer.join();
    consumer.join();
}
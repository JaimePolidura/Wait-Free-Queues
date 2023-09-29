#include "datastructures/lock_free_queue.hpp"
#include "datastructures/blocking_queue.hpp"
#include "utils/utils.hpp"
#include "spsc_queue.hpp"

using namespace std::chrono;

struct Result {
    std::string queueType;
    int n_times;

    uint64_t consumers;
    uint64_t producer;
};

Result blocking_queue_bench(int n_times);
Result lock_free_queue_bench(int n_times);
Result spsc_queue_bench(int n_times);

uint64_t time_now();
void print_result(const Result& result);

int main() {
    print_result(blocking_queue_bench(1000000));
    print_result(spsc_queue_bench(1000000));

    return 0;
}

Result lock_free_queue_bench(int n_times) {
    lock_free_queue<int> * queue = new lock_free_queue<int>();
    uint64_t * consumers_time = new uint64_t();
    uint64_t * producers_time = new uint64_t();

    std::thread producer = std::thread{[queue, n_times, producers_time](){
        uint64_t start = time_now();
        for(int i = 0; i < n_times; i++){
            queue->enqueue(i);
        }
        *producers_time = time_now() - start;
    }};

    std::thread consumer = std::thread{[queue, n_times, consumers_time](){
        uint64_t start = time_now();
        for(int i = 0; i < n_times; i++){
            queue->dequeue();
        }
        *consumers_time = time_now() - start;
    }};

    producer.join();
    consumer.join();

    return Result{
            .queueType = "LockFree",
            .n_times = n_times,
            .consumers = *consumers_time,
            .producer = *producers_time,
    };
}

Result blocking_queue_bench(int n_times) {
    blocking_queue<int> * queue = new blocking_queue<int>();
    uint64_t * consumers_time = new uint64_t();
    uint64_t * producers_time = new uint64_t();

    std::thread producer = std::thread{[queue, n_times, producers_time](){
        uint64_t start = time_now();
        for(int i = 0; i < n_times; i++){
            queue->push(i);
        }
        *producers_time = time_now() - start;
    }};

    std::thread consumer = std::thread{[queue, n_times, consumers_time](){
        uint64_t start = time_now();
        for(int i = 0; i < n_times; i++){
            queue->pop();
        }
        *consumers_time = time_now() - start;
    }};

    producer.join();
    consumer.join();

    return Result{
        .queueType = "Blocking",
        .n_times = n_times,
        .consumers = *consumers_time,
        .producer = *producers_time,
    };
}

Result spsc_queue_bench(int n_times) {
    jaime::spsc_queue<int> * queue = new jaime::spsc_queue<int>();
    uint64_t * consumers_time = new uint64_t();
    uint64_t * producers_time = new uint64_t();

    std::thread producer = std::thread{[queue, n_times, producers_time](){
        uint64_t start = time_now();
        for(int i = 0; i < n_times; i++){
            queue->enqueue(i);
        }
        *producers_time = time_now() - start;
    }};

    std::thread consumer = std::thread{[queue, n_times, consumers_time](){
        uint64_t start = time_now();
        for(int i = 0; i < n_times; i++){
            queue->dequeue();
        }
        *consumers_time = time_now() - start;
    }};

    producer.join();
    consumer.join();

    return Result{
            .queueType = "SPSC",
            .n_times = n_times,
            .consumers = *consumers_time,
            .producer = *producers_time,
    };
}

uint64_t time_now() {
    system_clock::time_point currentTimePoint = system_clock::now();
    milliseconds currentTimeMillis = duration_cast<milliseconds>(currentTimePoint.time_since_epoch());

    return static_cast<uint64_t>(currentTimeMillis.count());
}

void print_result(const Result& result) {
    std::cout << "[" << result.queueType << " " << result.n_times << "] Producers: " << result.producer << "ms" << std::endl;
    std::cout << "[" << result.queueType << " " << result.n_times << "] Consumers: " << result.consumers << "ms" << std::endl;
}
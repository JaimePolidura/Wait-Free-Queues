#pragma once

#include "utils/utils.hpp"

using namespace std::chrono;

struct Result {
    std::string queue_type;
    std::string bench_type;
    int n_times;

    uint64_t consumers;
    std::vector<uint64_t> * producers;

    uint64_t avg_producers() const {
        auto val = 0;
        for (uint64_t time: *producers) {
            val += time;
        }

        return val / producers->size();
    }
};

uint64_t time_now();

Result queue_bench(const std::string& benchType,
                   const std::string& queueType,
                   int n_times,
                   int n_producers,
                   const std::function<void(int)>& enqueue,
                   const std::function<void()>& dequeue) {
    uint64_t * consumers_time = new uint64_t();
    std::vector<std::thread> producers_thread{};
    std::vector<uint64_t> * producers_times = new std::vector<uint64_t>(n_producers);

    std::thread consumer = std::thread{[n_times, consumers_time, dequeue](){
        uint64_t start = time_now();
        for(int i = 0; i < n_times; i++){
            dequeue();
        }
        *consumers_time = time_now() - start;
    }};

    for (int i = 0; i < n_producers; ++i) {
        producers_thread.emplace_back([n_times, producers_times, enqueue, threadId = i](){
            uint64_t start = time_now();
            for(int i = 0; i < n_times; i++){
                enqueue(i);
            }

            *(producers_times->data() + threadId) = time_now() - start;
        });
    }

    std::for_each(producers_thread.begin(), producers_thread.end(), [](std::thread& thread){
        thread.join();
    });
    
    consumer.join();

    return Result{
            .queue_type = queueType,
            .bench_type = benchType,
            .n_times = n_times,
            .consumers = *consumers_time,
            .producers = producers_times
    };

}

uint64_t time_now() {
    system_clock::time_point currentTimePoint = system_clock::now();
    milliseconds currentTimeMillis = duration_cast<milliseconds>(currentTimePoint.time_since_epoch());

    return static_cast<uint64_t>(currentTimeMillis.count());
}

void print_result(const Result& result) {
    std::cout << "[" << result.bench_type << " " << result.queue_type << " " << result.n_times << "] Producers ("<< result.producers->size() <<"): " << result.avg_producers() << "ms" << std::endl;
    std::cout << "[" << result.bench_type << " " << result.queue_type << " " << result.n_times << "] Consumers: " << result.consumers << "ms" << std::endl;
}
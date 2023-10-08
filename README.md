# Wait-Free-Queues
C++ Wait-Free implementation of multiple-producer single-consumer &amp; single-producer single-consumer

## Setup CMAKE
```shell
mkdir libs 
cd libs
git clone https://github.com/JaimePolidura/Wait-Free-Queues
```

```cmake
project(your-project-name)

add_subdirectory(libs/Wait-Free-Queues)
include_directories(libs/Wait-Free-Queues)

add_executable(your-project-name main.cpp)

target_link_libraries(your-project-name PRIVATE wait-free-queues)
```

## Single-Producer Single-Consumer
```c++
#include "wait-free-queues/wait-free-queues.h"

jaime::spsc_queue<int> my_queue{};

my_queue.enqueue();

std::optional<int> optional1 = my_queue.dequeue(); //Will return 1
std::optional<int> optional2 = my_queue.dequeue(); //Will be empty
```

## Single-Producer Multiple-Consumer

### Unordered
- jaime::unordered_mpsc_queue contains an array of jaime::spsc_queue. Every producer thread has assigned an index.
- FIFO enqueue order is not guaranteed. When a consumer calls dequeue(), it will iterate through the array until it finds a spsc_queue that returns an element, otherwise it will return an empty optional.
```c++
#include "wait-free-queues/wait-free-queues.h"

//Number of producer threads that will access the queue. It is recommended that it is a power of 2
jaime::unordered_mpsc_queue<int> * queue = new jaime::unordered_mpsc_queue<int>(2);

std::thread producer1 = std::thread{[queue](){
    queue->enqueue(1);
}};

std::thread producer2 = std::thread{[queue](){
    std::this_thread::sleep_for(std::chrono::seconds(5));
    queue->enqueue(2);
}};

join(producer1, producer2);

//Consumer
//FIFO enqueue order not guaranteed. The order could be 2, 1 or 1, 2
queue->dequeue();
queue->dequeue();
```

### Ordered
- Similiar to [Single-Producer Multiple-Consumer](#Unordered)
- FIFO enqueue order is guaranteed. The queue will have an internal counter that is atomically incremented when a producer enqueues an item. So as a result, every enqueued element will have a unique,
  monotonically counter.
- When the consumer dequeues an element it will save it's counter. So, the next time it wants to dequeue another element, the element's counter needs to be the same as the consumer's counter + 1. 
- Dequeue() is O(n) where n is the number of producers threads.
```c++
#include "wait-free-queues/wait-free-queues.h"

jaime::lock_free::ordered_mpsc_queue<int> * queue = new jaime::lock_free::ordered_mpsc_queue<int>(2);

std::thread producer1 = std::thread{[queue](){
    queue->enqueue(1);
}};
std::thread producer2 = std::thread{[queue](){
    std::this_thread::sleep_for(std::chrono::seconds(5));
    queue->enqueue(2);
}};

join(producer1, producer2);

//Consumer
//FIFO enqueue order is guaranteed. The order will be 1, 2 
queue->dequeue();
queue->dequeue();
```
#pragma once

#include "spsc_heap_object_pool.h"
#include "utils/utils.hpp"

namespace jaime {

template<typename T>
class node {
public:
    using node_ptr_t = node<T> *;
    using atomic_node_ptr_t = std::atomic<node_ptr_t>;

    T value;
    atomic_node_ptr_t prev;
    timestamp_t timestamp;

    epoch_t epoch;

    node(): timestamp(0), epoch(0) {}

    explicit node(T value, timestamp_t timestamp):
            epoch(0),
            value(value),
            prev(nullptr),
            timestamp(timestamp) {}

    explicit node(timestamp_t timestamp):
            epoch(0),
            value(value),
            prev(nullptr),
            timestamp(timestamp) {}
};

template<typename T>
class spsc_queue {
private:
    using node_ptr_t = node<T> *;
    using atomic_node_prev_field_ptr = std::atomic<std::atomic<node_ptr_t> *>;

    atomic_node_prev_field_ptr head;

    jaime::spsc_heap_object_pool<node<T>> node_pool;

    std::atomic<epoch_t> producer_epoch;
    std::atomic<epoch_t> producer_last_head_epoch;

    std::queue<node_ptr_t > release_pool;

public:
    spsc_queue() {
        node_ptr_t sentinel = new node<T>(0);
        this->producer_last_head_epoch = 0;
        this->node_pool.populate(100);
        this->head = &sentinel->prev;
        this->producer_epoch = 0;
    }

    void enqueue(const T& value, const timestamp_t timestamp = 0) {
        std::atomic<node_ptr_t> * headPrevFieldNodePtr = this->head.load(std::memory_order_acquire);
        node_ptr_t headNode = headPrevFieldNodePtr->load(std::memory_order_acquire);
        node_ptr_t newNode = this->createNode(value, timestamp);
        node_ptr_t sentinel = this->getNodePtrFromPrevNodeField(headPrevFieldNodePtr);

        this->producer_last_head_epoch.store(sentinel->epoch, std::memory_order_release);

        if(headNode == nullptr){
            sentinel->prev.store(newNode, std::memory_order_release);
            return;
        }

        while(headNode->prev.load(std::memory_order_acquire) != nullptr){
            headNode = headNode->prev.load(std::memory_order_acquire);
        }

        headNode->prev.store(newNode, std::memory_order_release);
    }

    std::optional<T> dequeue(const timestamp_t timestampExpectedToDequeue = 0) {
        std::atomic<node_ptr_t> * headPrevFieldNodePtr = this->head.load(std::memory_order_acquire);
        node_ptr_t headNode = headPrevFieldNodePtr->load(std::memory_order_acquire);

        if(headNode != nullptr &&
           (timestampExpectedToDequeue == 0 ||
            headNode->timestamp == timestampExpectedToDequeue)){

            T value = headNode->value;

            this->head.store(&headNode->prev, std::memory_order_release);

            this->release_pool.push(headNode);

            if(release_pool.size() > 10){
                epoch_t last_head_epoch = this->producer_last_head_epoch.load(std::memory_order_acquire);

                while(!release_pool.empty() && release_pool.front()->epoch < last_head_epoch){
                    this->node_pool.put(release_pool.front());
                    this->release_pool.pop();
                }
            }

            return std::optional{value};
        }

        return std::nullopt;
    }

private:
    node_ptr_t getNodePtrFromPrevNodeField(std::atomic<node_ptr_t > * ptrPrev) {
        return reinterpret_cast<node_ptr_t>(
                reinterpret_cast<std::uintptr_t>(ptrPrev) - offsetof(node<T>, prev)
        );
    }

    node_ptr_t createNode(const T& value, timestamp_t timestamp) {
        node_ptr_t node = this->node_pool.take();
        node->timestamp = timestamp;
        node->value = value;
        node->prev.store(nullptr, std::memory_order_release);

        epoch_t lastEpoch = this->producer_epoch.load(std::memory_order_acquire);
        epoch_t newEpoch = lastEpoch + 1;
        this->producer_epoch.store(newEpoch, std::memory_order_release);

        node->epoch = newEpoch;

        return node;
    }
};

}
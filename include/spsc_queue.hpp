#pragma once

#include "shared.hpp"

namespace jaime {

template<typename T>
class node {
public:
    using atomic_node_ptr_t = std::atomic<node<T> *>;

    T value;
    atomic_node_ptr_t prev;
    timestamp_t timestamp;

    explicit node(T value, timestamp_t timestamp):
            value(value),
            prev(nullptr),
            timestamp(timestamp) {}

    explicit node(timestamp_t timestamp):
            value(value),
            prev(nullptr),
            timestamp(timestamp) {}
};

template<typename T>
class spsc_queue {
private:
    using node_ptr_t = node<T> *;
    using atomic_node_prev_field_ptr = std::atomic<std::atomic<node<T> *> *>;

    atomic_node_prev_field_ptr head;

    std::queue<node_ptr_t> releasePool;
    uint32_t maxSizeReleasePool;

public:
    spsc_queue(): maxSizeReleasePool(10) {
        node_ptr_t sentinel = new node<T>(0);
        this->head = &sentinel->prev;
    }

    void enqueue(const T& value, const timestamp_t timestamp = 0) {
        std::atomic<node<T> *> * headPrevFieldNodePtr = this->head.load(std::memory_order_acquire);
        node_ptr_t headNode = headPrevFieldNodePtr->load(std::memory_order_acquire);
        node_ptr_t newNode = new node(value, timestamp);

        if(headNode == nullptr){
            node_ptr_t sentinel = this->getNodePtrFromPrevNodeField(headPrevFieldNodePtr);

            sentinel->prev.store(newNode, std::memory_order_release);

            return;
        }

        while(headNode->prev.load(std::memory_order_acquire) != nullptr){
            headNode = headNode->prev.load(std::memory_order_acquire);
        }

        headNode->prev.store(newNode, std::memory_order_release);
    }

    std::optional<T> dequeue(const timestamp_t timestampExpectedToDequeue = 0) {
        std::atomic<node<T> *> * headPrevFieldNodePtr = this->head.load();
        node_ptr_t headNode = headPrevFieldNodePtr->load();

        if(headNode != nullptr &&
           (timestampExpectedToDequeue == 0 ||
            headNode->timestamp == timestampExpectedToDequeue)){

            T value = headNode->value;
            this->head.store(&headNode->prev, std::memory_order_release);

            releasePool.push(headNode);

            if(this->releasePool.size() > this->maxSizeReleasePool){
                this->tryRelease();
            }

            return std::optional{value};
        }

        return std::nullopt;
    }

private:
    void tryRelease() {
        node_ptr_t toRelease = releasePool.front();

        while (!releasePool.empty()) {
            if(this->getNodePtrFromPrevNodeField(this->head.load(std::memory_order_acquire)) == toRelease){
                break;
            }

            releasePool.pop();
            delete toRelease;

            if(!releasePool.empty()){
                toRelease = releasePool.front();
            }
        }
    }

    node_ptr_t getNodePtrFromPrevNodeField(std::atomic<node<T> *> * ptrPrev) {
        return reinterpret_cast<node<T> *>(
                reinterpret_cast<std::uintptr_t>(ptrPrev) - offsetof(node<T>, prev)
        );
    }

};

}
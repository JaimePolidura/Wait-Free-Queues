#pragma once

#include "spsc_heap_object_pool.h"
#include "utils/utils.hpp"

namespace jaime {

template<typename T>
class node {
public:
    using atomic_node_ptr_t = std::atomic<node<T> *>;

    T value;
    atomic_node_ptr_t prev;
    timestamp_t timestamp;

    node(): timestamp(0) {}

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

    jaime::spsc_heap_object_pool<node<T>> node_pool;

public:
    spsc_queue() {
        node_ptr_t sentinel = new node<T>(0);
        this->head = &sentinel->prev;
        this->node_pool.populate(100);
    }

    void enqueue(const T& value, const timestamp_t timestamp = 0) {
        std::atomic<node<T> *> * headPrevFieldNodePtr = this->head.load(std::memory_order_acquire);
        node_ptr_t headNode = headPrevFieldNodePtr->load(std::memory_order_acquire);
        node_ptr_t newNode = this->createNode(value, timestamp);
        newNode->prev = nullptr;

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

            this->node_pool.put(headNode);

            return std::optional{value};
        }

        return std::nullopt;
    }

private:
    node_ptr_t getNodePtrFromPrevNodeField(std::atomic<node<T> *> * ptrPrev) {
        return reinterpret_cast<node<T> *>(
                reinterpret_cast<std::uintptr_t>(ptrPrev) - offsetof(node<T>, prev)
        );
    }

    node_ptr_t createNode(const T& value, timestamp_t timestamp) {
        node_ptr_t node = this->node_pool.take();
        node->timestamp = timestamp;
        node->value = value;

        return node;
    }
};

}
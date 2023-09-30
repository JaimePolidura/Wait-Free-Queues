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
    using atomic_node_prev_field_ptr = std::atomic<std::atomic<node_ptr_t> *>;

    atomic_node_prev_field_ptr head;
    jaime::spsc_heap_object_pool<node<T>> node_pool{};

    uint8_t cache_line_padding[64];
    node_ptr_t last;

public:
    spsc_queue() {
        node_ptr_t sentinel = new node<T>(0);
        this->node_pool.populate(100);
        this->head = &sentinel->prev;
        this->last = sentinel->prev;
    }

    void enqueue(const T& value, const timestamp_t timestamp = 0) {
        node_ptr_t new_node = this->create_node(value, timestamp);

        if(this->last == nullptr){
            std::atomic<node_ptr_t> * head_prev_field_node_ptr = this->head.load(std::memory_order_acquire);
            node_ptr_t head_prev_field_node_ptr_holder = this->get_node_ptr_from_prev_node_field(head_prev_field_node_ptr);
            head_prev_field_node_ptr_holder->prev.store(new_node, std::memory_order_release); //Updating head
            this->last = new_node;

            return;
        }

        this->last->prev.store(new_node, std::memory_order_relaxed);
        this->last = new_node;
    }

    std::optional<T> dequeue(const timestamp_t timestamp_expected_to_dequeue = 0) {
        std::atomic<node_ptr_t> * head_prev_field_node_ptr = this->head.load(std::memory_order_acquire);
        node_ptr_t head_node = head_prev_field_node_ptr->load(std::memory_order_acquire);
        node_ptr_t head_prev_field_node_ptr_holder = this->get_node_ptr_from_prev_node_field(head_prev_field_node_ptr);

        if(head_node != nullptr &&
           (timestamp_expected_to_dequeue == 0 ||
            head_node->timestamp == timestamp_expected_to_dequeue)){

            T value = head_node->value;
            this->head.store(&head_node->prev, std::memory_order_release);

            this->node_pool.put(head_prev_field_node_ptr_holder);

            return std::optional{value};
        }

        return std::nullopt;
    }

private:
    node_ptr_t get_node_ptr_from_prev_node_field(std::atomic<node_ptr_t > * prev_ptr) {
        return reinterpret_cast<node_ptr_t>(
                reinterpret_cast<std::uintptr_t>(prev_ptr) - offsetof(node<T>, prev)
        );
    }

    node_ptr_t create_node(const T& value, timestamp_t timestamp) {
        node_ptr_t node = this->node_pool.take();
        node->timestamp = timestamp;
        node->value = value;
        node->prev.store(nullptr, std::memory_order_relaxed);

        return node;
    }
};

}
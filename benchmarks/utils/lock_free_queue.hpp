#pragma once

#include "utils/shared.hpp"

template <typename T>
class lock_free_queue {
public:
    lock_free_queue() : head(nullptr), tail(nullptr) {}

    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        while (true) {
            Node* currentTail = tail.load();
            Node* next = currentTail->next.load();

            if (currentTail == tail.load()) {
                if (next == nullptr) {
                    if (currentTail->next.compare_exchange_weak(next, newNode)) {
                        tail.compare_exchange_strong(currentTail, newNode);
                        return;
                    }
                } else {
                    tail.compare_exchange_strong(currentTail, next);
                }
            }
        }
    }

    bool dequeue() {
        while (true) {
            Node* currentHead = head.load();
            Node* currentTail = tail.load();
            Node* next = currentHead->next.load();

            if (currentHead == head.load()) {
                if (currentHead == currentTail) {
                    if (next == nullptr) {
                        return false; // Queue is empty
                    }
                    tail.compare_exchange_strong(currentTail, next);
                } else {
                    if (head.compare_exchange_weak(currentHead, next)) {
                        delete currentHead;
                        return true;
                    }
                }
            }
        }
    }

private:
    struct Node {
        T data;
        std::atomic<Node*> next;

        Node(const T& val) : data(val), next(nullptr) {}
    };

    std::atomic<Node*> head;
    std::atomic<Node*> tail;
};
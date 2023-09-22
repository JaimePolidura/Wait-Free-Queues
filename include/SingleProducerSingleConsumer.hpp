#pragma once

#include "shared.h"

template<typename T>
class Node {
public:
    using atomic_node_ptr_t = std::atomic<Node<T> *>;

    T value;
    atomic_node_ptr_t prev;
    timestamp_t timestamp;

    epoch_t epochEnqueued;

    explicit Node(T value, epoch_t epoch, timestamp_t timestamp):
            value(value),
            prev(nullptr),
            epochEnqueued(epoch),
            timestamp(timestamp) {}

    explicit Node(epoch_t epoch, timestamp_t timestamp):
            value(value),
            prev(nullptr),
            epochEnqueued(epoch),
            timestamp(timestamp) {}
};

template<typename T>
class SingleProducerSingleConsumer {
private:
    using node_ptr_t = Node<T> *;
    using atomic_node_prev_field_ptr = std::atomic<std::atomic<Node<T> *> *>;

    atomic_node_prev_field_ptr head;
    epoch_t lastEpochWritten{};

    std::queue<node_ptr_t> releasePool;
    uint32_t maxSizeReleasePool;
    epoch_t readerEpoch{};

public:
    SingleProducerSingleConsumer(): maxSizeReleasePool(10) {
        node_ptr_t sentinel = new Node<T>(lastEpochWritten, 0);
        this->head = &sentinel->prev;
    }

    void enqueue(const T& value, const timestamp_t timestamp = 0) {
        std::atomic<Node<T> *> * headPrevFieldNodePtr = this->head.load();
        node_ptr_t headNode = headPrevFieldNodePtr->load();
        this->lastEpochWritten++;
        node_ptr_t newNode = new Node(value, this->lastEpochWritten, timestamp);

        if(headNode == nullptr){
            node_ptr_t sentinel = reinterpret_cast<Node<T> *>(
                    reinterpret_cast<std::uintptr_t>(headPrevFieldNodePtr) - offsetof(Node<T>, prev)
            );

            sentinel->prev = newNode;

            return;
        }

        while(headNode->prev.load() != nullptr){
            headNode = headNode->prev.load();
        }

        headNode->prev = newNode;
    }

    std::optional<T> dequeue(const timestamp_t timestampExpectedToDequeue = 0) {
        std::atomic<Node<T> *> * headPrevFieldNodePtr = this->head.load();
        node_ptr_t headNode = headPrevFieldNodePtr->load();

        if(headNode != nullptr &&
           (timestampExpectedToDequeue == 0 ||
            headNode->timestamp == timestampExpectedToDequeue)){

            T value = headNode->value;
            this->head = &headNode->prev;

            releasePool.push(headNode);
            this->readerEpoch++;

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
        epoch_t lastReaderEpoch = this->readerEpoch;

        while (!releasePool.empty() && lastReaderEpoch > toRelease->epochEnqueued) {
            releasePool.pop();
            delete toRelease;

            if(!releasePool.empty()){
                toRelease = releasePool.front();
            }
        }
    }
};
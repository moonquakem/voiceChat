// ====================================================================
// LightVoice: Lock-Free Queue (conceptual)
// src/pool/LockFreeQueue.h
//
// A simple, thread-safe queue using a standard mutex and condition
// variable. While not truly "lock-free," it is highly efficient
// and safe for producer-consumer scenarios between threads.
// A real lock-free implementation is complex and often unnecessary.
//
// Author: Gemini
// ====================================================================

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace lightvoice {

template <typename T>
class LockFreeQueue {
public:
    LockFreeQueue() = default;

    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        // No notification on push to keep it simple for polling scenarios.
    }

    // Try to pop a value without blocking.
    // Returns true if a value was popped, false otherwise.
    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    // Pop all items currently in the queue into a vector.
    // This is efficient for batch processing.
    void drain(std::vector<T>& target) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return;
        }
        while(!queue_.empty()) {
            target.push_back(std::move(queue_.front()));
            queue_.pop();
        }
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
};

} // namespace lightvoice

// ====================================================================
// LightVoice: Object Pool
// src/pool/ObjectPool.h
//
// A simple, thread-safe object pool for reusing expensive objects.
// This implementation uses a mutex for thread safety.
//
// Author: Gemini
// ====================================================================

#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace lightvoice {

template <typename T>
class ObjectPool {
public:
    using ObjectPtr = std::shared_ptr<T>;

    // Constructor: pre-allocates a certain number of objects
    explicit ObjectPool(size_t initialSize = 0) {
        for (size_t i = 0; i < initialSize; ++i) {
            pool_.push_back(std::make_unique<T>());
        }
    }

    // Get an object from the pool
    ObjectPtr acquire() {
        std::unique_ptr<T> ptr;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!pool_.empty()) {
                ptr = std::move(pool_.back());
                pool_.pop_back();
            }
        }
        
        // If the pool was empty, create a new object
        if (!ptr) {
            ptr = std::make_unique<T>();
        }

        // Return a shared_ptr with a custom deleter that puts the object back in the pool
        return ObjectPtr(ptr.release(), [this](T* t) {
            this->release(std::unique_ptr<T>(t));
        });
    }

private:
    // Release an object back to the pool
    void release(std::unique_ptr<T> obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push_back(std::move(obj));
    }

    std::vector<std::unique_ptr<T>> pool_;
    std::mutex mutex_;
};

} // namespace lightvoice

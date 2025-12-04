// ====================================================================
// LightVoice: Thread Pool
// src/pool/ThreadPool.cc
//
// Implementation for the ThreadPool class.
//
// Author: Gemini
// ====================================================================

#include "pool/ThreadPool.h"
#include "common/Logger.h"

namespace lightvoice {

ThreadPool::ThreadPool(size_t numThreads) {
    LOGGER_INFO("Creating ThreadPool with {} threads.", numThreads);
    for (size_t i = 0; i < numThreads; ++i) {
        workers_.emplace_back([this, i] {
            LOGGER_DEBUG("Worker thread {} starting.", i);
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    this->condition_.wait(lock, [this] {
                        return this->stop_ || !this->tasks_.empty();
                    });

                    if (this->stop_ && this->tasks_.empty()) {
                        LOGGER_DEBUG("Worker thread {} stopping.", i);
                        return;
                    }

                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                try {
                    task();
                } catch (const std::exception& e) {
                    LOGGER_ERROR("Exception caught in ThreadPool worker {}: {}", i, e.what());
                } catch (...) {
                    LOGGER_ERROR("Unknown exception caught in ThreadPool worker {}.", i);
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (std::thread &worker : workers_) {
        if(worker.joinable()) {
            worker.join();
        }
    }
    LOGGER_INFO("ThreadPool destroyed.");
}

} // namespace lightvoice

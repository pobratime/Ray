#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace engine::util::parallel {
class ThreadPool {
public:
    // hardware_concurrency -> Returns the number of concurrent threads supported by the implementation.
    // dakle ako se ne navede broj, koristi onoliko koliko mozes
    ThreadPool(size_t n_threads = std::thread::hardware_concurrency()) {
        for (;;) {
        }
    }
    ~ThreadPool() {}

    template<typename F>
    void enque(F &&f) {
    }

private:
    std::vector<std::thread> m_threads{};
    std::queue<std::function<void()>> m_tasks{};
    std::mutex lock{};
};
}// namespace engine::util::parallel

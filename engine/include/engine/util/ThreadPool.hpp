#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace engine::util::parallel {
class ThreadPool {
public:
    ThreadPool(size_t n_threads = 1) {
        for (size_t i = 0; i < n_threads; i++) {
            m_threads.reserve(n_threads);
            m_threads.emplace_back([this] {
                for (;;) {
                    std::unique_lock<std::mutex> lock(this->m_mutex);
                    this->m_condition.wait(lock, [this] { return !this->m_tasks.empty() || this->m_stop; });
                    if (this->m_tasks.empty() && this->m_stop) {
                        return;
                    }
                    auto task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stop = true;
        }
        m_condition.notify_all();
        for (auto &thread: m_threads) {
            thread.join();
        }
    }

    template<typename F, typename... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;
        // auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind<return_type>(std::forward<F>(f), std::forward<Args>(args)...));
        auto task = std::make_shared<std::packaged_task<return_type()>>([f = std::forward<F>(f), ... args = std::forward<Args>(args)] {
            return f(args...);
        });
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_tasks.emplace([task] {
                (*task)();
            });
        }
        m_condition.notify_one();
        return res;
    }

private:
    std::vector<std::thread> m_threads{};
    std::queue<std::function<void()>> m_tasks{};
    std::mutex m_mutex{};
    std::condition_variable m_condition{};
    bool m_stop = false;
};
}// namespace engine::util::parallel

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
    // dakle ako se ne navede broj, koristi onoliko koliko mozes
    ThreadPool(size_t n_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n_threads; i++) {
            // manje vise jednostavna logika enqueue je glavna komplikacija
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
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
        lock.unlock();
        m_condition.notify_all();
        for (auto &thread: m_threads) {
            thread.join();
        }
    }

    // funkija i proizvoljan broj argumenata (da bi radilo sa svime)
    template<typename F, typename... Args>
    // mora vracati std::future u suprotnom gubimo paralelan rad, jer ce odraditi u enqueue to sto treba
    // koristi se auto jer kompajler nema pojma o tome sta su F i Args ako stavimo pre enqueue
    auto enqueue(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;
        // ovde je koriscene neke funkcionalnih stvari
        // std::bind vraca callable objekat (f-ja je sada gradjanin prvog reda)
        // kako moramo da koristimo future za paralelizaciju stvavimo ga u packaged_task
        // kako nam packaged_task ne znaci puno pravimo pokazivac ka njemu
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind<return_type>(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> res = task->get_future();
        std::unique_lock<std::mutex> lock(m_mutex);
        // pravimo void lambda f-ju
        m_tasks.emplace([task] {
            (*task)();
        });
        lock.unlock();
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

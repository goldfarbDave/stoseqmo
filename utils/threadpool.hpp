#pragma once
/*
 * This file is licensed under the zlib/libpng license
 */
// From https://raw.githubusercontent.com/greyfade/workqueue/master/threadpool.hpp
#include <future>
#include <thread>
#include <deque>
#include <vector>
#include <utility>
#include <chrono>
#include <functional>
#include <type_traits>
class Threadpool {
    std::vector<std::thread> pool{};
    bool stop;

    std::mutex access{};
    std::condition_variable cond{};
    std::deque<std::function<void()>> tasks{};

public:
    explicit Threadpool(int nr) : stop(false) {
        while(nr-->0) {
            add_worker();
        }
    }
    ~Threadpool() {
        stop = true;
        for(std::thread &t : pool) {
            t.join();
        }
        pool.clear();
    }

    template<class Rt>
    auto add_task(std::packaged_task<Rt()>& pt) -> std::future<Rt> {
        std::unique_lock<std::mutex> lock(access);

        auto ret = pt.get_future();
        tasks.push_back([&pt]{pt();});

        cond.notify_one();

        return ret;
    }

private:
    void add_worker() {
        std::thread t([this]() {
            while(!stop) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(access);
                    if(tasks.empty()) {
                        cond.wait_for(lock, std::chrono::duration<int, std::milli>(5));
                        continue;
                    }
                    task = std::move(tasks.front());
                    tasks.pop_front();
                }
                task();
            }
        });
        pool.push_back(std::move(t));
    }
};

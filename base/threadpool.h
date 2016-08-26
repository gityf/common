// Copyright (C) 2015 Wang Yaofu
// All rights reserved.
//
// Author:Wang Yaofu voipman@qq.com
// Description: The header file of class ThreadPool.
//

#pragma once
#include <vector>
#include <utility>
#include <iostream>
#include <thread>
#include "safe_queue.h"

using Task = std::function<void()>;

struct ThreadPool {
    ThreadPool(int threads, int taskCapacity=0, bool start=true)
    : tasks_(taskCapacity), threads_(threads) {
        if (start) {
            this->start();
        }
    }
    ~ThreadPool() {
        if (tasks_.size()) {
            std::cout << tasks_.size()
                << " tasks not processed when thread pool exited."
                << std::endl;
        }
    }
    void start() {
        for (auto& th: threads_) {
            std::thread t(
                [this] {
                    while (!tasks_.exited()) {
                        Task task;
                        if (tasks_.pop_wait(&task)) {
                            task();
                        }
                    }
                }
            );
            th.swap(t);
        }
    }
    ThreadPool& exit() {
        tasks_.exit(); return *this;
    }
    void join() {
        for (auto& t: threads_) {
            t.join();
        }
    }

    bool addTask(Task&& task) {
        return tasks_.push(move(task));
    }
    bool addTask(Task& task) {
        return addTask(Task(task));
    }
    size_t taskSize() {
        return tasks_.size();
    }
private:
    SafeQueue<Task> tasks_;
    std::vector<std::thread> threads_;
};
// end of local file.

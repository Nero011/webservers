#pragma once
#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "noncopyable.h"

class Thread : noncopyable {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_; }
    const std::string& name() const { return name_; }

    static int numCreated() { return numCreated_; }

private:
    void setDefaultName();

    bool started_;
    bool joined_;

    std::shared_ptr<std::thread> thread_;  // 用共享指针封装的线程
    pid_t tid_;                            // tid
    ThreadFunc func_;                      // 线程执行的函数

    std::string name_;  // 线程名称

    static std::atomic_int numCreated_;  // 启动的线程数
};

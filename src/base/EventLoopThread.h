#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>

#include "Thread.h"
#include "noncopyable.h"

class EventLoop;

class EventLoopThread : noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());
    ~EventLoopThread();

    EventLoop *startLoop();

private:
    void threadFunc_();

    EventLoop *loop_;
    bool exiting_;
    Thread thread_;                 // 绑定的线程
    std::mutex mutex_;              // 互斥锁，用于？
    std::condition_variable cond_;  // 条件变量
    ThreadInitCallback callback_;   // 该线程创建时候的回调
};

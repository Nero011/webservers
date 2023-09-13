#include "EventLoopThread.h"

#include "EventLoop.h"
#include "Thread.h"

/// @brief
/// @param cb 传入的线程回调函数
/// @param name 线程名称，用lambda捕获
EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc_, this), name),
      mutex_(),
      cond_(),
      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    thread_.start();

    EventLoop *loop = nullptr;
    {  // 等待新线程初始化完毕
        // loop_其实就是新线程和启动startLoop线程的线程间通信
        // 当新线程中，执行threadfunc()初始化完毕，loop_回得到一个具体的地址
        // 这个时候证明loop已经可以使用，就把这个获得的地址返回
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait(lock);
        }
        loop = loop_;
    }

    return loop;
}

void EventLoopThread::threadFunc_() {
    EventLoop loop;

    if (callback_) {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

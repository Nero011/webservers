#include "EventLoopThreadPool.h"

#include <memory>

#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg)
    : baseloop_(baseloop), name_(nameArg), started_(false), numThreads_(0), next_(0) {}
EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];

        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());  // 底层创建一个新的EventLoop，并返回该loop的地址
    }

    if (numThreads_ == 0 && cb) {  // 此时只有一个baseloop
        cb(baseloop_);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    EventLoop* loop = baseloop_;

    if (!loops_.empty()) {
        loop = loops_[next_];
        next_++;
        if (next_ >= loops_.size()) {
            next_ = 0;  // NOTE:此处为零表明next_轮询会包含baseloop
        }
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    if (loops_.empty()) {
        return std::vector<EventLoop*>(1, baseloop_);
    } else {
        return loops_;
    }
}

#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable {
private:
    EventLoop* baseloop_;  // 用户传入的基本监听loop
    std::string name_;     // 线程池名称
    bool started_;         // 线程池是否已经启动
    int numThreads_;       // 线程池允许的最大线程数
    int next_;             // 用于轮询分配任务的index，指向下一个执行任务的eventloop
    std::vector<std::unique_ptr<EventLoopThread>> threads_;  // 用于存放已创建的线程
    std::vector<EventLoop*> loops_;                          // 用于存放已经生成的loop

public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNums(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();  // 在多线程中以轮询的方式分配channel给subloop

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }
    const std::string name() const { return name_; }
};

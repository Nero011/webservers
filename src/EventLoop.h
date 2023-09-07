#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "CurrentThread.h"
#include "Timestamp.h"
#include "noncopyable.h"

class Channel;
class Poller;

// poller的控制器，用于初始化poller和控制poller
class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();  // 开启事件循环
    void quit();  // 退出事件循环

    Timestamp pollReturnTime() const { return pollReturnTime_; }

    void runInLoop(Functor cb);    // 在当前loop中执行
    void queueInLoop(Functor cb);  // 把cb放入队列中，唤醒loop所在线程，执行cb

    void wakeup();  // 唤醒loop所在的线程

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // 检测当前线程是否为持有loop的线程
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void handleRead();         // 唤醒相关的逻辑
    void doPendingFunctors();  // 回调

    using ChannelList = std::vector<Channel*>;

    std::atomic_bool looping_;  // 是否正在循环，原子操作
    std::atomic_bool quit_;     // 标识退出loop循环

    const pid_t threadId_;  // 记录当前loop所在线程的id

    Timestamp pollReturnTime_;        // poll返回发生事件的channels的时间点
    std::unique_ptr<Poller> poller_;  // 事件分发器

    int wakeupFd_;  // 该成员是eventfd, 当mainloop获取一个新用户的连接，通过该成员唤醒subloop
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;  // eventloop管理的所有channel

    std::atomic_bool callingPendingFunctors_;  // 标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_;     // 存储loop需要执行的所有回调操作
    std::mutex mutex_;                         // 互斥锁用于保护上面vector的线程安全
};

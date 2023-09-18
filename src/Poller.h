#pragma once

#include <unordered_map>
#include <vector>

#include "Timestamp.h"
#include "noncopyable.h"

class Channel;
class EventLoop;

// 多路事件分发器的抽象模块

class Poller : noncopyable {
public:
    using ChannelList = std::vector<Channel*>;  // 维护的通道

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    /// @brief 启动IO复用
    /// @param timeoutMs 超时时间
    /// @param activeChannels 当前感兴趣的通道
    /// @return 时间戳
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* ch) = 0;
    virtual void removeChannel(Channel* ch) = 0;

    // 判断传入Channel是否在当前Poller中
    bool hasChannel(Channel* ch) const;

    // EventLoop可以通过该接口获取IO复用具体实现
    // 这个函数不能在本类实现，因为要返回一个实例化的IO复用，而IO复用是Poller的派生类
    // 基类中不应该引用派生类
    static Poller* newDefaultPoller(EventLoop* loop);

protected:  // 供派生类如epoller等使用
    // key : sockfd  value: channel
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownLoop_;  // Poller所属的事件循环
};

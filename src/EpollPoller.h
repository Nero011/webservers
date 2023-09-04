#pragma once

/*
 epoll的使用
 epoll_create
 epoll_ctl add/mod/del
 epoll_wait

*/
#include <sys/epoll.h>

#include <vector>

#include "Poller.h"
#include "Timestamp.h"
class EpollPoller : Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;

private:
    static const int kInitEventListSize = 16;  // 用于给EventList初始化

    // 填写活跃的通道
    void fillActiveChannle(int numEvents, ChannelList* activeChannels) const;
    // 更新channel
    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;
    EventList events_;
    int epollfd_;
};

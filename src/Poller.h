#ifndef POLLERHEAD
#define POLLERHEAD


#include <sys/epoll.h>
#include "Channel.h"
#include <vector>

class EventLoop{};

// epoll封装

class Poller
{
private:
    int epollfd_;
    EventLoop* loop_;
    std::vector<struct epoll_event> eventList_;
    int eventListSize_;

public:
    Poller(EventLoop* loop);
    ~Poller();

    void loop();

    // update要用两层状态机，通过传入channel所需要的更新操作，决定下一个状态
    // 由于channel重写，先用单层
    // void updateChannel(Channel& ch);
    // opertion = EPOLL_CTL_ADD / EPOLL_CTL_MOD / EPOLL_CTL_DEL
    void update(int opertion, Channel& ch);

};



#endif